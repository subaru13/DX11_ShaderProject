#include "Toon.hlsli"

float4 toonShader(
float3 lightDirection,
float3 viewDirection,
float3 normal,
float4 baseColor,
float4 rimColor,
int toneLevels,
float outlineThreshold)
{
	// ライトの方向と法線ベクトルの内積を計算
	float NdotL = saturate(dot(-lightDirection, normal));

	// 階調の制御
	float threshold = 1.0 / (float) toneLevels;
	float intensity = floor(NdotL / threshold) * threshold + threshold;

	float rim = saturate(dot(-viewDirection, normal));
	
	// 最終的な色の計算
	return lerp(rimColor, baseColor * intensity, step(outlineThreshold, rim));
}


float4 main(PixelInput pin) : SV_TARGET
{
	return toonShader(
	normalize(lightDir),
	normalize(pin.position - eyePos),
	normalize(pin.normal),
	matColor,
	rimColor,
	toonLevels,
	outlineThreshold);
}
