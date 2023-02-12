#pragma once

#include <Unknwn.h> // Required to interop with IUnknown. Must be included before C++/WinRT headers.
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.Preview.Holographic.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Graphics.Holographic.h>

#include "AppLanguage.h"

namespace windows {
    using namespace winrt::Windows::ApplicationModel::Activation;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::Text::Core;
    using namespace winrt::Windows::UI::ViewManagement;
    using namespace winrt::Windows::Graphics::Holographic;
    using namespace winrt::Windows::ApplicationModel::Preview::Holographic;
} // namespace windows

struct AppView : winrt::implements<AppView, windows::IFrameworkView> {
    AppView(AppLanguage language);
    void Initialize(windows::CoreApplicationView const& applicationView);
    void Load(winrt::hstring const& entryPoint);
    void Uninitialize();
    void OnActivated(windows::CoreApplicationView const&, windows::IActivatedEventArgs const& args);
    void Run();
    void SetWindow(windows::CoreWindow const& window);
    void InitializeTextEditingContext();
    void OnKeyDown(windows::CoreWindow const& sender, windows::KeyEventArgs const& args);
    void OnWindowClosed(windows::CoreWindow const& sender, windows::CoreWindowEventArgs const& args);

private:
    bool m_windowClosed{ false };
    AppLanguage m_language;
};

struct AppViewSource : winrt::implements<AppViewSource, windows::IFrameworkViewSource> {
    AppViewSource(AppLanguage language);
    windows::IFrameworkView CreateView();

private:
    AppLanguage m_language;
};