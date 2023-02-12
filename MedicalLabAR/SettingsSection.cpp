#include "pch.h"
#include "SettingsSection.h"
#if __has_include("SettingsSection.g.cpp")
#include "SettingsSection.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::MedicalLabAR::implementation
{
    int32_t SettingsSection::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SettingsSection::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void SettingsSection::SaveButtonHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        hstring ger_str = L"Deutsch";
        hstring eng_str = L"Englisch";

        Windows::UI::Xaml::Controls::Frame rootFrame = { nullptr };
        rootFrame = Windows::UI::Xaml::Controls::Frame();

        winrt::Windows::Foundation::IPropertyValue selected_str = ComboBoxContent().SelectionBoxItem().as<winrt::Windows::Foundation::IPropertyValue>();

        if (selected_str.GetString() == eng_str)
        {
            rootFrame.Navigate(xaml_typename<MedicalLabAR::MainPage_en>());
            // Place the frame in the current Window
            Window::Current().Content(rootFrame);
            // Ensure the current window is active
            Window::Current().Activate();
        }
        else if (selected_str.GetString() == ger_str)
        {
            ContentDialog dialog;
            dialog.Title(box_value(L"Sprachauswahl"));
            dialog.Content(box_value(L"Die von Ihnen gewählte Sprache wird bereits verwendet."));
            dialog.CloseButtonText(L"OK");
            dialog.ShowAsync();
        }
        else
        {
            ContentDialog dialog;
            dialog.Title(box_value(L"Fehler"));
            dialog.Content(box_value(L"Die ausgewählte Sprache entspricht nicht den verfügbaren Sprachen der Applikation."));
            dialog.CloseButtonText(L"OK");
            dialog.ShowAsync();
        }
    }
}
