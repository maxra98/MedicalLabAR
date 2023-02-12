#include "pch.h"
#include "WelcomeSection.h"
#if __has_include("WelcomeSection.g.cpp")
#include "WelcomeSection.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MedicalLabAR::implementation
{
    int32_t WelcomeSection::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void WelcomeSection::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
