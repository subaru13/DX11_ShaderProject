#pragma once
#include "Painter.h"

class SpritePainter : public Painter
{
public:
	struct Vertex
	{
		Float2 mPos{};
		Float2 mUV{};
		Float4 mColor{ 1.0f,1.0f,1.0f,1.0f };
	};
private:
	PixelShader pixelShader;
	VertexShader vertexShader;
public:
	SpritePainter(ID3D11Device* device);
	virtual ~SpritePainter() = default;

	virtual void drawBegin(ID3D11DeviceContext* immediateContext)override;
	virtual void drawEnd(ID3D11DeviceContext* immediateContext)override;

	void draw(
		ID3D11DeviceContext* immediateContext,
		ShaderResource* shaderResource,
		VertexBuffer* vertexBuffer,
		PixelShader* customPixelShader);

	void drawIndexed(
		ID3D11DeviceContext* immediateContext,
		ShaderResource* shaderResource,
		VertexBuffer* vertexBuffer,
		IndexBuffer* indexBuffer,
		PixelShader* customPixelShader);
};
