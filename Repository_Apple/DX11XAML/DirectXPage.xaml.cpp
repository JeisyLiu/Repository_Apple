//
// DirectXPage.xaml.cpp
// DirectXPage 类的实现。
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace DX11XAML_Orient_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr),
	m_rotationSpeed(1.0f)
{
	InitializeComponent();

	// 注册页面生命周期的事件处理程序。
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// we can create resource connected to device now because we got 
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// register our SwapChainPanel attain the independent input pointer event
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// as for targeted device type，whatever which thread it created by，CoreIndependentInputSource will occur the event
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// mouse pointer events' register will be activated at the back thread
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);
		
		//Events from keyboard

		// 一旦发送输入消息，即开始处理它们。once send the Input
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// 在高优先级的专用后台线程上运行任务。
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<DX11XAML_Orient_AppMain>(new DX11XAML_Orient_AppMain(m_deviceResources));
	m_main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// 析构时停止渲染和处理事件。
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// 针对挂起和终止事件保存应用程序的当前状态。
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// 挂起应用程序时停止渲染。
	m_main->StopRenderLoop();

	// 在此处放置代码以保存应用程序状态。
}

// 针对恢复事件加载应用程序的当前状态。
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// 在此处放置代码以加载应用程序状态。

	// 恢复应用程序时开始渲染。
	m_main->StartRenderLoop();
}

// 窗口事件处理程序。

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation 事件处理程序。

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// 注意: 在此处检索到的 LogicalDpi 值可能与应用的有效 DPI 不匹配
	// 如果正在针对高分辨率设备对它进行缩放。在 DeviceResources 上设置 DPI 后，
	// 应始终使用 GetDpi 方法进行检索。
	// 有关详细信息，请参阅 DeviceResources.cpp。
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// 在单击应用程序栏按钮时调用。
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// 使用应用程序栏(如果它适合你的应用程序)。设计应用程序栏，
	// 然后填充事件处理程序(与此类似)。
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// 按下指针时开始跟踪指针移动。
	m_main->StartTracking(e->CurrentPoint->Position.X);
}

void DirectXPage::OnPointerMoved(Object^sender, PointerEventArgs^ e)
{
	m_main->TrackingUpdate(e->CurrentPoint->Position.X);
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// 释放指针时停止跟踪指针移动。
	m_main->StopTracking();
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}


void DirectXPage::Btn1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->SwitchTracking();
	String^speed = RotateSpeed->Text;
	
}


void DX11XAML_Orient_App::DirectXPage::RotateSpeed_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
}


void DX11XAML_Orient_App::DirectXPage::SpeedUp(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_rotationSpeed += 0.25f;
	m_main->SetRotationSpeed(m_rotationSpeed);
}


void DX11XAML_Orient_App::DirectXPage::SpeedDown(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_rotationSpeed -= 0.25f;
	m_main->SetRotationSpeed(m_rotationSpeed);
}
