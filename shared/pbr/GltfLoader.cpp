////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
// Enable iterative parsing to avoid possible stack overflow
#define RAPIDJSON_PARSE_DEFAULT_FLAGS kParseIterativeFlag
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_USE_RAPIDJSON_CRTALLOCATOR
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include "..\Gltf\GltfHelper.h"
#include "GltfLoader.h"

using namespace DirectX;

namespace {
    // Create a DirectX texture view from a tinygltf Image.
    winrt::com_ptr<ID3D11ShaderResourceView> LoadImage(_In_ ID3D11Device* device, const tinygltf::Image& image, bool sRGB) {
        // First convert the image to RGBA if it isn't already.
        std::vector<uint8_t> tempBuffer;
        const uint8_t* rgbaBuffer = GltfHelper::ReadImageAsRGBA(image, &tempBuffer);
        if (rgbaBuffer == nullptr) {
            return nullptr;
        }

        const DXGI_FORMAT format = sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        return Pbr::Texture::CreateTexture(device, rgbaBuffer, image.width * image.height * 4, image.width, image.height, format);
    }

    D3D11_FILTER ConvertFilter(int glMinFilter, int glMagFilter) {
        const D3D11_FILTER_TYPE minFilter = glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST
                                                ? D3D11_FILTER_TYPE_POINT
                                                : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR
                                                      ? D3D11_FILTER_TYPE_LINEAR
                                                      : glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST
                                                            ? D3D11_FILTER_TYPE_POINT
                                                            : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST
                                                                  ? D3D11_FILTER_TYPE_LINEAR
                                                                  : glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR
                                                                        ? D3D11_FILTER_TYPE_POINT
                                                                        : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
                                                                              ? D3D11_FILTER_TYPE_LINEAR
                                                                              : D3D11_FILTER_TYPE_POINT;
        const D3D11_FILTER_TYPE mipFilter = glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST
                                                ? D3D11_FILTER_TYPE_POINT
                                                : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR
                                                      ? D3D11_FILTER_TYPE_POINT
                                                      : glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST
                                                            ? D3D11_FILTER_TYPE_POINT
                                                            : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST
                                                                  ? D3D11_FILTER_TYPE_POINT
                                                                  : glMinFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR
                                                                        ? D3D11_FILTER_TYPE_LINEAR
                                                                        : glMinFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
                                                                              ? D3D11_FILTER_TYPE_LINEAR
                                                                              : D3D11_FILTER_TYPE_POINT;
        const D3D11_FILTER_TYPE magFilter =
            glMagFilter == TINYGLTF_TEXTURE_FILTER_NEAREST
                ? D3D11_FILTER_TYPE_POINT
                : glMagFilter == TINYGLTF_TEXTURE_FILTER_LINEAR ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT;

        const D3D11_FILTER filter = D3D11_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, D3D11_FILTER_REDUCTION_TYPE_STANDARD);
        return filter;
    }

    // Create a DirectX sampler state from a tinygltf Sampler.
    winrt::com_ptr<ID3D11SamplerState> CreateSampler(_In_ ID3D11Device* device,
                                                     const tinygltf::Model& gltfModel,
                                                     const tinygltf::Sampler& sampler) {
        D3D11_SAMPLER_DESC samplerDesc{};

        samplerDesc.Filter = ConvertFilter(sampler.minFilter, sampler.magFilter);
        samplerDesc.AddressU =
            sampler.wrapS == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE
                ? D3D11_TEXTURE_ADDRESS_CLAMP
                : sampler.wrapS == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV =
            sampler.wrapT == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE
                ? D3D11_TEXTURE_ADDRESS_CLAMP
                : sampler.wrapT == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        winrt::com_ptr<ID3D11SamplerState> samplerState;
        Pbr::Internal::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, samplerState.put()));
        return samplerState;
    }

    // Maps a glTF material to a PrimitiveBuilder. This optimization combines all primitives which use
    // the same material into a single primitive for reduced draw calls. Each primitive's vertex specifies
    // which node it corresponds to any appropriate node transformation be happen in the shader.
    using PrimitiveBuilderMap = std::map<int, Pbr::PrimitiveBuilder>;

    // Load a glTF node from the tinygltf object model. This will process the node's mesh (if specified) and then recursively load the child
    // nodes too.
    void XM_CALLCONV LoadNode(Pbr::NodeIndex_t parentNodeIndex,
                              const tinygltf::Model& gltfModel,
                              int nodeId,
                              PrimitiveBuilderMap& primitiveBuilderMap,
                              Pbr::Model& model) {
        const tinygltf::Node& gltfNode = gltfModel.nodes.at(nodeId);

        // Read the local transform for this node and add it into the Pbr Model.
        const XMMATRIX nodeLocalTransform = GltfHelper::ReadNodeLocalTransform(gltfNode);
        const Pbr::NodeIndex_t transformIndex = model.AddNode(nodeLocalTransform, parentNodeIndex, gltfNode.name);

        if (gltfNode.mesh != -1) // Load the node's optional mesh when specified.
        {
            // A glTF mesh is composed of primitives.
            const tinygltf::Mesh& gltfMesh = gltfModel.meshes.at(gltfNode.mesh);
            for (const tinygltf::Primitive& gltfPrimitive : gltfMesh.primitives) {
                // Read the primitive data from the glTF buffers.
                const GltfHelper::Primitive primitive = GltfHelper::ReadPrimitive(gltfModel, gltfPrimitive);

                // Insert or append the primitive into the PBR primitive builder. Primitives which use the same
                // material are appended to reduce the number of draw calls.
                Pbr::PrimitiveBuilder& primitiveBuilder = primitiveBuilderMap[gltfPrimitive.material];

                // Use the starting offset for vertices and indices since multiple glTF primitives can
                // be put into the same primitive builder.
                const uint32_t startVertex = (uint32_t)primitiveBuilder.Vertices.size();
                const uint32_t startIndex = (uint32_t)primitiveBuilder.Indices.size();

                // Convert the GltfHelper vertices into the PBR vertex format.
                primitiveBuilder.Vertices.resize(startVertex + primitive.Vertices.size());
                for (size_t i = 0; i < primitive.Vertices.size(); i++) {
                    const GltfHelper::Vertex& vertex = primitive.Vertices[i];
                    Pbr::Vertex pbrVertex;
                    pbrVertex.Position = vertex.Position;
                    pbrVertex.Normal = vertex.Normal;
                    pbrVertex.Tangent = vertex.Tangent;
                    pbrVertex.Color0 = vertex.Color0;
                    pbrVertex.TexCoord0 = vertex.TexCoord0;
                    pbrVertex.ModelTransformIndex = transformIndex;

                    primitiveBuilder.Vertices[i + startVertex] = pbrVertex;
                }

                // Insert indicies with reverse winding order.
                primitiveBuilder.Indices.resize(startIndex + primitive.Indices.size());
                for (size_t i = 0; i < primitive.Indices.size(); i += 3) {
                    primitiveBuilder.Indices[startIndex + i + 0] = startVertex + primitive.Indices[i + 0];
                    primitiveBuilder.Indices[startIndex + i + 1] = startVertex + primitive.Indices[i + 2];
                    primitiveBuilder.Indices[startIndex + i + 2] = startVertex + primitive.Indices[i + 1];
                }
            }
        }

        // Recursively load all children.
        for (const int childNodeId : gltfNode.children) {
            LoadNode(transformIndex, gltfModel, childNodeId, primitiveBuilderMap, model);
        }
    }
} // namespace

namespace Gltf {
    std::shared_ptr<Pbr::Model> FromGltfObject(const Pbr::Resources& pbrResources, const tinygltf::Model& gltfModel) {
        // Start off with an empty Pbr Model.
        auto model = std::make_shared<Pbr::Model>();

        // Read and transform mesh/node data. Primitives with the same material are merged to reduce draw calls.
        PrimitiveBuilderMap primitiveBuilderMap;
        {
            const int defaultSceneId = (gltfModel.defaultScene == -1) ? 0 : gltfModel.defaultScene;
            const tinygltf::Scene& defaultScene = gltfModel.scenes.at(defaultSceneId);

            // Process the root scene nodes. The children will be processed recursively.
            for (const int rootNodeId : defaultScene.nodes) {
                LoadNode(Pbr::RootNodeIndex, gltfModel, rootNodeId, primitiveBuilderMap, *model);
            }
        }

        // Load the materials referenced by the primitives
        std::map<int, std::shared_ptr<Pbr::Material>> materialMap;
        {
            // Create D3D cache for reuse of texture views and samplers when possible.
            using ImageKey = std::tuple<const tinygltf::Image*, bool>; // Item1 is a pointer to the image, Item2 is sRGB.
            std::map<ImageKey, winrt::com_ptr<ID3D11ShaderResourceView>> imageMap;
            std::map<const tinygltf::Sampler*, winrt::com_ptr<ID3D11SamplerState>> samplerMap;

            // primitiveBuilderMap is grouped by material. Loop through the referenced materials and load their resources. This will only
            // load materials which are used by the active scene.
            for (const auto& primitiveBuilderPair : primitiveBuilderMap) {
                std::shared_ptr<Pbr::Material> pbrMaterial;

                const int materialIndex = primitiveBuilderPair.first;
                if (materialIndex == -1) // No material was referenced. Make up a material for it.
                {
                    // Default material is a grey material, 50% roughness, non-metallic.
                    pbrMaterial = Pbr::Material::CreateFlat(pbrResources, {0.5f, 0.5f, 0.5f, 0.5f}, 0.5f);
                } else {
                    const tinygltf::Material& gltfMaterial = gltfModel.materials.at(materialIndex);

                    const GltfHelper::Material material = GltfHelper::ReadMaterial(gltfModel, gltfMaterial);
                    pbrMaterial = std::make_shared<Pbr::Material>(pbrResources);

                    // Read a tinygltf texture and sampler into the Pbr Material.
                    auto loadTexture = [&](Pbr::ShaderSlots::PSMaterial slot,
                                           const GltfHelper::Material::Texture& texture,
                                           bool sRGB,
                                           Pbr::RGBAColor defaultRGBA) {
                        // Find or load the image referenced by the texture.
                        const ImageKey imageKey = std::make_tuple(texture.Image, sRGB);
                        winrt::com_ptr<ID3D11ShaderResourceView> textureView = imageMap[imageKey];
                        if (!textureView) // If not cached, load the image and store it in the texture cache.
                        {
                            // TODO: Generate mipmaps if sampler's minification filter (minFilter) uses mipmapping.
                            // TODO: If texture is not power-of-two and (sampler has wrapping=repeat/mirrored_repeat OR minFilter uses
                            // mipmapping), resize to power-of-two.
                            textureView = texture.Image != nullptr ? LoadImage(pbrResources.GetDevice().get(), *texture.Image, sRGB)
                                                                   : pbrResources.CreateSolidColorTexture(defaultRGBA);
                            imageMap[imageKey] = textureView;
                        }

                        // Find or create the sampler referenced by the texture.
                        winrt::com_ptr<ID3D11SamplerState> samplerState = samplerMap[texture.Sampler];
                        if (!samplerState) // If not cached, create the sampler and store it in the sampler cache.
                        {
                            samplerState = texture.Sampler != nullptr
                                               ? CreateSampler(pbrResources.GetDevice().get(), gltfModel, *texture.Sampler)
                                               : Pbr::Texture::CreateSampler(pbrResources.GetDevice().get(), D3D11_TEXTURE_ADDRESS_WRAP);
                            samplerMap[texture.Sampler] = samplerState;
                        }

                        pbrMaterial->SetTexture(slot, textureView.get(), samplerState.get());
                    };

                    pbrMaterial->Name = gltfMaterial.name;

                    loadTexture(Pbr::ShaderSlots::BaseColor, material.BaseColorTexture, true /* sRGB */, Pbr::RGBA::White);
                    loadTexture(Pbr::ShaderSlots::MetallicRoughness, material.MetallicRoughnessTexture, false /* sRGB */, Pbr::RGBA::White);
                    loadTexture(Pbr::ShaderSlots::Emissive, material.EmissiveTexture, true /* sRGB */, Pbr::RGBA::White);
                    loadTexture(Pbr::ShaderSlots::Normal, material.NormalTexture, false /* sRGB */, Pbr::RGBA::FlatNormal);
                    loadTexture(Pbr::ShaderSlots::Occlusion, material.OcclusionTexture, false /* sRGB */, Pbr::RGBA::White);

                    pbrMaterial->SetDoubleSided(material.DoubleSided);
                    pbrMaterial->SetAlphaBlended(material.AlphaMode == GltfHelper::AlphaMode::Blend);

                    Pbr::Material::ConstantBufferData& parameters = pbrMaterial->Parameters();
                    parameters.BaseColorFactor = material.BaseColorFactor;
                    parameters.MetallicFactor = material.MetallicFactor;
                    parameters.RoughnessFactor = material.RoughnessFactor;
                    parameters.EmissiveFactor = material.EmissiveFactor;
                    parameters.OcclusionStrength = material.OcclusionStrength;
                    parameters.NormalScale = material.NormalScale;
                    parameters.AlphaCutoff =
                        material.AlphaMode == GltfHelper::AlphaMode::Mask ? material.AlphaCutoff : std::numeric_limits<float>::lowest();
                }

                materialMap.insert(std::make_pair(materialIndex, std::move(pbrMaterial)));
            }
        }

        // Convert the primitive builders into primitives with their respective material and add it into the Pbr Model.
        for (const auto& primitiveBuilderPair : primitiveBuilderMap) {
            const Pbr::PrimitiveBuilder& primitiveBuilder = primitiveBuilderPair.second;
            const std::shared_ptr<Pbr::Material>& material = materialMap.find(primitiveBuilderPair.first)->second;
            model->AddPrimitive(Pbr::Primitive(pbrResources, primitiveBuilder, material));
        }

        return model;
    }

    std::shared_ptr<Pbr::Model> FromGltfBinary(const Pbr::Resources& pbrResources,
                                               _In_reads_bytes_(bufferBytes) const uint8_t* buffer,
                                               uint32_t bufferBytes) {
        // Parse the GLB buffer data into a tinygltf model object.
        tinygltf::Model gltfModel;
        std::string errorMessage;
        tinygltf::TinyGLTF loader;
        if (!loader.LoadBinaryFromMemory(&gltfModel, &errorMessage, nullptr /*warn*/, buffer, bufferBytes, ".")) {
            const auto msg =
                std::string("\r\nFailed to load gltf model (") + std::to_string(bufferBytes) + " bytes). Error: " + errorMessage;
            throw std::exception(msg.c_str());
        }

        return FromGltfObject(pbrResources, gltfModel);
    }
} // namespace Gltf
