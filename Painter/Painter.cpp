#include "Painter.h"
#include <fstream>
#include <map>
#include <string>
#include <wrl.h>
#include <assert.h>
#include <vector>
#include <WICTextureLoader.h>

#define hrInspection(hr) assert(hr == S_OK)

namespace detail
{
	using namespace std;
	struct CsoData
	{
		char* code = 0;
		SIZE_T length = 0;

		void malloc(SIZE_T size)
		{
			free();
			length = size;
			code = new char[size];
		}

		void free()
		{
			if (code)
			{
				length = 0;
				delete[] code;
				code = 0;
			}
		}

		~CsoData() { free(); }
	};

	inline void loadCsoFile(const char* path, CsoData& data)
	{
		ifstream ifs{ path,ios::binary };
		if (ifs)
		{
			ifs.seekg(0, ios::end);
			streampos fileSize = ifs.tellg();
			ifs.seekg(0, ios::beg);
			data.malloc(fileSize);
			ifs.read(data.code, fileSize);
		}
	}

	using PsCache = map<string, PixelShader>;
	using VsCache = map<string, VertexShader>;
	using DsCache = map<string, DomainShader>;
	using HsCache = map<string, HullShader>;
	using GsCache = map<string, GeometryShader>;

	HRESULT createTexture2D(ID3D11Device* device,
		ID3D11Texture2D** texture2D,
		UINT width, UINT height,
		DXGI_FORMAT format,
		UINT bindFlag)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		if (bindFlag & D3D11_BIND_DEPTH_STENCIL)
		{
			desc.CPUAccessFlags = 0;
		}
		else
		{
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		desc.BindFlags = bindFlag;
		return device->CreateTexture2D(&desc, nullptr, texture2D);
	}

	HRESULT createResource(ID3D11Device* device,
		ID3D11Texture2D* texture2D,
		ID3D11ShaderResourceView** resourceView)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		ZeroMemory(&srvd, sizeof(srvd));
		if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		}
		else
		{
			srvd.Format = desc.Format;
		}
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.MipLevels = desc.MipLevels;
		return device->CreateShaderResourceView(texture2D, &srvd, resourceView);
	}

	HRESULT createRenderTarget(ID3D11Device* device,
		ID3D11Texture2D* texture2D,
		ID3D11RenderTargetView** targetView)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		ZeroMemory(&rtvd, sizeof(rtvd));
		if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			rtvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		}
		else
		{
			rtvd.Format = desc.Format;
		}
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;
		return device->CreateRenderTargetView(texture2D, &rtvd, targetView);
	}

	HRESULT createDepthStencil(ID3D11Device* device,
		ID3D11Texture2D* texture2D,
		ID3D11DepthStencilView** depthStencil)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;
		return device->CreateDepthStencilView(texture2D, &dsvd, depthStencil);
	}
}

void PixelShader::set(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->PSSetShader(shader.Get(), nullptr, 0);
}



PipelineState::PipelineState(ID3D11Device* device)
{
	HRESULT hr;

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SamplerState::point].ReleaseAndGetAddressOf());
	hrInspection(hr);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SamplerState::linear].GetAddressOf());
	hrInspection(hr);

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SamplerState::anisotropic].GetAddressOf());
	hrInspection(hr);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[DepthStencilState::common].GetAddressOf());
	hrInspection(hr);

	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[DepthStencilState::none].GetAddressOf());
	hrInspection(hr);

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blendDesc, blendStates[BlendState::none].GetAddressOf());
	hrInspection(hr);

	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blendDesc, blendStates[BlendState::alpha].GetAddressOf());
	hrInspection(hr);

	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_ONE D3D11_BLEND_SRC_ALPHA
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blendDesc, blendStates[BlendState::add].GetAddressOf());
	hrInspection(hr);

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[RasterizerState::solid].GetAddressOf());
	hrInspection(hr);

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerStates[RasterizerState::wireframe].GetAddressOf());
	hrInspection(hr);
}

void PipelineState::setDepthStencilState(
	ID3D11DeviceContext* immediateContext,
	DepthStencilState depthStencilState,
	UINT stencil_ref)
{
	immediateContext->OMSetDepthStencilState(depthStencilStates[depthStencilState].Get(), stencil_ref);
}

void PipelineState::setSamplerStates(
	ID3D11DeviceContext* immediateContext,
	SamplerState samplerState,
	UINT slot,
	bool useVs,
	bool usePs,
	bool useDs,
	bool useHs,
	bool useGs)
{
	assert(immediateContext && "The context is invalid.");
	if (useVs)immediateContext->VSSetSamplers(slot, 1, samplerStates[samplerState].GetAddressOf());
	if (usePs)immediateContext->PSSetSamplers(slot, 1, samplerStates[samplerState].GetAddressOf());
	if (useDs)immediateContext->DSSetSamplers(slot, 1, samplerStates[samplerState].GetAddressOf());
	if (useHs)immediateContext->HSSetSamplers(slot, 1, samplerStates[samplerState].GetAddressOf());
	if (useGs)immediateContext->GSSetSamplers(slot, 1, samplerStates[samplerState].GetAddressOf());
}

void PipelineState::setBlendState(
	ID3D11DeviceContext* immediateContext,
	BlendState blendState)
{
	immediateContext->OMSetBlendState(blendStates[blendState].Get(), NULL, 0xFFFFFFFF);
}

void PipelineState::setRasterizerState(
	ID3D11DeviceContext* immediateContext,
	RasterizerState rasterizerState)
{
	immediateContext->RSSetState(rasterizerStates[rasterizerState].Get());
}

void VertexShader::set(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->VSSetShader(shader.Get(), nullptr, 0);
	immediateContext->IASetInputLayout(layout.Get());
}

void DomainShader::set(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->DSSetShader(shader.Get(), nullptr, 0);
}

void HullShader::set(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->HSSetShader(shader.Get(), nullptr, 0);
}

void GeometryShader::set(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->GSSetShader(shader.Get(), nullptr, 0);
}

void ShaderResource::set(ID3D11DeviceContext* immediateContext, UINT slot, bool useVs, bool usePs, bool useDs, bool useHs, bool useGs)
{
	assert(immediateContext && "The context is invalid.");
	if (useVs)immediateContext->VSSetShaderResources(slot, 1, resource.GetAddressOf());
	if (usePs)immediateContext->PSSetShaderResources(slot, 1, resource.GetAddressOf());
	if (useDs)immediateContext->DSSetShaderResources(slot, 1, resource.GetAddressOf());
	if (useHs)immediateContext->HSSetShaderResources(slot, 1, resource.GetAddressOf());
	if (useGs)immediateContext->GSSetShaderResources(slot, 1, resource.GetAddressOf());
}

void StructuredBuffer::updateSubresource(ID3D11DeviceContext* immediateContext, const void* data)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->UpdateSubresource(buffer.Get(), 0, 0, data, 0, 0);
}

void ConstantBuffer::updateSubresource(ID3D11DeviceContext* immediateContext, const void* data)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->UpdateSubresource(buffer.Get(), 0, 0, data, 0, 0);
}

void ConstantBuffer::set(ID3D11DeviceContext* immediateContext, UINT slot, bool useVs, bool usePs, bool useDs, bool useHs, bool useGs)
{
	assert(immediateContext && "The context is invalid.");
	if (useVs)immediateContext->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	if (usePs)immediateContext->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	if (useDs)immediateContext->DSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	if (useHs)immediateContext->HSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	if (useGs)immediateContext->GSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
}

void RenderTexture::clear(ID3D11DeviceContext* immediateContext, float r, float g, float b, float a)
{
	assert(immediateContext && "The context is invalid.");
	const FLOAT color[]{ r,g,b,a };
	immediateContext->ClearRenderTargetView(view.Get(), color);
}

void DepthTexture::clear(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->ClearDepthStencilView(view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Layer::clear(ID3D11DeviceContext* immediateContext, float r, float g, float b, float a)
{
	assert(immediateContext && "The context is invalid.");
	colorMap.clear(immediateContext, r, g, b, a);
	depthMap.clear(immediateContext);
}

void Layer::switching(ID3D11DeviceContext* immediateContext)
{
	assert(immediateContext && "The context is invalid.");
	immediateContext->OMSetRenderTargets(1, colorMap.view.GetAddressOf(), depthMap.view.Get());
	immediateContext->RSSetViewports(1, &viewport);
}

void VertexBuffer::set(ID3D11DeviceContext* immediateContext, UINT slot, UINT offset)
{
	immediateContext->IASetVertexBuffers(slot, 1, buffer.GetAddressOf(), &stride, &offset);
}

void IndexBuffer::set(ID3D11DeviceContext* immediateContext)
{
	immediateContext->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void Mesh::set(ID3D11DeviceContext* immediateContext, UINT slot, UINT offset)
{
	if (vertexBuffer.buffer)
	{
		vertexBuffer.set(immediateContext, slot, offset);
	}
	if (indexBuffer.buffer)
	{
		indexBuffer.set(immediateContext);
	}
}

void Painter::drawBegin(ID3D11DeviceContext* immediateContext)
{
	pushState(immediateContext);
}

void Painter::drawEnd(ID3D11DeviceContext* immediateContext)
{
	popState(immediateContext);
}

void Painter::pushState(ID3D11DeviceContext* immediateContext)
{
	cachedHandle.push(pushCachedComObjects(immediateContext));
}

void Painter::popState(ID3D11DeviceContext* immediateContext)
{
	if (cachedHandle.empty()) { return; }
	popCachedComObjects(immediateContext, cachedHandle.top());
	cachedHandle.pop();
}

void Geometry::set(ID3D11DeviceContext* immediateContext)
{
	vertexBuffer.set(immediateContext, 0, 0);
	indexBuffer.set(immediateContext);
}

void makeCube(ID3D11Device* device, Geometry* cube)
{
	using Vertex = Geometry::Vertex;
	Vertex vertices[24] = {};
	UINT indices[36] = { 0 };

	int face;
	face = 0;
	vertices[face * 4 + 0].position = Float3(-0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 1].position = Float3(+0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 2].position = Float3(-0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 3].position = Float3(+0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 0].normal = Float3(+0.0f, +1.0f, +0.0f);
	vertices[face * 4 + 1].normal = Float3(+0.0f, +1.0f, +0.0f);
	vertices[face * 4 + 2].normal = Float3(+0.0f, +1.0f, +0.0f);
	vertices[face * 4 + 3].normal = Float3(+0.0f, +1.0f, +0.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;
	face += 1;
	vertices[face * 4 + 0].position = Float3(-0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 1].position = Float3(+0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 2].position = Float3(-0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 3].position = Float3(+0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 0].normal = Float3(+0.0f, -1.0f, +0.0f);
	vertices[face * 4 + 1].normal = Float3(+0.0f, -1.0f, +0.0f);
	vertices[face * 4 + 2].normal = Float3(+0.0f, -1.0f, +0.0f);
	vertices[face * 4 + 3].normal = Float3(+0.0f, -1.0f, +0.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;
	face += 1;
	vertices[face * 4 + 0].position = Float3(-0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 1].position = Float3(+0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 2].position = Float3(-0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 3].position = Float3(+0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 0].normal = Float3(+0.0f, +0.0f, -1.0f);
	vertices[face * 4 + 1].normal = Float3(+0.0f, +0.0f, -1.0f);
	vertices[face * 4 + 2].normal = Float3(+0.0f, +0.0f, -1.0f);
	vertices[face * 4 + 3].normal = Float3(+0.0f, +0.0f, -1.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;
	face += 1;
	vertices[face * 4 + 0].position = Float3(-0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 1].position = Float3(+0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 2].position = Float3(-0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 3].position = Float3(+0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 0].normal = Float3(+0.0f, +0.0f, +1.0f);
	vertices[face * 4 + 1].normal = Float3(+0.0f, +0.0f, +1.0f);
	vertices[face * 4 + 2].normal = Float3(+0.0f, +0.0f, +1.0f);
	vertices[face * 4 + 3].normal = Float3(+0.0f, +0.0f, +1.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;
	face += 1;
	vertices[face * 4 + 0].position = Float3(+0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 1].position = Float3(+0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 2].position = Float3(+0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 3].position = Float3(+0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 0].normal = Float3(+1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 1].normal = Float3(+1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 2].normal = Float3(+1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 3].normal = Float3(+1.0f, +0.0f, +0.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;
	face += 1;
	vertices[face * 4 + 0].position = Float3(-0.5f, +0.5f, -0.5f);
	vertices[face * 4 + 1].position = Float3(-0.5f, +0.5f, +0.5f);
	vertices[face * 4 + 2].position = Float3(-0.5f, -0.5f, -0.5f);
	vertices[face * 4 + 3].position = Float3(-0.5f, -0.5f, +0.5f);
	vertices[face * 4 + 0].normal = Float3(-1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 1].normal = Float3(-1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 2].normal = Float3(-1.0f, +0.0f, +0.0f);
	vertices[face * 4 + 3].normal = Float3(-1.0f, +0.0f, +0.0f);
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	createVertexBuffer(device, &cube->vertexBuffer, sizeof(Vertex), 24, vertices);
	createIndexBuffer(device, &cube->indexBuffer, 24, indices);
}

void makeSphere(ID3D11Device* device, Geometry* sphere, UINT slices, UINT stacks)
{
	using Vertex = Geometry::Vertex;
	UINT verticesSize = (slices + 1) * (stacks + 1);
	UINT indicesSize = stacks * slices * 2 * 3;
	std::vector<Vertex> vertices{};
	std::vector<UINT> indices{};
	vertices.resize(verticesSize);
	indices.resize(indicesSize);

	//頂点作成
	for (UINT y = 0; y < stacks + 1; y++)
	{
		float h = 0.5f * cosf(y * DirectX::XM_PI / stacks);
		float w = 0.5f * sinf(y * DirectX::XM_PI / stacks);
		for (UINT x = 0; x < slices + 1; x++)
		{
			UINT index = y * (slices + 1) + x;
			float rad_slices = x * DirectX::XM_2PI / slices;

			vertices[static_cast<size_t>(index)].position.x = w * sinf(rad_slices);
			vertices[static_cast<size_t>(index)].position.y = h;
			vertices[static_cast<size_t>(index)].position.z = w * cosf(rad_slices);

			vertices[static_cast<size_t>(index)].normal.x = vertices[static_cast<size_t>(index)].position.x * 2.0f;
			vertices[static_cast<size_t>(index)].normal.y = vertices[static_cast<size_t>(index)].position.y * 2.0f;
			vertices[static_cast<size_t>(index)].normal.z = vertices[static_cast<size_t>(index)].position.z * 2.0f;
		}
	}
	//インデックス作成
	for (UINT y = 0; y < stacks; y++)
	{
		for (UINT x = 0; x < slices; x++)
		{
			UINT face = (y * slices + x);
			UINT vertices_index = y * (slices + 1) + x;
			indices[static_cast<size_t>(face) * 6] = vertices_index + 1;
			indices[static_cast<size_t>(face) * 6 + 1] = vertices_index;
			indices[static_cast<size_t>(face) * 6 + 2] = vertices_index + (slices + 1);

			indices[static_cast<size_t>(face) * 6 + 3] = vertices_index + 1;
			indices[static_cast<size_t>(face) * 6 + 4] = vertices_index + (slices + 1);
			indices[static_cast<size_t>(face) * 6 + 5] = vertices_index + (slices + 1) + 1;
		}
	}

	createVertexBuffer(device, &sphere->vertexBuffer, sizeof(Vertex), verticesSize, vertices.data());
	createIndexBuffer(device, &sphere->indexBuffer, indicesSize, indices.data());
}

HRESULT loadPixelShader(ID3D11Device* device, PixelShader* outPs, const char* path)
{
	assert(device && "The device is invalid.");
	static detail::PsCache psCache;
	HRESULT hr = S_FALSE;
	if (psCache.count(path))
	{
		(*outPs) = psCache[path];
		hr = S_OK;
	}
	else
	{
		detail::CsoData csoData;
		detail::loadCsoFile(path, csoData);
		hr = device->CreatePixelShader(csoData.code, csoData.length, nullptr, outPs->shader.ReleaseAndGetAddressOf());
		hrInspection(hr);
		psCache[path] = (*outPs);
	}
	return hr;
}

HRESULT loadVertexShader(ID3D11Device* device,
	VertexShader* outVs,
	const char* path,
	D3D11_INPUT_ELEMENT_DESC* descs,
	UINT descsArrSize)
{
	assert(device && "The device is invalid.");
	static detail::VsCache vsCache;
	HRESULT hr = S_FALSE;
	if (vsCache.count(path))
	{
		(*outVs) = vsCache[path];
		hr = S_OK;
	}
	else
	{
		detail::CsoData csoData;
		detail::loadCsoFile(path, csoData);
		hr = device->CreateVertexShader(csoData.code, csoData.length, nullptr, outVs->shader.ReleaseAndGetAddressOf());
		hrInspection(hr);
		if (descs)
		{
			hr = device->CreateInputLayout(descs, descsArrSize, csoData.code, csoData.length, outVs->layout.ReleaseAndGetAddressOf());
			hrInspection(hr);
		}
		vsCache[path] = (*outVs);
	}
	return hr;
}

HRESULT loadDomainShader(ID3D11Device* device, DomainShader* outDs, const char* path)
{
	assert(device && "The device is invalid.");
	static detail::DsCache dsCache;
	HRESULT hr = S_FALSE;
	if (dsCache.count(path))
	{
		(*outDs) = dsCache[path];
		hr = S_OK;
	}
	else
	{
		detail::CsoData csoData;
		detail::loadCsoFile(path, csoData);
		hr = device->CreateDomainShader(csoData.code, csoData.length, nullptr, outDs->shader.ReleaseAndGetAddressOf());
		hrInspection(hr);
		dsCache[path] = (*outDs);
	}
	return hr;
}

HRESULT loadHullShader(ID3D11Device* device, HullShader* outHs, const char* path)
{
	assert(device && "The device is invalid.");
	static detail::HsCache hsCache;
	HRESULT hr = S_FALSE;
	if (hsCache.count(path))
	{
		(*outHs) = hsCache[path];
		hr = S_OK;
	}
	else
	{
		detail::CsoData csoData;
		detail::loadCsoFile(path, csoData);
		hr = device->CreateHullShader(csoData.code, csoData.length, nullptr, outHs->shader.ReleaseAndGetAddressOf());
		hrInspection(hr);
		hsCache[path] = (*outHs);
	}
	return hr;
}

HRESULT loadGeometryShader(ID3D11Device* device, GeometryShader* outGs, const char* path)
{
	assert(device && "The device is invalid.");
	static detail::GsCache gsCache;
	HRESULT hr = S_FALSE;
	if (gsCache.count(path))
	{
		(*outGs) = gsCache[path];
		hr = S_OK;
	}
	else
	{
		detail::CsoData csoData;
		detail::loadCsoFile(path, csoData);
		hr = device->CreateGeometryShader(csoData.code, csoData.length, nullptr, outGs->shader.ReleaseAndGetAddressOf());
		hrInspection(hr);
		gsCache[path] = (*outGs);
	}
	return hr;
}

HRESULT loadShaderResource(ID3D11Device* device, ShaderResource* outSr, const wchar_t* path)
{
	assert(device && "The device is invalid.");
	return DirectX::CreateWICTextureFromFile(device,
		path,
		nullptr,
		outSr->resource.ReleaseAndGetAddressOf());
}

HRESULT createStructuredBuffer(ID3D11Device* device,
	StructuredBuffer* outSb,
	UINT elementSize,
	UINT count,
	void* initData)
{
	assert(device && "The device is invalid.");
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = elementSize * count;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;
	HRESULT hr = S_OK;
	if (initData)
	{
		D3D11_SUBRESOURCE_DATA _init_data{};
		_init_data.pSysMem = initData;
		hr = device->CreateBuffer(&desc, &_init_data, outSb->buffer.ReleaseAndGetAddressOf());
	}
	else
	{
		hr = device->CreateBuffer(&desc, nullptr, outSb->buffer.ReleaseAndGetAddressOf());
	}
	hrInspection(hr);
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srv_desc.BufferEx.FirstElement = 0;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
	hr = device->CreateShaderResourceView(outSb->buffer.Get(), &srv_desc, outSb->resource.ReleaseAndGetAddressOf());
	hrInspection(hr);
	return hr;
}

HRESULT createConstantBuffer(ID3D11Device* device, ConstantBuffer* outCb, UINT elementSize, void* initData)
{
	assert(device && "The device is invalid.");
	assert(elementSize % 16 == 0 && "constant buffer's need to be 16 byte aligned");
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = elementSize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	HRESULT hr = S_OK;
	if (initData)
	{
		D3D11_SUBRESOURCE_DATA subresourceData{};
		subresourceData.pSysMem = initData;
		hr = device->CreateBuffer(&desc, &subresourceData, outCb->buffer.ReleaseAndGetAddressOf());
	}
	else
	{
		hr = device->CreateBuffer(&desc, nullptr, outCb->buffer.ReleaseAndGetAddressOf());
	}
	hrInspection(hr);
	return hr;
}

HRESULT createRenderTextrue(ID3D11Device* device,
	RenderTexture* outRt,
	UINT width, UINT height,
	DXGI_FORMAT format)
{
	assert(device && "The device is invalid.");
	HRESULT hr;
	ID3D11Texture2D* texture2D;
	hr = detail::createTexture2D(device, &texture2D, width, height, format, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	hrInspection(hr);

	hr = detail::createResource(device, texture2D, outRt->resource.ReleaseAndGetAddressOf());
	hrInspection(hr);

	hr = detail::createRenderTarget(device, texture2D, outRt->view.ReleaseAndGetAddressOf());
	hrInspection(hr);

	texture2D->Release();

	return hr;
}

HRESULT createDepthTextrue(ID3D11Device* device,
	DepthTexture* outDt,
	UINT width, UINT height)
{
	assert(device && "The device is invalid.");
	HRESULT hr;
	ID3D11Texture2D* texture2D;
	hr = detail::createTexture2D(device, &texture2D, width, height, DXGI_FORMAT_R24G8_TYPELESS, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	hrInspection(hr);

	hr = detail::createResource(device, texture2D, outDt->resource.ReleaseAndGetAddressOf());
	hrInspection(hr);

	hr = detail::createDepthStencil(device, texture2D, outDt->view.ReleaseAndGetAddressOf());
	hrInspection(hr);

	texture2D->Release();

	return hr;
}

HRESULT createLayer(ID3D11Device* device, Layer* outLayer, UINT width, UINT height, DXGI_FORMAT format)
{
	assert(device && "The device is invalid.");
	HRESULT hr;

	hr = createRenderTextrue(device, &outLayer->colorMap, width, height, format);
	hrInspection(hr);

	hr = createDepthTextrue(device, &outLayer->depthMap, width, height);
	hrInspection(hr);

	outLayer->viewport.TopLeftX = 0.0f;
	outLayer->viewport.TopLeftY = 0.0f;
	outLayer->viewport.Width = (FLOAT)width;
	outLayer->viewport.Height = (FLOAT)height;
	outLayer->viewport.MinDepth = 0.0f;
	outLayer->viewport.MaxDepth = 1.0f;

	return hr;
}

HRESULT createVertexBuffer(ID3D11Device* device, VertexBuffer* outVertexBuffer, UINT stride, UINT count, const void* initialValue)
{
	outVertexBuffer->stride = stride;
	outVertexBuffer->count = count;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = stride * count;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = initialValue;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, outVertexBuffer->buffer.ReleaseAndGetAddressOf());
	hrInspection(hr);
	return hr;
}

HRESULT createIndexBuffer(ID3D11Device* device, IndexBuffer* outIndexBuffer, UINT count, const UINT* initialValue)
{
	outIndexBuffer->count = count;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(UINT) * count;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = initialValue;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, outIndexBuffer->buffer.ReleaseAndGetAddressOf());
	hrInspection(hr);
	return hr;
}
