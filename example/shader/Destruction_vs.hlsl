#include "Destruction.hlsli"
LAYOUT main(float4 position : POSITION, float4 normal : NORMAL)
{
	LAYOUT vout;
	vout.sv_position = mul(position, mul(world, viewProjection));
	vout.position = position.xyz;
	vout.normal = normal.xyz;
	return vout;
}