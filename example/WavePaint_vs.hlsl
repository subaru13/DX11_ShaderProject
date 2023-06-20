
void main(in uint vertex_id : SV_VERTEXID, out float4 sv_position : SV_POSITION, out float2 texcoord : TEXCOORD)
{
	texcoord = float2((vertex_id << 1) & 2, vertex_id & 2);
	sv_position = float4(texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
}