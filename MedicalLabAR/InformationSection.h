#pragma once

#include "InformationSection.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct InformationSection : InformationSectionT<InformationSection>
    {
        InformationSection() 
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
    struct InformationSection : InformationSectionT<InformationSection, implementation::InformationSection>
    {
    };
}
