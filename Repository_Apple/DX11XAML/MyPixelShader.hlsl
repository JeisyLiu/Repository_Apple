
struct PixelShader_input
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

float4 main(PixelShader_input input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
