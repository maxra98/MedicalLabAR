#include "pch.h"
#include "InformationSection.h"
#if __has_include("InformationSection.g.cpp")
#include "InformationSection.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MedicalLabAR::implementation
{
    int32_t InformationSection::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void InformationSection::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
