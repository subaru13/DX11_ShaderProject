#include "SpritePainter.h"

SpritePainter::SpritePainter(ID3D11Device* device)
	:Painter(device)
{
	loadPixelShader(device, &pixelShader, "asset\\Sprite_ps.cso");
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	loadVertexShader(device, &vertexShader, "asset\\Sprite_vs.cso", inputElementDesc, 3);
}

void SpritePainter::drawBegin(ID3D11DeviceContext* immediateContext)
{
	pushStates(immediateContext);
	setDepthStencilState(immediateContext, DepthStencilState::none);
	setRasterizerState(immediateContext, RasterizerState::solid);
	setBlendState(immediateContext, BlendState::alpha);
	setSamplerStates(immediateContext, SamplerState::linear, 0, false, true, false, false, false);
}

void SpritePainter::drawEnd(ID3D11DeviceContext* immediateContext)
{
	popStates(immediateContext);
}

void SpritePainter::draw(
	ID3D11DeviceContext* immediateContext,
	ShaderResource* shaderResource,
	VertexBuffer* vertexBuffer,
	PixelShader* customPixelShader)
{
	immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	if (customPixelShader)
	{
		pixelShader.set(immediateContext);
	}
	else
	{
		pixelShader.set(immediateContext);
	}
	shaderResource->set(immediateContext, 0, false, true, false, false, false);
	vertexShader.set(immediateContext);
	vertexBuffer->set(immediateContext, 0);
	immediateContext->Draw(vertexBuffer->count, 0);
}

void SpritePainter::drawIndexed(
	ID3D11DeviceContext* immediateContext,
	ShaderResource* shaderResource,
	VertexBuffer* vertexBuffer,
	IndexBuffer* indexBuffer,
	PixelShader* customPixelShader)
{
	immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	setDepthStencilState(immediateContext, DepthStencilState::none);
	setRasterizerState(immediateContext, RasterizerState::solid);
	if (customPixelShader)
	{
		pixelShader.set(immediateContext);
	}
	else
	{
		pixelShader.set(immediateContext);
	}
	shaderResource->set(immediateContext, 0, false, true, false, false, false);
	vertexShader.set(immediateContext);
	vertexBuffer->set(immediateContext, 0);
	indexBuffer->set(immediateContext);
	immediateContext->DrawIndexed(indexBuffer->count, 0, 0);
}
