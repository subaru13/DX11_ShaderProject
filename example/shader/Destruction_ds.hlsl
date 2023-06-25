#include "Destruction.hlsli"

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
LAYOUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<LAYOUT, NUM_CONTROL_POINTS> patch)
{
	LAYOUT dout;
	dout.sv_position =
	patch[0].sv_position * domain.x 
	+ patch[1].sv_position * domain.y 
	+ patch[2].sv_position * domain.z;
	dout.sv_position.w = 1.0;
	dout.position =
	patch[0].position * domain.x 
	+ patch[1].position * domain.y 
	+ patch[2].position * domain.z;
	dout.normal =
	patch[0].normal * domain.x 
	+ patch[1].normal * domain.y 
	+ patch[2].normal * domain.z;
	return dout;
}
