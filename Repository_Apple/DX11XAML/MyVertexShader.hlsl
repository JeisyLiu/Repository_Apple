cbuffer MVP_Buffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShader_input
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct PixelShader_input
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

PixelShader_input mVertexShader(VertexShader_input input)
{
	PixelShader_input output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.color = input.color;

	return output;
}
