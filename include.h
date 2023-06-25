#pragma once
#include "func/DX11System.h"
#include "func/KeyInput.h"
#include "func/CerealIO.h"
#include "func/FrameworkConfig.h"
#include "func/Arithmetic.h"
#include "painter/Painter.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ScreenGrab.h>
#include <filesystem>
#include <wincodec.h>
#include <Windows.h>
#include <string>

void debugLog(const char*, ...);

inline void saveToPng(
	const std::filesystem::path& filepath,
	DX11System* system,
	ID3D11Resource* resource)
{
	SaveWICTextureToFile(
		system->d3d11DeviceContext.Get(),
		resource,
		GUID_ContainerFormatPng,
		filepath.wstring().c_str());
}

inline void saveToPng(
	const std::filesystem::path& filepath,
	DX11System* system,
	ShaderResource* shaderResource)
{
	ComPtr<ID3D11Resource> resource{};
	shaderResource->resource->GetResource(resource.ReleaseAndGetAddressOf());
	saveToPng(filepath, system, resource.Get());
}
