#include "pch.h"
#include "SkinBlockModelSection_en.h"
#if __has_include("SkinBlockModelSection_en.g.cpp")
#include "SkinBlockModelSection_en.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::MedicalLabAR::implementation
{
    int32_t SkinBlockModelSection_en::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SkinBlockModelSection_en::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

	void SkinBlockModelSection_en::SkinBlockModelButtonHandler(IInspectable const&, RoutedEventArgs const&)
	{
		auto newView = windows::CoreApplication::CreateNewView(winrt::make<AppViewSource>(ENG));

		newView.Dispatcher().RunAsync(
			windows::CoreDispatcherPriority::Normal, windows::DispatchedHandler([this]()
				{
					auto viewId = windows::ApplicationView::GetForCurrentView().Id();

					windows::CoreWindow::GetForCurrentThread().Activate();

					windows::CoreApplication::MainView().Dispatcher().RunAsync(
						windows::CoreDispatcherPriority::Normal, windows::DispatchedHandler([viewId]()
							{
								auto asyncAction = windows::ApplicationViewSwitcher::SwitchAsync(
									viewId,
									windows::ApplicationView::GetForCurrentView().Id());
							}));
				}));
	}
}
