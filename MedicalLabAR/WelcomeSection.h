#pragma once

#include "WelcomeSection.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct WelcomeSection : WelcomeSectionT<WelcomeSection>
    {
        WelcomeSection() 
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
    struct WelcomeSection : WelcomeSectionT<WelcomeSection, implementation::WelcomeSection>
    {
    };
}
