struct PixelInput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 color : COLOR;
};

Texture2D colorMap : register(t0);
SamplerState linearSamplerState : register(s0);

float4 main(PixelInput pin) : SV_TARGET
{
	return colorMap.Sample(linearSamplerState, pin.texcoord) * pin.color;
}
