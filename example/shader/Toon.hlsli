cbuffer ConstantData : register(b0)
{
	row_major float4x4 world;
	row_major float4x4 viewProjection;
	float4 matColor;
	float4 rimColor;
	float3 lightDir;
	int toonLevels;
	float3 eyePos;
	float outlineThreshold;
};

struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VertexOutput
{
	float4 sv_position : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
};
typedef VertexOutput PixelInput;
