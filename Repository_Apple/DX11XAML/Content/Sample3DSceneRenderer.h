#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include <math.h>

namespace DX11XAML_Orient_App
{
	// this indecator realize a basical render tube
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		void SwitchMoving();
		void PushTotalRoad(float *outer) { *outer = m_totalRad; }
		void SetRotationSpeed(float speed) { m_rotatingSpeed = speed; }
		bool IsTracking() { return m_tracking; }

	private:
		void Rotate(float radians);
		void RotateXOY(float radians);

	private:
		//restored device resource
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		//D3D resource from stereoscopic
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// system resource of cube
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// vars used for rendering the cycle
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
		bool	m_MovingAva;

		//the object's counter vars
		float m_runtime;
		float m_totalRad;

		//controller vars
		float m_rotatingSpeed;

	};
}

