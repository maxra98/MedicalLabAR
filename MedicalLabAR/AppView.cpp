#include "pch.h"
#include "AppView.h"

#include <XrSceneLib/XrApp.h>
#include <SampleShared/Trace.h>

std::unique_ptr<engine::Scene> CreateSkinModelLabels(engine::Context& context, AppLanguage language);
std::unique_ptr<engine::Scene> CreateModelAlignment(engine::Context& context);

std::unique_ptr<engine::XrApp> CreateUwpXrApp(XrHolographicWindowAttachmentMSFT&& holographicWindowAttachment, AppLanguage language) {
    engine::XrAppConfiguration appConfig({ "MedicalLabAR", 2 });
    appConfig.HolographicWindowAttachment = std::move(holographicWindowAttachment);

    appConfig.RequestedExtensions.push_back(XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_EXT_HAND_JOINTS_MOTION_RANGE_EXTENSION_NAME);
    appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);

    // NOTE: Uncomment a filter below to test specific action binding of given profile.
    // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/microsoft/hand_interaction");
    // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/khr/simple_controller");

    auto app = CreateXrApp(appConfig);

    app->AddScene(CreateSkinModelLabels(app->Context(), language));
    app->AddScene(CreateModelAlignment(app->Context()));

    return app;
}

AppView::AppView(AppLanguage language) : m_language(language) {}

void AppView::Initialize(windows::CoreApplicationView const& applicationView)
{
    applicationView.Activated({ this, &AppView::OnActivated });
}

void AppView::Load(winrt::hstring const& entryPoint)
{
}

void AppView::Uninitialize()
{
}

void AppView::OnActivated(windows::CoreApplicationView const&, windows::IActivatedEventArgs const& args)
{
    if (args.Kind() == windows::ActivationKind::Protocol) {
        windows::ProtocolActivatedEventArgs eventArgs{ args.as<windows::ProtocolActivatedEventArgs>() };
        sample::Trace("Protocol uri : {}", winrt::to_string(eventArgs.Uri().RawUri()).c_str());
    }

    // Inspecting whether the application is launched from within holographic shell or from desktop.
    if (windows::HolographicApplicationPreview::IsHolographicActivation(args)) {
        sample::Trace("App activation is targeted at the holographic shell.");
    }
    else {
        sample::Trace("App activation is targeted at the desktop.");
    }
}

void AppView::Run()
{
    // Creating a HolographicSpace before activating the CoreWindow to make it a holographic window
    windows::CoreWindow window = windows::CoreWindow::GetForCurrentThread();
    windows::HolographicSpace holographicSpace = windows::HolographicSpace::CreateForCoreWindow(window);
    window.Activate();

    windows::ApplicationView newAppView = windows::ApplicationView::GetForCurrentView();

    XrHolographicWindowAttachmentMSFT holographicWindowAttachment{ XR_TYPE_HOLOGRAPHIC_WINDOW_ATTACHMENT_MSFT };
    holographicWindowAttachment.coreWindow = window.as<::IUnknown>().get();
    holographicWindowAttachment.holographicSpace = holographicSpace.as<::IUnknown>().get();

    std::unique_ptr<engine::XrApp> app = CreateUwpXrApp(std::move(holographicWindowAttachment), m_language);

    while (!m_windowClosed && app->Step()) {
        window.Dispatcher().ProcessEvents(windows::CoreProcessEventsOption::ProcessAllIfPresent);
    }
}

void AppView::SetWindow(windows::CoreWindow const& window)
{
    InitializeTextEditingContext();
    window.KeyDown({ this, &AppView::OnKeyDown });
    window.Closed({ this, &AppView::OnWindowClosed });
}

void AppView::InitializeTextEditingContext()
{
    // This sample customizes the text input pane with manual display policy and email address scope.
    windows::CoreTextServicesManager manager = windows::CoreTextServicesManager::GetForCurrentView();
    windows::CoreTextEditContext editingContext = manager.CreateEditContext();
    editingContext.InputPaneDisplayPolicy(windows::CoreTextInputPaneDisplayPolicy::Manual);
    editingContext.InputScope(windows::CoreTextInputScope::EmailAddress);
}

void AppView::OnKeyDown(windows::CoreWindow const& sender, windows::KeyEventArgs const& args)
{
    // This sample toggles the software keyboard in HMD using space key
    if (args.VirtualKey() == winrt::Windows::System::VirtualKey::Space) {
        windows::InputPane inputPane = windows::InputPane::GetForCurrentView();
        if (inputPane.Visible()) {
            const bool hidden = inputPane.TryHide();
            sample::Trace("InputPane::TryHide() -> {}", hidden);
        }
        else {
            const bool shown = inputPane.TryShow();
            sample::Trace("InputPane::TryShow() -> {}", shown);
        }
    }
}

void AppView::OnWindowClosed(windows::CoreWindow const& sender, windows::CoreWindowEventArgs const& args)
{
    m_windowClosed = true;
}

AppViewSource::AppViewSource(AppLanguage language) : m_language(language) {}

windows::IFrameworkView AppViewSource::CreateView()
{
    return winrt::make<AppView>(m_language);
}