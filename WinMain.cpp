#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"ImGui.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <wincodec.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ScreenGrab.h>
#include "func/CerealIO.h"
#include "func/DX11System.h"
#include "func/KeyInput.h"
#include "func/CameraControl.h"
#include "example/example.h"

#define TEST_WP 0

HWND winInit(HINSTANCE, int);

void guiInit(DX11System*);
void guiNewFrame();
void guiRender();
void guiUninit();

int WINAPI WinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE prevInstance,
	_In_ LPSTR cmdLine,
	_In_ int cmdShow)
{
	using namespace DirectX;

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	auto hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	DX11System system{ winInit(instance, cmdShow) };

	guiInit(&system);

	Geometry sphere{};
	makeSphere(system.d3d11Device.Get(), &sphere, 32, 32);

	CameraControl cameraControl{};
	ToonPainter toonPainter{ system.d3d11Device.Get() };
	toonPainter.data.world = matrixToFloat4x4(XMMatrixIdentity());

	Deserialize(".json", toonPainter.data);

	Float4 clearColor{ colorCodeToRGBA(0x4FB6FFFF) };

	MSG msg{};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			guiNewFrame();
			KeyManager::instance()->update();
			Mouse::instance()->update(system.hwnd);

			if (ImGui::Begin("toonShader"))
			{
				ImGui::ColorEdit4("clearColor", &clearColor.x);
				ImGui::DragFloat3("eyePos", (float*)cameraControl.getPos(), 0.01f);
				if (ImGui::SliderFloat3("lightDir", &toonPainter.data.lightDir.x, -1.0f, 1.0f))
				{
					if (XMVector3IsNaN(XMLoadFloat3(&toonPainter.data.lightDir)))
					{
						toonPainter.data.lightDir = Float3{ 0,-1.0f,0 };
					}
					else
					{
						toonPainter.data.lightDir = vec3Normalize(toonPainter.data.lightDir);
					}
				}
				ImGui::ColorEdit4("matColor", &toonPainter.data.matColor.x);
				ImGui::ColorEdit4("rimColor", &toonPainter.data.rimColor.x);
				if (ImGui::DragInt("toonLevels", &toonPainter.data.toonLevels))
				{
					toonPainter.data.toonLevels = max(toonPainter.data.toonLevels, 2);
				}
				if (ImGui::DragFloat("outlineThreshold", &toonPainter.data.outlineThreshold,0.01f))
				{
					toonPainter.data.outlineThreshold = min(max(toonPainter.data.outlineThreshold, 0.0f), 1.0f);
				}
			}
			ImGui::End();
			system.clearRenderTargets(&clearColor.x);


			auto view = cameraControl.getView();
			auto proj = cameraControl.getProjection();
			toonPainter.data.viewProjection = matrixToFloat4x4(XMLoadFloat4x4(&view) * XMLoadFloat4x4(&proj));
			std::memcpy(&toonPainter.data.eyePos, cameraControl.getPos(), sizeof(Float3));
			toonPainter.drawBegin(system.d3d11DeviceContext.Get());
			toonPainter.draw(system.d3d11DeviceContext.Get(), &sphere);
			toonPainter.drawEnd(system.d3d11DeviceContext.Get());

			guiRender();
			system.present();
		}
	}
	Serialize(".json", toonPainter.data);

	guiUninit();
	UnregisterClass(L"Shader", instance);
	CoUninitialize();
	return static_cast<int>(msg.wParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK handleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

HWND winInit(HINSTANCE instance, int cmdShow)
{
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = handleMessage;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Shader";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	RECT rc{ 0, 0, 1280, 720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowEx(0, wcex.lpszClassName, L"Shader",
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, instance, NULL);
	ShowWindow(hwnd, cmdShow);
	return hwnd;
}

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

void guiInit(DX11System* system)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	ImGui_ImplWin32_Init(system->hwnd);
	ImGui_ImplDX11_Init(system->d3d11Device.Get(), system->d3d11DeviceContext.Get());
}

void guiNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void guiRender()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void guiUninit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
