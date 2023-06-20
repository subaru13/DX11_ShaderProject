#include "Destruction.hlsli"

float4x4 axisRotationMatrix(float3 axis, float angle)
{
	float _C = cos(angle);
	float _S = sin(angle);
	int x = 0, y = 1, z = 2;

	float M1[3][3];
	M1[0][x] = _C;
	M1[0][y] = -axis.z * _S;
	M1[0][z] = axis.y * _S;
	M1[1][x] = axis.z * _S;
	M1[1][y] = _C;
	M1[1][z] = -axis.x * _S;
	M1[2][x] = -axis.y * _S;
	M1[2][y] = axis.x * _S;
	M1[2][z] = _C;

	row_major float4x4 M2;
	M2[0][x] = axis.x * axis.x * (1 - _C) + M1[0][x];
	M2[0][y] = axis.y * axis.x * (1 - _C) + M1[0][y];
	M2[0][z] = axis.x * axis.z * (1 - _C) + M1[0][z];
	M2[0][3] = 0.0f;
	M2[1][x] = axis.x * axis.y * (1 - _C) + M1[1][x];
	M2[1][y] = axis.y * axis.y * (1 - _C) + M1[1][y];
	M2[1][z] = axis.y * axis.z * (1 - _C) + M1[1][z];
	M2[1][3] = 0.0f;
	M2[2][x] = axis.x * axis.z * (1 - _C) + M1[2][x];
	M2[2][y] = axis.y * axis.z * (1 - _C) + M1[2][y];
	M2[2][z] = axis.z * axis.z * (1 - _C) + M1[2][z];
	M2[2][3] = 0.0f;
	M2[3][x] = 0.0f;
	M2[3][y] = 0.0f;
	M2[3][z] = 0.0f;
	M2[3][3] = 1.0f;

	return M2;
}


[maxvertexcount(3)]
void main(
	triangle LAYOUT gin[3],
	inout TriangleStream<LAYOUT> gout
)
{
	LAYOUT layout[3];

	float3 centerPos = (float3) 0;
	uint i = 0;
	[unroll]
	for (i = 0; i < 3; i++)
	{
		layout[i].position = mul(float4(gin[i].position, 1.0f), world).xyz;
		layout[i].normal = mul(float4(gin[i].normal, 0.0f), world).xyz;
		centerPos += layout[i].position;
	}

	centerPos /= 3.0f;
	float3 surfaceNormal =
		normalize(float4(cross((layout[1].position - layout[0].position).xyz,
			(layout[2].position - layout[0].position).xyz), 0)).xyz;

	float3 tempPos;

	float4x4 rotationMatrix = axisRotationMatrix(surfaceNormal, rotation);

	[unroll]
	for (i = 0; i < 3; i++)
	{
		tempPos = layout[i].position - centerPos;
		tempPos *= scale;
		tempPos = mul(float4(tempPos, 1.0), rotationMatrix).xyz;
		tempPos += centerPos + (surfaceNormal * move);
		layout[i].sv_position = mul(float4(tempPos, 1.0), viewProjection);
		layout[i].position = tempPos;
		gout.Append(layout[i]);
	}
	gout.RestartStrip();
}