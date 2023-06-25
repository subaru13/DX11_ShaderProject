struct LAYOUT
{
	float4 sv_position : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
};

cbuffer ConstantData : register(b0)
{
	float4x4 world;
	float4x4 viewProjection;
	float4 color;
	float divNum;
	float move;
	float scale;
	float rotation;
};