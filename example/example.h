#pragma once
#include "../Painter/Painter.h"

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
	ConstantBuffer		constantBuffer;

public:
	struct Data
	{
		Float4x4 world = {};
		Float4x4 viewProjection = {};
		Float4 color = { 1,1,1,1 };
		float divNum = 6;
		float move = 0;
		float scale = 1;
		float rotation = 0;
	}data;
	DestructionPainter(ID3D11Device* device);
	void draw(ID3D11DeviceContext* immediateContext, Geometry* geometry);
};

class ToonPainter :public Painter
{
private:
	PixelShader			pixelShader;
	VertexShader		vertexShader;
	ConstantBuffer		constantBuffer;
public:
	struct Data
	{
		Float4x4 world = {};
		Float4x4 viewProjection = {};
		Float4 matColor = { 1,1,1,1 };
		Float4 rimColor = { 0,0,0,1 };
		Float3 lightDir = { 0,-1,0 };
		int toonLevels = { 5 };
		Float3 eyePos = { 0,0,0 };
		float outlineThreshold = { 0 };
	}data;
	ToonPainter(ID3D11Device* device);
	void draw(ID3D11DeviceContext* immediateContext, Geometry* geometry);
};
