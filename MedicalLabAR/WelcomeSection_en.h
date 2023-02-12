#pragma once

#include "WelcomeSection_en.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct WelcomeSection_en : WelcomeSection_enT<WelcomeSection_en>
    {
        WelcomeSection_en() 
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::MedicalLabAR::factory_implementation
{
    struct WelcomeSection_en : WelcomeSection_enT<WelcomeSection_en, implementation::WelcomeSection_en>
    {
    };
}
