#pragma once

#include "SkinBlockModelSection_en.g.h"
#include "AppLanguage.h"
#include "AppView.h"

namespace winrt::MedicalLabAR::implementation
{
    struct SkinBlockModelSection_en : SkinBlockModelSection_enT<SkinBlockModelSection_en>
    {
        SkinBlockModelSection_en() 
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void SkinBlockModelButtonHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::MedicalLabAR::factory_implementation
{
    struct SkinBlockModelSection_en : SkinBlockModelSection_enT<SkinBlockModelSection_en, implementation::SkinBlockModelSection_en>
    {
    };
}
