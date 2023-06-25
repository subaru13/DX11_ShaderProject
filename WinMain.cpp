#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"ImGui.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"winmm.lib")
#include "func/HighResolutionTimer.h"
#include "include.h"

#define CLASSNAME L"Shader"

HWND winInit(HINSTANCE, int);

void guiInit();
void guiNewFrame();
void guiRender();
void guiUninit();
void showLog();

void init(DX11System*);
void update(float);
void draw(DX11System*);
void uninit();

DX11System* dx11System{};
HighResolutionTimer highResolutionTimer{};

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
	(void)CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	dx11System = new DX11System(winInit(instance, cmdShow));
	guiInit();
	init(dx11System);
	highResolutionTimer.Tick();

	MSG msg{};

	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			highResolutionTimer.Tick();
			guiNewFrame();
			KeyManager::instance()->update();
			Mouse::instance()->update(dx11System->hwnd);
			dx11System->clearRenderTargets(nullptr);
			dx11System->setRenderTargets();
			update((float)highResolutionTimer.GetElapsedTime());
			draw(dx11System);
			showLog();
			dx11System->setRenderTargets();
			guiRender();
			dx11System->present();
		}
	} while (WM_QUIT != msg.message);

	uninit();
	guiUninit();
	delete dx11System;
	UnregisterClass(CLASSNAME, instance);
	CoUninitialize();
	return static_cast<int>(msg.wParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

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
	wcex.lpszClassName = CLASSNAME;
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	RECT rc{ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
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
#include <vector>

#define LOG_COUNT 1024
std::vector<std::string> imguiLog{};
bool showLogConsoleOpen{ false };
ImGuiWindowFlags logConsoleFrags{ 0 };

void guiInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	ImGui_ImplWin32_Init(dx11System->hwnd);
	ImGui_ImplDX11_Init(dx11System->d3d11Device.Get(), dx11System->d3d11DeviceContext.Get());
	imguiLog.reserve(LOG_COUNT);
}

void guiNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void guiRender()
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::MenuItem("log console", nullptr, &showLogConsoleOpen);
		ImGui::EndMainMenuBar();
	}
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

void debugLog(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int temp = _vscprintf(fmt, args);
	if (temp > 0)
	{
		std::string& newText = imguiLog.emplace_back();
		size_t len = static_cast<size_t>(temp) + 1;
		newText.resize(len);
		_vsprintf_s_l(newText.data(), len, fmt, NULL, args);
	}
	va_end(args);
}

void showLog()
{
	if (!showLogConsoleOpen)return;
	if (ImGui::Begin("log console", &showLogConsoleOpen, logConsoleFrags))
	{
		if (imguiLog.size() > LOG_COUNT)
		{
			size_t index = imguiLog.size() - LOG_COUNT;
			imguiLog.erase(imguiLog.begin(), imguiLog.begin() + index);
		}

		if (ImGui::MenuItem("clear"))
		{
			imguiLog.clear();
		}

		ImGui::Text("%d/%d", imguiLog.size(), LOG_COUNT);
		if (ImGui::BeginChild("log console child"))
		{
			for (auto it = imguiLog.rbegin(); it != imguiLog.rend(); ++it)
			{
				ImGui::Text(it->c_str());
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
