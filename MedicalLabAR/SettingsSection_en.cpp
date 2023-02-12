#include "pch.h"
#include "SettingsSection_en.h"
#if __has_include("SettingsSection_en.g.cpp")
#include "SettingsSection_en.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::MedicalLabAR::implementation
{
    int32_t SettingsSection_en::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SettingsSection_en::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void SettingsSection_en::SaveButtonHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        hstring ger_str = L"German";
        hstring eng_str = L"English";

        Windows::UI::Xaml::Controls::Frame rootFrame = { nullptr };
        rootFrame = Windows::UI::Xaml::Controls::Frame();

        winrt::Windows::Foundation::IPropertyValue selected_str = ComboBoxContent().SelectionBoxItem().as<winrt::Windows::Foundation::IPropertyValue>();

        if (selected_str.GetString() == eng_str)
        {
            ContentDialog dialog;
            dialog.Title(box_value(L"Language selection"));
            dialog.Content(box_value(L"The language you selected is already in use."));
            dialog.CloseButtonText(L"OK");
            dialog.ShowAsync();
        }
        else if (selected_str.GetString() == ger_str)
        {
            rootFrame.Navigate(xaml_typename<MedicalLabAR::MainPage>());
            // Place the frame in the current Window
            Window::Current().Content(rootFrame);
            // Ensure the current window is active
            Window::Current().Activate();
        }
        else
        {
            ContentDialog dialog;
            dialog.Title(box_value(L"Error"));
            dialog.Content(box_value(L"The selected language does not correspond to the available languages of the application."));
            dialog.CloseButtonText(L"OK");
            dialog.ShowAsync();
        }
    }
}
