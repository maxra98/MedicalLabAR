#pragma once

#include "MainPage.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void InitializeComponent();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void PaneToggleButton(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
        void SectionHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::MedicalLabAR::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
