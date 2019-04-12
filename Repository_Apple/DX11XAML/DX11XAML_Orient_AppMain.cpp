﻿#include "pch.h"
#include "DX11XAML_Orient_AppMain.h"
#include "Common\DirectXHelper.h"

using namespace DX11XAML_Orient_App;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// 加载应用程序时加载并初始化应用程序资产。
DX11XAML_Orient_AppMain::DX11XAML_Orient_AppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_pointerLocationX(0.0f),
	m_pointerLocationY(0.0f)
{
	// 注册以在设备丢失或重新创建时收到通知
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: 将此替换为应用程序内容的初始化。
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: 如果需要默认的可变时间步长模式之外的其他模式，请更改计时器设置。
	// 例如，对于 60 FPS 固定时间步长更新逻辑，请调用:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

DX11XAML_Orient_AppMain::~DX11XAML_Orient_AppMain()
{
	// 取消注册设备通知
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// 在窗口大小更改(例如，设备方向更改)时更新应用程序状态
void DX11XAML_Orient_AppMain::CreateWindowSizeDependentResources() 
{
	// TODO: 将此替换为应用程序内容的与大小相关的初始化。
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void DX11XAML_Orient_AppMain::StartRenderLoop()
{
	// 如果动画呈现循环已在运行，则勿启动其他线程。
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// 创建一个将在后台线程上运行的任务。
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// 计算更新的帧并且在每个场消隐期呈现一次。
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// 在高优先级的专用后台线程上运行任务。
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void DX11XAML_Orient_AppMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// 每帧更新一次应用程序状态。
void DX11XAML_Orient_AppMain::Update() 
{
	ProcessInput();

	// 更新场景对象。
	m_timer.Tick([&]()
	{
		// TODO: 将此替换为应用程序内容的更新函数。
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// 在更新游戏状态之前处理所有用户输入
void DX11XAML_Orient_AppMain::ProcessInput()
{
	// TODO: 按帧输入处理在此处添加。
	m_sceneRenderer->PushTotalRoad(&m_runtime);
	float radians = 6.283185307f * 2.0f *(m_pointerLocationX - m_pointerPressX) / m_deviceResources->GetOutputSize().Width;

	m_sceneRenderer->TrackingUpdate((m_runtime + radians));
}

// show the frame at present depends on application status
// 如果帧已呈现并且已准备好显示，则返回 true。
bool DX11XAML_Orient_AppMain::Render() 
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// 将视区重置为针对整个屏幕。
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// 将呈现目标重置为屏幕。
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// 清除后台缓冲区和深度模具视图。
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::LightYellow);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 呈现场景对象。
	// TODO: 将此替换为应用程序内容的渲染函数。
	m_sceneRenderer->Render();
	//m_theGreatFirewall->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// 通知呈现器，需要释放设备资源。
void DX11XAML_Orient_AppMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	//m_theGreatFirewall->ReleaseDeviceResource();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// 通知呈现器，现在可重新创建设备资源。
void DX11XAML_Orient_AppMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	// m_theGreatFirewall->CreateDeviceDependsResource();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
