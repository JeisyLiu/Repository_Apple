#include "pch.h"
#include "MySceneRender.h"
#include "Common\DirectXHelper.h"

using namespace Windows;
using namespace Windows::Foundation;
using namespace DX11XAML_Orient_App;


/*
void MyStereoscopic::CreateWindowSizeDependsResource()
{
	//Optimize size of the window indecator
	Size outputSize = m_deviceResource->GetOutputSize();
	float propotion = outputSize.Width / outputSize.Height;
	float angleY = 70.0f*XM_PI / 180.0f;
	if (propotion < 1.0f)angleY *= 2.0f;
	// set the forematrix
	XMMATRIX foresetMatrix = XMMatrixPerspectiveFovRH(angleY, propotion, 0.01f, 100.0f);
	//now multiply the matrix depend on math
	XMFLOAT4X4 orientation = m_deviceResource->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
	XMStoreFloat4x4(&m_structProperty.project,
		XMMatrixTranspose(orientationMatrix * foresetMatrix)
	);
}
*/
void MyStereoscopic::AddNewObject()
{
}

void MyStereoscopic::Render()
{
	if (m_loading)return;
	//render
	auto context = m_deviceResource->GetD3DDeviceContext();
	context->UpdateSubresource1(
		m_contentsBuffer.Get(), 0, NULL, &m_structProperty, 0, 0, 0);

	UINT stide = sizeof(VertexProperty);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &stide, &offset);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	context->VSSetConstantBuffers1(0, 1, m_contentsBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(m_indexCounts, 0, 0);
}
void MyStereoscopic::CreateDeviceDependsResource()
{
	auto loadvertextask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadpixeltask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	//create vertex shader after loading the the file
	auto createvs = loadvertextask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResource->GetD3DDevice()->CreateVertexShader(
			&fileData[0], fileData.size(), nullptr, &m_vertexShader)
		);
		// set vertexdesc for
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[]=
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
		};
		DX::ThrowIfFailed(m_deviceResource->GetD3DDevice()->CreateInputLayout(
		vertexDesc,ARRAYSIZE(vertexDesc),&fileData[0],fileData.size(),&m_inputLayout)
		);
	});
	auto createps = loadpixeltask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResource->GetD3DDevice()->CreatePixelShader(
			&fileData[0], fileData.size(), nullptr, &m_pixelShader));
		
		CD3D11_BUFFER_DESC bufferDesc(sizeof(ConstructProperty),D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResource->GetD3DDevice()->CreateBuffer(
			&bufferDesc, nullptr, &m_contentsBuffer));
	});
	//create the static stereoscopic object
	auto createWall = (createps&&createvs).then([this]() {
		static const VertexProperty myWallVerties[] =
		{
			{XMFLOAT3(-5.0f, 0.0f,-3.0f),XMFLOAT3( 0.f,0.0f,0.0f)},	//0
			{XMFLOAT3(-5.0f, 0.0f,-2.5f),XMFLOAT3( 1.f,1.f,1.f)},
			{XMFLOAT3(-5.0f,10.0f,-3.0f),XMFLOAT3( 0.f,0.0f,0.0f)},
			{XMFLOAT3(-5.0f,10.0f,-2.5f),XMFLOAT3( 1.f,1.f,1.f)},
			{XMFLOAT3( 5.0f,10.0f,-3.0f),XMFLOAT3( 1.f,1.f,1.f)},	//4
			{XMFLOAT3( 5.0f,10.0f,-2.5f),XMFLOAT3( 0.f,0.0f,0.0f)},
			{XMFLOAT3( 5.0f, 0.0f,-3.0f),XMFLOAT3( 0.f,0.0f,0.0f)},
			{XMFLOAT3( 5.0f, 0.0f,-2.5f),XMFLOAT3( 1.f,1.f,1.f)}
		};
		//deal the subresource data of d3d11
		D3D11_SUBRESOURCE_DATA vertexBuffer = { 0 };
		vertexBuffer.pSysMem = myWallVerties;
		vertexBuffer.SysMemPitch = 0;
		vertexBuffer.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(myWallVerties), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResource->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBuffer,
				&m_vertexBuffer)
		); 

		static const unsigned short WallIndices[] =
		{
			0,1,2,
			1,3,2,

			4,5,7,
			4,7,6,

			0,1,7,
			0,7,6,

			2,3,5,
			2,5,4,
			
			0,2,4,
			0,4,6,

			1,3,5,
			1,5,7,
		};
		m_indexCounts = ARRAYSIZE(WallIndices);
		//deal the subresource data of d3d11
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = WallIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC wallbuffer(sizeof(indexBufferData), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResource->GetD3DDevice()->CreateBuffer(
				&wallbuffer,
				&indexBufferData,
				&m_indexBuffer
			)
		);

	});
	createWall.then([this]() { m_loading = false; });
}

void MyStereoscopic::ReleaseDeviceResource()
{
	m_loading = true;
	m_contentsBuffer.Reset();
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
	m_pixelShader.Reset();
	m_vertexShader.Reset();
	m_inputLayout.Reset();
}


MyStereoscopic::MyStereoscopic(const std::shared_ptr<DX::DeviceResources> &deviceResource) :
	m_loading(true),
	m_deviceResource(deviceResource),
	m_indexCounts(0)
{
	CreateDeviceDependsResource();
}
