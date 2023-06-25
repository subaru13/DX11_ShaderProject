struct VertexInput
{
	float2 pos : POSITION;
	float2 texcoord : TEXCOORD;
	float4 color : COLOR;
};
struct VertexOutput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 color : COLOR;
};

VertexOutput main(VertexInput vin)
{
	VertexOutput vout;
	vout.pos = float4(vin.pos, 0, 1);
	vout.texcoord = vin.texcoord;
	vout.color = vin.color;
	return vout;
}
