#include "pch.h"
#include "PbrModelObject.h"
#include "TextTexture.h"
#include "Scene.h"
#include "BoxLinePos.h"

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    struct SkinBlockModelAlignment : public engine::Scene {

        SkinBlockModelAlignment(engine::Context& context) : Scene(context)
        {
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("model_alignment", "Model alignment action");

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

            constexpr float margin = 0.005f;
            constexpr float axisLength = 0.1f;
            constexpr float axisThickness = 0.01f;

            // Creates the background block for the text
            const auto& material = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::FromSRGB(Colors::Transparent));
            m_background = AddObject(engine::CreateQuad(m_context.PbrResources, { axisLength, axisThickness }, material));
            m_background->SetVisible(false);

            // Creates axis attached to the TextBlock item
            auto placeAxis = [&](AxisObj& obj) {

                // Upper cube
                // Skin block model sizes: x = 18 cm, y = 15 cm, z = 8.5 cm
                XrPosef upperCubePos;
                obj.Object = AddObject(engine::CreateCube(m_context.PbrResources, { 0.18f, 0.15f, 0.085f }, Pbr::FromSRGB(Colors::Blue), 1.0f, 1.0f));
                upperCubePos.position = { 0, -0.2f, 0 };
                upperCubePos.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -0.523f);
                obj.Object->Pose() = upperCubePos;
                obj.Object->SetParent(m_background);

                // Lower cube
                XrPosef lowerCubePos;
                obj.Object = AddObject(engine::CreateCube(m_context.PbrResources, { 0.225f, 0.023f, 0.16f }, Pbr::FromSRGB(Colors::Gray), 1.0f, 1.0f));
                lowerCubePos.position = { 0, -0.275f, 0 };
                lowerCubePos.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -0.523f);
                obj.Object->Pose() = lowerCubePos;
                obj.Object->SetParent(m_background);

                // Axis left front corner
                XrPosef firstAxisPos;
                obj.Object = AddObject(engine::CreateAxis(m_context.PbrResources, 0.023f, 0.005f, 1.0f, 0.01f));
                firstAxisPos.position = { -0.138f, -0.2865f, 0.014f };
                firstAxisPos.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, 1.047f);
                obj.Object->Pose() = firstAxisPos;
                obj.Object->SetParent(m_background);

                // Axis right front corner
                XrPosef secondAxisPos;
                obj.Object = AddObject(engine::CreateAxis(m_context.PbrResources, 0.023f, 0.005f, 1.0f, 0.01f));
                secondAxisPos.position = { 0.058f, -0.2865f, 0.126f };
                secondAxisPos.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -3.665f);
                obj.Object->Pose() = secondAxisPos;
                obj.Object->SetParent(m_background);

                // Axis right back corner
                XrPosef thirdAxisPos;
                obj.Object = AddObject(engine::CreateAxis(m_context.PbrResources, 0.023f, 0.005f, 1.0f, 0.01f));
                thirdAxisPos.position = { 0.14f, -0.2865f, -0.012f };
                thirdAxisPos.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -2.094f);
                obj.Object->Pose() = thirdAxisPos;
                obj.Object->SetParent(m_background);
            };
            placeAxis(m_obj);

            // Uncomment if model alignment view should hide after air tap
            m_background->SetVisible(true);
        }

        void OnUpdate(const engine::FrameTime& frameTime) override
        {
            XrActionStateBoolean state{ XR_TYPE_ACTION_STATE_BOOLEAN };
            XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
            getInfo.action = m_selectAction;
            CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &state));
            const bool isSelectPressed = state.isActive && state.changedSinceLastSync && state.currentState;

            if (m_background->IsVisible() && !isSelectPressed)
            {
                XrSpaceLocation viewInApp = { XR_TYPE_SPACE_LOCATION };
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInApp));
                if (Pose::IsPoseValid(viewInApp)) {
                    XrPosef titleInView = { {0, 0, 0, 1}, {0, 0, -1.0f} }; // 1.0 meter in front
                    XrPosef titleInApp = titleInView * viewInApp.pose;
                    XrVector3f forward = titleInApp.position - viewInApp.pose.position;
                    m_targetPose = Pose::LookAt(titleInApp.position, forward, { 0, 1, 0 });
                    m_targetPose.position.y += 0; // floating in the top of user's view
                    //m_targetPose.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -0.5f);

                    if (!m_background->IsVisible()) {
                        m_background->SetVisible(true);
                        m_background->Pose() = m_targetPose;
                    }
                    else {
                        // Ease the object to the target pose slowly. 98%^90hz~=16%
                        m_background->Pose() = Pose::Slerp(m_background->Pose(), m_targetPose, 0.02f);
                    }
                }
            }
            else
            {
                m_background->SetVisible(false);
            }

            // Uncomment for a static model alignment view
            //XrSpaceLocation viewInApp = { XR_TYPE_SPACE_LOCATION };
            //CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInApp));
            //if (Pose::IsPoseValid(viewInApp)) {
            //    XrPosef titleInView = { {0, 0, 0, 1}, {0, 0, -1.0f} }; // 1.0 meter in front
            //    XrPosef titleInApp = titleInView * viewInApp.pose;
            //    XrVector3f forward = titleInApp.position - viewInApp.pose.position;
            //    m_targetPose = Pose::LookAt(titleInApp.position, forward, { 0, 1, 0 });
            //    m_targetPose.position.y += 0; // floating in the top of user's view
            //    //m_targetPose.orientation = Quaternion::RotationAxisAngle({ 0, 1, 0 }, -0.5f);

            //    if (!m_background->IsVisible()) {
            //        m_background->SetVisible(true);
            //        m_background->Pose() = m_targetPose;
            //    }
            //    else {
            //        // Ease the object to the target pose slowly. 98%^90hz~=16%
            //        //m_background->Pose() = Pose::Slerp(m_background->Pose(), m_targetPose, 0.02f);
            //    }
            //}
        }

    private:
        struct AxisObj {
            std::shared_ptr<engine::PbrModelObject> Object;
        };

        xr::SpaceHandle m_viewSpace;
        std::shared_ptr<engine::PbrModelObject> m_background;
        AxisObj m_obj;
        XrPosef m_targetPose;

        XrAction m_selectAction{ XR_NULL_HANDLE };
    };

} // namespace

std::unique_ptr<engine::Scene> CreateModelAlignment(engine::Context& context)
{
    return std::make_unique<SkinBlockModelAlignment>(context);
}