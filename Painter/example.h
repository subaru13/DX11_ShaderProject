#pragma once
#include <DirectXMath.h>
#include "Painter.h"

class WavePainter :public Painter
{
private:
	PixelShader			pixelShader;
	VertexShader		vertexShader;
	StructuredBuffer	structuredBuffer;
	struct Data
	{
		float red = 1;
		float green = 1;
		float blue = 1;
		float alpha = 1;
		UINT sampleCount = 0;
		UINT samplePerSec = 0;
		float tick = 0;
		float thickness = 0.02f;
	}data;
	ConstantBuffer		constantBuffer;
public:
	WavePainter(ID3D11Device* device, const char* waveFilename);
	void tick(float elapsedTime) { data.tick += elapsedTime; }
	void draw(ID3D11DeviceContext* immediateContext);
	void bake(ID3D11DeviceContext* immediateContext, Layer* layer);
};


class DestructionPainter :public Painter
{
private:
	PixelShader			pixelShader;
	VertexShader		vertexShader;
	DomainShader		domainShader;
	HullShader			hullShader;
	GeometryShader		geometryShader;
	struct Data
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		UINT divNum = 6;
		float speed = 1.0f;
		float achievement = 0.0f;
		int padding = 0;
	}data;
	ConstantBuffer		constantBuffer;
	
public:
	DestructionPainter(ID3D11Device* device);
	void draw(ID3D11DeviceContext* immediateContext);
};