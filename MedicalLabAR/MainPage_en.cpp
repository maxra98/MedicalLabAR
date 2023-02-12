#include "pch.h"
#include "MainPage_en.h"
#if __has_include("MainPage_en.g.cpp")
#include "MainPage_en.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MedicalLabAR::implementation
{
    int32_t MainPage_en::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage_en::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

	void MainPage_en::InitializeComponent()
	{
		// Call base InitializeComponent() to register with the Xaml runtime
		MainPage_enT::InitializeComponent();
		// Can now access Xaml properties
		MainPage_en::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::WelcomeSection_en>());
	}

	void MainPage_en::PaneToggleButton(IInspectable const&, RoutedEventArgs const&)
	{
		if (Splitter().IsPaneOpen())
		{
			Splitter().IsPaneOpen(false);
		}
		else
		{
			Splitter().IsPaneOpen(true);
		}
	}

	void MainPage_en::SectionHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (info_section().IsSelected() == true)
		{
			MainPage_en::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::InformationSection_en>());
		}
		else if (skin_section().IsSelected() == true)
		{
			MainPage_en::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::SkinBlockModelSection_en>());
		}
		else if (settings_section().IsSelected() == true)
		{
			MainPage_en::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::SettingsSection_en>());
		}
	}
}
