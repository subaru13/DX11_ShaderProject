//波形データ
StructuredBuffer<float> amplitudes : register(t0);

cbuffer WaveData : register(b0)
{
	float4 lineColor;//線の色
	uint frameCount;//波形データのサンプル総数
	uint samplingRate;//サンプリングレート
	float tick;//経過時間
	float thickness;//線の厚さ
};

float isRange(float x, float a, float b)
{
	return step(min(a, b), x) * step(x, max(a, b));
}

float4 main(in float4 sv_position : SV_POSITION, in float2 texcoord : TEXCOORD) : SV_TARGET
{
	const float fineness = 50;

	//サンプル開始位置を算出
	float samplingPositionBegin = samplingRate * tick;

	//サンプル位置を算出
	float samplingPosition = samplingPositionBegin + samplingRate * trunc(texcoord.x * fineness);

	//波形データをサンプリングして-0.5 ~ 0.5へ変換する(1つ目)
	float amplitudeBegin = amplitudes[(uint) samplingPosition % frameCount] * 0.5 + 0.5;

	//1サンプル分移動
	samplingPosition += samplingRate;

	//波形データをサンプリングして-0.5 ~ 0.5へ変換する(2つ目)
	float amplitudeEnd = amplitudes[(uint) samplingPosition % frameCount] * 0.5 + 0.5;

	float4 outColor = (float4) 0;

	//現在見ているピクセルが2つのサンプル間のどこに位置するのか算出
	float amplitude = lerp(amplitudeBegin, amplitudeEnd, frac(texcoord.x * fineness));

	//線の幅を算出
	float width = thickness * 0.5;

	//現在見ているピクセルが線上にあるか判定
	outColor = lerp(outColor, lineColor, isRange(texcoord.y, amplitude - width, amplitude + width));

	return outColor;
}
