#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "MySceneRender.h"

//Show the D2D and D3D contains on the screen
namespace DX11XAML_Orient_App
{
	class DX11XAML_Orient_AppMain : public DX::IDeviceNotify
	{
	public:
		DX11XAML_Orient_AppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~DX11XAML_Orient_AppMain();
		void CreateWindowSizeDependentResources();
		void SwitchTracking() { m_sceneRenderer->SwitchMoving(); }
		void StartTracking(float pressposX) { m_sceneRenderer->StartTracking(); m_pointerPressX = pressposX; }
		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }
		void TrackingUpdateVertical(float positionY) { m_pointerLocationY = positionY; }
		void Get3DSceneCubeRuntime(float runtime) { m_runtime = runtime; }
		void SetRotationSpeed(float speed) { m_sceneRenderer->SetRotationSpeed(speed); }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		//Resotored device resource pointer
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		//replace as ur content indecator
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		//the cycling timer
		DX::StepTimer m_timer;

		//vars changes with envior;
		float m_pointerPressX;
		float m_runtime;

		//the mouse position
		float m_pointerLocationX;
		float m_pointerLocationY;

		//attached stereoscopic(s)
		std::unique_ptr<MyStereoscopic> m_theGreatFirewall;
	};
}