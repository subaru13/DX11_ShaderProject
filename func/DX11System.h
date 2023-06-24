#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <assert.h>

using namespace Microsoft::WRL;

class DX11System
{
public:
	HWND hwnd;
	ComPtr<ID3D11Device>			d3d11Device;
	ComPtr<ID3D11DeviceContext>		d3d11DeviceContext;
	ComPtr<ID3D11RenderTargetView>	d3d11RenderTarget;
	ComPtr<ID3D11DepthStencilView>	d3d11DepthStencil;
	ComPtr<IDXGISwapChain>			dxgiSwapChain;
	D3D11_VIEWPORT					d3d11Viewport;

	DX11System(HWND hWnd)
		:hwnd(hWnd)
	{
		HRESULT hr = S_OK;
		RECT rc;
		GetClientRect(hwnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;
		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		D3D_FEATURE_LEVEL featureLevel;
		hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
			&sd, &dxgiSwapChain, &d3d11Device, &featureLevel, &d3d11DeviceContext);
		assert(hr == S_OK);

		ComPtr<ID3D11Texture2D> backBuffer = nullptr;
		hr = this->dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.ReleaseAndGetAddressOf()));
		assert(hr == S_OK);

		hr = d3d11Device->CreateRenderTargetView(backBuffer.Get(), nullptr, d3d11RenderTarget.ReleaseAndGetAddressOf());
		assert(hr == S_OK);

		{
			ComPtr<ID3D11Texture2D> depthStencilBuffer{};
			D3D11_TEXTURE2D_DESC texture2dDesc{};
			texture2dDesc.Width = width;
			texture2dDesc.Height = height;
			texture2dDesc.MipLevels = 1;
			texture2dDesc.ArraySize = 1;
			texture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			texture2dDesc.SampleDesc.Count = 1;
			texture2dDesc.SampleDesc.Quality = 0;
			texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
			texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			texture2dDesc.CPUAccessFlags = 0;
			texture2dDesc.MiscFlags = 0;
			hr = d3d11Device->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.ReleaseAndGetAddressOf());
			assert(hr == S_OK);

			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
			depthStencilViewDesc.Format = texture2dDesc.Format;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;
			hr = d3d11Device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, d3d11DepthStencil.ReleaseAndGetAddressOf());
			assert(hr == S_OK);
		}

		d3d11Viewport.TopLeftX = 0;
		d3d11Viewport.TopLeftY = 0;
		d3d11Viewport.Width = static_cast<float>(width);
		d3d11Viewport.Height = static_cast<float>(height);
		d3d11Viewport.MinDepth = 0.0f;
		d3d11Viewport.MaxDepth = 1.0f;
		d3d11DeviceContext->RSSetViewports(1, &d3d11Viewport);
	}

	void clearRenderTargets(const float* clearColor = nullptr)
	{
		static const FLOAT defClearColor[4]{ 0.0f,0.4f,0.4f,1.0f };
		d3d11DeviceContext->ClearRenderTargetView(d3d11RenderTarget.Get(), clearColor ? clearColor : defClearColor);
		d3d11DeviceContext->ClearDepthStencilView(d3d11DepthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void setRenderTargets()
	{
		d3d11DeviceContext->OMSetRenderTargets(1, d3d11RenderTarget.GetAddressOf(), d3d11DepthStencil.Get());
		d3d11DeviceContext->RSSetViewports(1, &d3d11Viewport);
	}

	void present()
	{
		HRESULT hr = dxgiSwapChain->Present(1, 0);
		assert(hr == S_OK);
	}


};
