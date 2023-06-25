#pragma once
#include "../painter/Painter.h"
#include <cereal/cereal.hpp>

class WavePainter :public Painter
{
private:
	PixelShader			pixelShader;
	VertexShader		vertexShader;
	StructuredBuffer	structuredBuffer;
	ConstantBuffer		constantBuffer;
public:
	struct Data
	{
		Float4 color{1,1,1,1};
		UINT sampleCount = 0;
		UINT samplePerSec = 0;
		float tick = 0;
		float thickness = 0.02f;

		template<class T>
		void serialize(T& archive)
		{
			archive(
				CEREAL_NVP(color),
				CEREAL_NVP(sampleCount),
				CEREAL_NVP(samplePerSec),
				CEREAL_NVP(tick),
				CEREAL_NVP(thickness)
			);
		}
	}data;
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

		template<class T>
		void serialize(T& archive)
		{
			archive(
				CEREAL_NVP(world),
				CEREAL_NVP(viewProjection),
				CEREAL_NVP(color),
				CEREAL_NVP(divNum),
				CEREAL_NVP(move),
				CEREAL_NVP(scale),
				CEREAL_NVP(rotation)
			);
		}
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

		template<class T>
		void serialize(T& archive)
		{
			archive(
				CEREAL_NVP(world),
				CEREAL_NVP(viewProjection),
				CEREAL_NVP(matColor),
				CEREAL_NVP(rimColor),
				CEREAL_NVP(lightDir),
				CEREAL_NVP(toonLevels),
				CEREAL_NVP(eyePos),
				CEREAL_NVP(outlineThreshold)
			);
		}
	}data;
	ToonPainter(ID3D11Device* device);
	void draw(ID3D11DeviceContext* immediateContext, Geometry* geometry);
};
