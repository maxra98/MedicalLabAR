﻿#pragma once

#include "SettingsSection_en.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct SettingsSection_en : SettingsSection_enT<SettingsSection_en>
    {
        SettingsSection_en() 
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void SaveButtonHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::MedicalLabAR::factory_implementation
{
    struct SettingsSection_en : SettingsSection_enT<SettingsSection_en, implementation::SettingsSection_en>
    {
    };
}
