#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>

#include <sdkddkver.h>

#include <exception>
#include <memory>
#include <tuple>
#include <vector>
#include <map>
#include <array>
#include <chrono>
#include <string>
#include <atomic>
#include <future>
#include <pathcch.h>
#include <initializer_list>
#include <optional>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <XrUtility/XrError.h>
#include <XrUtility/XrMath.h>
#include <XrUtility/XrHandle.h>

#define ENABLE_GLOBAL_XR_DISPATCH_TABLE
#include <XrUtility/XrDispatchTable.h>

#include <winrt/base.h> // for winrt::com_ptr

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
