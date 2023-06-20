#include "Toon.hlsli"

VertexOutput main(VertexInput vin)
{
	VertexOutput vout;
	vout.sv_position = mul(float4(vin.position, 1), mul(world, viewProjection));
	vout.position = mul(float4(vin.position, 1), world).xyz;
	vout.normal = mul(float4(vin.normal, 0), world).xyz;
	return vout;
}
