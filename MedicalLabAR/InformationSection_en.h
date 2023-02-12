#pragma once

#include "InformationSection_en.g.h"

namespace winrt::MedicalLabAR::implementation
{
    struct InformationSection_en : InformationSection_enT<InformationSection_en>
    {
        InformationSection_en() 
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
    struct InformationSection_en : InformationSection_enT<InformationSection_en, implementation::InformationSection_en>
    {
    };
}
