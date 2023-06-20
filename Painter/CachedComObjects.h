#pragma once
#include <d3d11.h>
#include <memory>
#include <assert.h>

class CachedComObjects
{
public:
	using Handle = std::unique_ptr<CachedComObjects>;
private:
	template<class T>
	using Com = T*;

	Com<ID3D11InputLayout>			inputLayout		= nullptr;
	Com<ID3D11VertexShader>			vertexShader	= nullptr;
	Com<ID3D11PixelShader>			pixelShader		= nullptr;
	Com<ID3D11HullShader>			hullShader		= nullptr;
	Com<ID3D11GeometryShader>		geometryShader	= nullptr;
	Com<ID3D11DomainShader>			domainShader	= nullptr;

	Com<ID3D11RenderTargetView>		renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
	Com<ID3D11DepthStencilView>		depthStencilView = nullptr;
	Com<ID3D11ShaderResourceView>	shaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { nullptr };

	Com<ID3D11SamplerState>			samplerStates[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = { nullptr };
	Com<ID3D11BlendState>			blendState = nullptr;
	FLOAT							blendFactor[4] = { 1,1,1,1 };
	UINT							sampleMask = 0xffffffff;

	Com<ID3D11RasterizerState>		rasterizerState = nullptr;
	Com<ID3D11DepthStencilState>	depthStencilState = nullptr;
	UINT							RasterizerState = 1;

	D3D_PRIMITIVE_TOPOLOGY			primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	D3D11_VIEWPORT					viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	UINT							numberOfViewport = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

	CachedComObjects(ID3D11DeviceContext* immediateContext)
	{
		immediateContext->IAGetInputLayout(&inputLayout);
		immediateContext->VSGetShader(&vertexShader, nullptr, nullptr);
		immediateContext->PSGetShader(&pixelShader, nullptr, nullptr);
		immediateContext->HSGetShader(&hullShader, nullptr, nullptr);
		immediateContext->GSGetShader(&geometryShader, nullptr, nullptr);
		immediateContext->DSGetShader(&domainShader, nullptr, nullptr);
		immediateContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, renderTargetViews, &depthStencilView);
		immediateContext->PSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, shaderResourceViews);
		immediateContext->PSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerStates);
		immediateContext->OMGetBlendState(&blendState, blendFactor, &sampleMask);
		immediateContext->RSGetState(&rasterizerState);
		immediateContext->OMGetDepthStencilState(&depthStencilState, &RasterizerState);
		immediateContext->IAGetPrimitiveTopology(&primitiveTopology);
		immediateContext->RSGetViewports(&numberOfViewport, viewports);
	}

	void pop(ID3D11DeviceContext* immediateContext)const
	{
		immediateContext->IASetInputLayout(inputLayout);
		immediateContext->VSSetShader(vertexShader, nullptr, 0);
		immediateContext->PSSetShader(pixelShader, nullptr, 0);
		immediateContext->HSSetShader(hullShader, nullptr, 0);
		immediateContext->GSSetShader(geometryShader, nullptr, 0);
		immediateContext->DSSetShader(domainShader, nullptr, 0);
		immediateContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, renderTargetViews, depthStencilView);
		immediateContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, shaderResourceViews);
		immediateContext->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, samplerStates);
		immediateContext->OMSetBlendState(blendState, blendFactor, sampleMask);
		immediateContext->RSSetState(rasterizerState);
		immediateContext->OMSetDepthStencilState(depthStencilState, RasterizerState);
		immediateContext->IASetPrimitiveTopology(primitiveTopology);
		immediateContext->RSSetViewports(numberOfViewport, viewports);
	}


	template<class T>
	static void safeRelease(Com<T>& com)
	{
		if (com)com->Release();
	}

public:
	~CachedComObjects()
	{
		safeRelease(inputLayout);
		safeRelease(vertexShader);
		safeRelease(pixelShader);
		safeRelease(hullShader);
		safeRelease(geometryShader);
		safeRelease(domainShader);
		for (decltype(auto) render_target_view : renderTargetViews)
		{
			safeRelease(render_target_view);
		}
		safeRelease(depthStencilView);
		for (decltype(auto) shader_resource_view : shaderResourceViews)
		{
			safeRelease(shader_resource_view);
		}
		for (decltype(auto) SamplerState : samplerStates)
		{
			safeRelease(SamplerState);
		}
		safeRelease(blendState);
		safeRelease(rasterizerState);
		safeRelease(depthStencilState);
	}

	friend inline Handle pushCachedComObjects(ID3D11DeviceContext* immediateContext)
	{
		assert(immediateContext && "The context is invalid.");
		return Handle(new CachedComObjects(immediateContext));
	}

	friend inline void popCachedComObjects(ID3D11DeviceContext* immediateContext, Handle& handle)
	{
		assert(immediateContext && "The context is invalid.");
		CachedComObjects* data = handle.release();
		if (data)
		{
			data->pop(immediateContext);
			delete data;
		}
	}
};

using CachedHandle = CachedComObjects::Handle;
CachedHandle pushCachedComObjects(ID3D11DeviceContext*);
void popCachedComObjects(ID3D11DeviceContext*, CachedHandle&);

