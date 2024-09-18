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
	static inline ID3D11Device* pDevice{ nullptr };
	static inline ID3D11DeviceContext* pContext{ nullptr };
	static inline IDXGISwapChain* pSwap_chain{ nullptr };
	static inline ID3D11RenderTargetView* pTarget_view{ nullptr };

	static inline bool running = true;
	static inline bool initialized = false;
	static void RenderBody(const HWND& window);
	static void RenderEffects();
	static void SetupStyle();
	static void InitializeImGui();
public:
	static inline Animation animation_show = Animation(0.25f);
	static inline Animation animation_maximize = Animation(0.15f);
	static inline POINTS position = {};
	static inline int windowWidth = 1100;
	static inline int windowHeight = 620;
	static inline int windowPosX = 100;
	static inline int windowPosY = 100;

	static inline float margin = 20;
	static inline float titleBarHeight = 36;
	static ImVec2 GetFullSize();
	static int GetFullWidth();
	static int GetFullHeight();
	static bool Initialize(const HWND& window, const WNDCLASSEXA& wc, const int& cmd_show);
	static bool Running();
	static void ResizeRenderTarget();
};