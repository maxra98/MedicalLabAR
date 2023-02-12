#include "pch.h"
#include "PbrModelObject.h"
#include "TextTexture.h"
#include "Scene.h"
#include "BoxLinePos.h"
#include "AppLanguage.h"

#include <Unknwn.h> // Required to interop with IUnknown. Must be included before C++/WinRT headers.
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.Preview.Holographic.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Graphics.Holographic.h>

namespace windows {
    using namespace winrt::Windows::ApplicationModel::Activation;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::Text::Core;
    using namespace winrt::Windows::UI::ViewManagement;
    using namespace winrt::Windows::Graphics::Holographic;
    using namespace winrt::Windows::ApplicationModel::Preview::Holographic;
} // namespace windows

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    struct SkinBlockModelLabels : public engine::Scene {

        SkinBlockModelLabels(engine::Context& context, AppLanguage language) : Scene(context), m_language(language)
        {
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("create_labels", "Create labels action");

            m_selectAction = actionSet.CreateAction("select_action", "Select Action", XR_ACTION_TYPE_BOOLEAN_INPUT, {});

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                {
                    {m_selectAction, "/user/hand/right/input/select"},
                    {m_selectAction, "/user/hand/left/input/select"},
                });

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                {
                    {m_selectAction, "/user/hand/right/input/trigger"},
                    {m_selectAction, "/user/hand/left/input/trigger"},
                });

            if (context.Extensions.SupportsHandInteraction) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                    {
                        {m_selectAction, "/user/hand/right/input/select"},
                        {m_selectAction, "/user/hand/left/input/select"},
                    });
            }

            XrReferenceSpaceCreateInfo createInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            createInfo.poseInReferenceSpace = Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(m_context.Session.Handle, &createInfo, m_viewSpace.Put(xrDestroySpace)));

            // Text block size
            constexpr float margin = 0.001f;
            constexpr float titleWidth = 0.06f;
            constexpr float titleHeight = 0.02f;

            engine::TextTextureInfo textInfo{ 306, 178 };

            textInfo.Foreground = Pbr::RGBA::White;
            textInfo.Background = Pbr::FromSRGB(Colors::DarkBlue);
            textInfo.Margin = 0.5; // pixels
            textInfo.TextAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;

            const auto& material = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::FromSRGB(Colors::Transparent));
            m_background = AddObject(engine::CreateQuad(m_context.PbrResources, { titleWidth, titleHeight }, material));
            m_background->SetVisible(false);

            auto placeTextBlock = [&](TextBlock& block, float x, float y, float z)
            {
                // Creates the white background for the TextBlock
                XrPosef textBlockBackgroundPos;
                const auto& material_background = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::FromSRGB(Colors::White));
                block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { titleWidth, titleHeight }, material_background));
                textBlockBackgroundPos.position = { x, y, z };
                textBlockBackgroundPos.orientation = Quaternion::Identity();
                block.Object->Pose() = textBlockBackgroundPos;
                block.Object->SetParent(m_background);

                // Creates the dark blue background for the TextBlock
                XrPosef textBlockForegroundPos;
                textInfo.Height = (uint32_t)std::floor((titleHeight - (2 * margin)) * textInfo.Width / titleWidth); // Keep texture aspect ratio
                auto textTexture = std::make_unique<engine::TextTexture>(m_context, textInfo);
                textTexture->Draw(block.Text.c_str());
                const auto& material_foreground = textTexture->CreatePbrMaterial(m_context.PbrResources);
                block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { titleWidth - (2 * margin), titleHeight - (2 * margin) }, material_foreground));
                textBlockForegroundPos.position = { x, y, z + 0.002f }; // Foreground z-value should not be the same as in Background
                textBlockForegroundPos.orientation = Quaternion::Identity();
                block.Object->Pose() = textBlockForegroundPos;
                block.Object->SetParent(m_background);
            };

            // Angle in rad
            auto placeBoxLine = [&](TextBlock& block, float x, float y, float z, float lineLength, BoxLinePos pos, float angle) {
                XrPosef boxLinePos;
                float boxLineHight;
                float boxLineWidth;
                float angleRatio;
                const auto& material_box_line = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::FromSRGB(Colors::White));

                //// Specifies the ratio for the distance that results from the BoxLine rotation.
                // The line rotation happens in the middle of the line. Because of that it needs some
                // geometry to position the end of the lines to the middle of the TextBlocks.
                // 1.5% of the angle is enough to bring the line again near to the middle.
                if (angle < 0)
                {
                    angleRatio = angle * (-0.015f);
                }
                else
                {
                    angleRatio = angle * 0.015f;
                }

                switch (pos)
                {
                case BM:
                    boxLineHight = lineLength;
                    boxLineWidth = margin;
                    block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { boxLineWidth, boxLineHight }, material_box_line));
                    boxLinePos.orientation = Quaternion::RotationAxisAngle({ 0, 0, 1 }, angle);
                    boxLinePos.position = { x + ((boxLineHight / 2) * std::sin(angle)), y - ((titleHeight / 2) + (boxLineHight / 2)) + angleRatio, z };
                    break;
                case LM:
                    boxLineHight = margin;
                    boxLineWidth = lineLength;
                    block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { boxLineWidth, boxLineHight }, material_box_line));
                    boxLinePos.orientation = Quaternion::RotationAxisAngle({ 0, 0, 1 }, angle);
                    boxLinePos.position = { x - ((titleWidth / 2) + (boxLineWidth / 2)) + angleRatio, y + (-(boxLineWidth / 2) * std::sin(angle)), z };
                    break;
                case TM:
                    boxLineHight = lineLength;
                    boxLineWidth = margin;
                    block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { boxLineWidth, boxLineHight }, material_box_line));
                    boxLinePos.orientation = Quaternion::RotationAxisAngle({ 0, 0, 1 }, angle);
                    boxLinePos.position = { x + ((boxLineHight / 2) * std::sin(angle)), y + ((titleHeight / 2) + (boxLineHight / 2)) - angleRatio, z };
                    break;
                case RM:
                    boxLineHight = margin;
                    boxLineWidth = lineLength;
                    block.Object = AddObject(engine::CreateQuad(m_context.PbrResources, { boxLineWidth, boxLineHight }, material_box_line));
                    boxLinePos.orientation = Quaternion::RotationAxisAngle({ 0, 0, 1 }, angle);
                    boxLinePos.position = { x + ((titleWidth / 2) + (boxLineWidth / 2)) - angleRatio, y + ((boxLineWidth / 2) * std::sin(angle)), z };
                    break;
                default:
                    break;
                };

                block.Object->Pose() = boxLinePos;
                block.Object->SetParent(m_background);
            };

            auto createLabel = [&](TextBlock& block, const std::string& gerText, const std::string& engText, float fontSizeGer, float fontSizeEng, float x, float y, float z, float lineLength, BoxLinePos pos, float angle)
            {
                if (m_language == GER)
                {
                    m_title.Text = fmt::format(gerText).c_str();
                    textInfo.FontSize = fontSizeGer;
                }
                else if (m_language == ENG)
                {
                    m_title.Text = fmt::format(engText).c_str();
                    textInfo.FontSize = fontSizeEng;
                }
                placeTextBlock(block, x, y, z);
                placeBoxLine(block, x, y, z, lineLength, pos, angle);
            };

            createLabel(m_title, "Haarschaft", "Scapus", 34.0f, 34.0f, -0.17f, -0.08f, 0, 0.06f, RM, -0.174f);
            createLabel(m_title, "Haarwurzel", "Hair root", 34.0f, 34.0f, -0.17f, -0.12f, 0, 0.108f, RM, -0.349f);
            createLabel(m_title, "Talgdruese", "Sebaceous gland", 34.0f, 34.0f, -0.17f, -0.16f, 0, 0.11f, RM, -0.0872f);
            createLabel(m_title, "Pilarmuskel", "Arrector pili muscle", 34.0f, 34.0f, -0.17f, -0.2f, 0, 0.11f, RM, 0.0524f);
            createLabel(m_title, "Schweissdruese", "Sweat gland", 34.0f, 34.0f, -0.17f, -0.24f, 0, 0.055f, RM, 0.349f);
            createLabel(m_title, "Hornschicht", "Stratum corneum", 34.0f, 34.0f, -0.05f, -0.05f, 0, 0.055f, BM, 0);
            createLabel(m_title, "Glanzschicht", "Stratum lucidum", 34.0f, 34.0f, 0.045f, -0.05f, 0, 0.065f, BM, -0.384f);
            createLabel(m_title, "Koernerschicht", "Stratum granulosum", 34.0f, 34.0f, 0.135f, -0.065f, 0, 0.088f, LM, 0.698f);
            createLabel(m_title, "Stachelzellschicht", "Stratum spinosum", 34.0f, 34.0f, 0.17f, -0.1f, 0, 0.1f, LM, 0.253f);
            createLabel(m_title, "Basalschicht", "Stratum basale", 34.0f, 34.0f, 0.17f, -0.13f, 0, 0.09f, LM, -0.0349f);
            createLabel(m_title, "Krausescher Endkolben", "Krause's corpuscle", 30.0f, 30.0f, 0.17f, -0.16f, 0, 0.082f, LM, -0.279f);
            createLabel(m_title, "Vater-Pacinisches Lamellenkoerperchen", "Vater-Pacini corpuscle", 28.0f, 28.0f, 0.17f, -0.19f, 0, 0.14f, LM, 0);
            createLabel(m_title, "Fettzellen", "Lipocytes", 34.0f, 34.0f, 0.17f, -0.22f, 0, 0.045f, LM, 0);
            createLabel(m_title, "Nerv", "Nerve", 34.0f, 34.0f, 0.17f, -0.25f, 0, 0.15f, LM, -0.0523f);
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            XrActionStateBoolean state{ XR_TYPE_ACTION_STATE_BOOLEAN };
            XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
            getInfo.action = m_selectAction;
            CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &state));
            const bool isSelectPressed = state.isActive && state.changedSinceLastSync && state.currentState;
            static int airTapCounter = 0;

            if (isSelectPressed)
            {
                airTapCounter++;

                if (airTapCounter == 1)
                {
                    XrSpaceLocation viewInApp = { XR_TYPE_SPACE_LOCATION };
                    CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInApp));
                    if (Pose::IsPoseValid(viewInApp)) {
                        XrPosef titleInView = { {0, 0, 0, 1}, {0, 0, -1.0f} }; // 1.0 meter in front
                        XrPosef titleInApp = titleInView * viewInApp.pose;
                        XrVector3f forward = titleInApp.position - viewInApp.pose.position;
                        m_targetPose = Pose::LookAt(titleInApp.position, forward, { 0, 1, 0 });
                        m_targetPose.position.y += 0; // floating in the top of user's view

                        if (!m_background->IsVisible()) {
                            m_background->SetVisible(true);
                            m_background->Pose() = m_targetPose;
                        }
                    }
                }
				else if (airTapCounter >= 2)
				{
                    // This takes the user back to the 2D XAML view. Unfortunately, it also crashes the application.
                    windows::CoreApplicationView currentView = windows::CoreApplication::GetCurrentView();
                    currentView.CoreWindow().Close();
				}
            }
        }

    private:
        struct TextBlock {
            std::string Text;
            std::shared_ptr<engine::PbrModelObject> Object;
        };

        xr::SpaceHandle m_viewSpace;
        std::shared_ptr<engine::PbrModelObject> m_background;
        TextBlock m_title;
        TextBlock m_subtitle;
        TextBlock m_closeBlock;
        XrPosef m_targetPose;

        XrAction m_selectAction{ XR_NULL_HANDLE };

        AppLanguage m_language;
    };
} // namespace

std::unique_ptr<engine::Scene> CreateSkinModelLabels(engine::Context& context, AppLanguage language)
{
    return std::make_unique<SkinBlockModelLabels>(context, language);
}