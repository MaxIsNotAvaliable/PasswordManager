#pragma once
#include <base.h>

#include <anime/animation.h>

#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")


class RenderGUI
{
private:
	static inline bool running = true;
	static void RenderBody(const HWND& window);
	static void RenderEffects();
	static void SetupStyle();
	static void InitializeImGui();

public:
	static inline Animation show_animation = Animation(0.25f);
	static inline POINTS position = {};
	static inline ImVec2 size;
	static inline float margin;
	static inline float titleBarHeight = 36;
	static bool Initialize(const HWND& window, const WNDCLASSEXA& wc, const int& cmd_show);
	static bool Running();
};