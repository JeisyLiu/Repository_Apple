#pragma once
#include "Common\DeviceResources.h"
#include "Common\StepTimer.h"
#include "ObjectProperty.h"
using namespace Microsoft::WRL;
namespace DX11XAML_Orient_App
{
	class MyStereoscopic
	{
	public:
		MyStereoscopic(const std::shared_ptr<DX::DeviceResources>&deviceResource);
		void CreateDeviceDependsResource();
		void ReleaseDeviceResource();
		void AddNewObject();
		//void CreateWindowSizeDependsResource();
		void Render();
	private:

		std::shared_ptr<DX::DeviceResources> m_deviceResource;

		ComPtr<ID3D11InputLayout> m_inputLayout;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_contentsBuffer;
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;

		uint32 m_indexCounts;
		ConstructProperty m_structProperty;
		bool m_loading;
	};
}