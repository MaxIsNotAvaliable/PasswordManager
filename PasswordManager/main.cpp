#include "gui/renderGui.h"
#include "encryption/encstring.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <Windows.h>
#include <thread>
#include <dwmapi.h>
#include <future>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool finished = false;


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMessage, wParam, lParam))
		return 0L;

	if (!IsIconic(hWnd) && RenderGUI::animation_show.AnimationEnded(Animation::back))
		ShowWindow(hWnd, SW_MINIMIZE);

	if (uMessage == WM_SYSCOMMAND)
	{
		RenderGUI::animation_show.Start(Animation::forward);
		if ((wParam & 0xFFF0) == SC_KEYMENU)
			return 0L;
	}
	else if (uMessage == WM_DESTROY)
	{
		PostQuitMessage(NULL);
		return 0L;
	}
	else if (uMessage == WM_LBUTTONDOWN)
	{
		RenderGUI::position = MAKEPOINTS(lParam);
		return 0L;
	}
	else if (uMessage == WM_MOUSEMOVE)
	{
		if (wParam == MK_LBUTTON && RenderGUI::animation_maximize.AnimationEnded(Animation::back))
		{
			const POINTS points = MAKEPOINTS(lParam);
			RECT rect = RECT{};

			GetWindowRect(hWnd, &rect);

			rect.left += points.x - RenderGUI::position.x;
			rect.top += points.y - RenderGUI::position.y;

			if (RenderGUI::position.x >= RenderGUI::margin &&
				RenderGUI::position.x <= RenderGUI::GetFullWidth() - RenderGUI::margin &&
				RenderGUI::position.y >= RenderGUI::margin && RenderGUI::position.y <= RenderGUI::titleBarHeight + RenderGUI::margin)
			{
				SetWindowPos(
					hWnd, HWND_TOPMOST,
					rect.left, rect.top, 0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
				RenderGUI::windowPosX = rect.left;
				RenderGUI::windowPosY = rect.top;
			}

		}
		return 0L;
	}
	else if (uMessage == WM_SIZE)
	{
		if (wParam != SIZE_MINIMIZED) {
			// Update the viewport and render target view here
			// Resize your swap chain and recreate the render target
			RenderGUI::ResizeRenderTarget();
		}

	}
	return DefWindowProcA(hWnd, uMessage, wParam, lParam);
}

void InitializeWindow(HINSTANCE hInstance, int nShowCmd)
{
	const char* class_name = "PasswordManagerClass";
	const char* window_name = "Password Manager";

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = class_name;

	RegisterClassExA(&wc);

	RenderGUI::animation_show.SetDelay(0.5f);
	RenderGUI::animation_show.Start(Animation::forward);
	RenderGUI::animation_show.SetDelay(0.f);
	RenderGUI::position = { SHORT(RenderGUI::windowPosX), SHORT(RenderGUI::windowPosY) };

	const HWND window = CreateWindowExA(
		WS_EX_TRANSPARENT |
		0,
		class_name,
		window_name,
		WS_POPUP,
		RenderGUI::position.x, RenderGUI::position.y,
		int(RenderGUI::GetFullWidth()), int(RenderGUI::GetFullHeight()),
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		GetClientRect(window, &client_area);

		RECT window_area{};
		GetWindowRect(window, &window_area);

		POINT diff{};
		ClientToScreen(window, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(window, &margins);
	}

	RenderGUI::Initialize(window, wc, nShowCmd);
	finished = true;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	auto encryptionWorker = [&]()
		{
			while (!finished)
			{

				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
		};


	std::thread thWin(InitializeWindow, hInstance, nShowCmd);
	//std::thread thEncr(encryptionWorker);

	thWin.join();
	//thEncr.join();

	return EXIT_SUCCESS;
}