#pragma once
#include <DirectXMath.h>
using namespace DirectX;

//the property of the whole object
struct ConstructProperty
{
	XMFLOAT4X4 obj;
	XMFLOAT4X4 project;
};
//property of the vertex
struct VertexProperty
{
	XMFLOAT3 vertex;
	XMFLOAT3 color;
};

struct LineProperty
{
	XMFLOAT4 vector;
	XMFLOAT3 vertex;
	XMFLOAT3 color;
};