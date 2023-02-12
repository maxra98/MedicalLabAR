#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MedicalLabAR::implementation
{
    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

	void MainPage::InitializeComponent()
	{
		// Call base InitializeComponent() to register with the Xaml runtime
		MainPageT::InitializeComponent();
		// Can now access Xaml properties
		MainPage::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::WelcomeSection>());
	}

	void MainPage::PaneToggleButton(IInspectable const&, RoutedEventArgs const&)
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

	void MainPage::SectionHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (info_section().IsSelected() == true)
		{
			MainPage::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::InformationSection>());
		}
		else if (skin_section().IsSelected() == true)
		{
			MainPage::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::SkinBlockModelSection>());
		}
		else if (settings_section().IsSelected() == true)
		{
			MainPage::ScenarioFrame().Navigate(winrt::xaml_typename<MedicalLabAR::SettingsSection>());
		}
	}
}
