#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <assert.h>
#include <map>
#include <stack>
#include "../func/Arithmetic.h"
#include "CachedComObjects.h"

using Microsoft::WRL::ComPtr;

struct BasicShader
{
	virtual ~BasicShader() = default;
	virtual void set(ID3D11DeviceContext*) = 0;
};

struct PixelShader :public BasicShader
{
	ComPtr<ID3D11PixelShader> shader;

	void set(ID3D11DeviceContext* immediateContext)override;
};

struct VertexShader :public BasicShader
{
	ComPtr<ID3D11VertexShader> shader;
	ComPtr<ID3D11InputLayout> layout;
	void set(ID3D11DeviceContext* immediateContext)override;
};

struct DomainShader :public BasicShader
{
	ComPtr<ID3D11DomainShader> shader;
	void set(ID3D11DeviceContext* immediateContext)override;
};
struct HullShader :public BasicShader

{
	ComPtr<ID3D11HullShader> shader;
	void set(ID3D11DeviceContext* immediateContext)override;
};

struct GeometryShader :public BasicShader
{
	ComPtr<ID3D11GeometryShader> shader;
	void set(ID3D11DeviceContext* immediateContext)override;
};

struct ShaderResource
{
	ComPtr<ID3D11ShaderResourceView> resource;
	void set(ID3D11DeviceContext* immediateContext,
		UINT slot,
		bool useVs = true,
		bool usePs = true,
		bool useDs = true,
		bool useHs = true,
		bool useGs = true);
};

struct StructuredBuffer :public ShaderResource
{
	ComPtr<ID3D11Buffer> buffer;
	void updateSubresource(ID3D11DeviceContext* immediateContext, const void* data);
};

struct ConstantBuffer
{
	ComPtr<ID3D11Buffer> buffer;
	void updateSubresource(ID3D11DeviceContext* immediateContext, const void* data);

	void set(ID3D11DeviceContext* immediateContext,
		UINT slot,
		bool useVs = true,
		bool usePs = true,
		bool useDs = true,
		bool useHs = true,
		bool useGs = true);
};


struct RenderTexture :public ShaderResource
{
	ComPtr<ID3D11RenderTargetView> view;
	void clear(ID3D11DeviceContext* immediateContext, float r = 0, float g = 0, float b = 0, float a = 0);
};

struct DepthTexture :public ShaderResource
{
	ComPtr<ID3D11DepthStencilView> view;
	void clear(ID3D11DeviceContext* immediateContext);
};

struct Layer
{
	RenderTexture colorMap;
	DepthTexture depthMap;
	D3D11_VIEWPORT viewport;
	void clear(ID3D11DeviceContext* immediateContext, float r = 0, float g = 0, float b = 0, float a = 0);
	void switching(ID3D11DeviceContext* immediateContext);
};

struct VertexBuffer
{
	ComPtr<ID3D11Buffer> buffer;
	UINT stride = 0;
	UINT count = 0;
	void set(ID3D11DeviceContext* immediateContext, UINT slot, UINT offset = 0);
};

struct IndexBuffer
{
	ComPtr<ID3D11Buffer> buffer;
	UINT count = 0;
	void set(ID3D11DeviceContext* immediateContext);
};

struct Mesh
{
	VertexBuffer vertexBuffer{};
	IndexBuffer indexBuffer{};
	void set(ID3D11DeviceContext* immediateContext, UINT slot, UINT offset = 0);
};

struct Geometry
{
	struct Vertex
	{
		Float3 position;
		Float3 normal;
	};
	VertexBuffer vertexBuffer{};
	IndexBuffer indexBuffer{};
	void set(ID3D11DeviceContext* immediateContext);
};

enum class SamplerState { point, linear, anisotropic };
enum class DepthStencilState { none, common };
enum class BlendState { none, alpha, add };
enum class RasterizerState { solid, wireframe };

class PipelineState
{
private:
	std::map<SamplerState, ComPtr<ID3D11SamplerState>> samplerStates;
	std::map<DepthStencilState, ComPtr<ID3D11DepthStencilState>> depthStencilStates;
	std::map<BlendState, ComPtr<ID3D11BlendState>> blendStates;
	std::map<RasterizerState, ComPtr<ID3D11RasterizerState>> rasterizerStates;
public:
	PipelineState(ID3D11Device* device);

	void setDepthStencilState(
		ID3D11DeviceContext* immediateContext,
		DepthStencilState depthStencilState,
		UINT stencilRef = 0);

	void setSamplerStates(
		ID3D11DeviceContext* immediateContext,
		SamplerState samplerState,
		UINT slot,
		bool useVs = true,
		bool usePs = true,
		bool useDs = true,
		bool useHs = true,
		bool useGs = true);

	void setBlendState(ID3D11DeviceContext* immediateContext, BlendState blendState);

	void setRasterizerState(ID3D11DeviceContext* immediateContext, RasterizerState rasterizerState);

	virtual ~PipelineState() = default;
};

class Painter : public PipelineState
{
public:
	Painter(ID3D11Device* device) :PipelineState(device) {}
	virtual ~Painter() = default;
	virtual void drawBegin(ID3D11DeviceContext* immediateContext);
	virtual void drawEnd(ID3D11DeviceContext* immediateContext);
private:
	std::stack<CachedHandle> cachedHandle;
protected:
	virtual void pushState(ID3D11DeviceContext* immediateContext)final;
	virtual void popState(ID3D11DeviceContext* immediateContext)final;
};

void makeCube(ID3D11Device* device, Geometry* cube);
void makeSphere(ID3D11Device* device, Geometry* sphere, UINT slices = 32, UINT stacks = 32);

HRESULT loadPixelShader(ID3D11Device* device, PixelShader* outPs, const char* path);
HRESULT loadVertexShader(ID3D11Device* device, VertexShader* outVs, const char* path, D3D11_INPUT_ELEMENT_DESC* descs = 0, UINT descsArrSize = 0);
HRESULT loadDomainShader(ID3D11Device* device, DomainShader* outDs, const char* path);
HRESULT loadHullShader(ID3D11Device* device, HullShader* outHs, const char* path);
HRESULT loadGeometryShader(ID3D11Device* device, GeometryShader* outGs, const char* path);
HRESULT loadShaderResource(ID3D11Device* device, ShaderResource* outSr, const wchar_t* path);
HRESULT createStructuredBuffer(ID3D11Device* device, StructuredBuffer* outSb, UINT elementSize, UINT count, void* initData = 0);
HRESULT createConstantBuffer(ID3D11Device* device, ConstantBuffer* outCb, UINT elementSize, void* initData = 0);
HRESULT createRenderTextrue(ID3D11Device* device, RenderTexture* outRt, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
HRESULT createDepthTextrue(ID3D11Device* device, DepthTexture* outDt, UINT width, UINT height);
HRESULT createLayer(ID3D11Device* device, Layer* outLayer, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
HRESULT createVertexBuffer(ID3D11Device* device, VertexBuffer* outVertexBuffer, UINT stride, UINT count, const void* initialValue);
HRESULT createIndexBuffer(ID3D11Device* device, IndexBuffer* outIndexBuffer, UINT count, const UINT* initialValue);

