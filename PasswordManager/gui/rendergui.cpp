#include "renderGui.h"
#include <render/drawing.h>

#include <files_helper/files.h>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

static int currentTabId = 0;
static int nextTabId = currentTabId;

static Animation contentAnim = Animation(Animation::forward, 0.25f);

void RenderGUI::RenderBody(const HWND& window)
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImGuiIO* io = &ImGui::GetIO();

	show_animation.Proceed();

	contentAnim.Proceed();
	if (contentAnim.StartAfter(contentAnim, Animation::back, Animation::forward))
		currentTabId = nextTabId;

	ImVec2 windowPos = ImVec2(margin, margin);
	ImVec2 windowSize = size - ImVec2(margin, margin) * 2;
	float animationValue = show_animation.GetValueSin();
	float alpha = Animation::lerp(0, 0.98f, animationValue);

	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowBgAlpha(alpha);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animationValue);

	const float fontSize = ImGui::GetFontSize();

	if (ImGui::Begin("Password Manager", &running, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
	{
		// title bar
		{
			ImGui::GetWindowDrawList()->AddRectFilled(windowPos, windowPos + ImVec2(windowSize.x, RenderGUI::titleBarHeight), ImGui::GetColorU32(colors::background * colors::darker), style->WindowRounding);

			const ImVec2 titleBtnSize = ImVec2(30, 30);
			const ImVec2 injectBtnSz = ImVec2(170, 48);

			const float titleBtnWidth = titleBtnSize.x;
			const float titleBtnPd = titleBtnWidth / 4.5f;

			ImGui::SetCursorPosX(10);

			ImGui::Text("Password manager");
			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - titleBtnWidth * 2 - style->ItemSpacing.x);
			ImVec2 HidePos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			ImVec2 CrossPos = HidePos + ImVec2(titleBtnWidth + style->ItemSpacing.x, 0);

			if (items::ShadowButton("##minimizeBtn", titleBtnSize))
			{
				show_animation.Start(Animation::back);
			}

			ImGui::SameLine();
			if (items::ShadowButton("##closeBtn", titleBtnSize))
			{
				show_animation.Start(Animation::back);
				running = false;
			}

			draw::Underline(ImVec2(HidePos.x + titleBtnWidth / 2, HidePos.y + titleBtnWidth / 2), titleBtnPd, colors::text);
			draw::Cross(ImVec2(CrossPos.x + titleBtnWidth / 2, CrossPos.y + titleBtnWidth / 2), titleBtnPd * show_animation.GetValueSin(), colors::text);
		}


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::background * colors::darker);

		ImVec2 loginChildSize = ImVec2(340, 230);
		ImVec2 deltaPos = windowSize - loginChildSize + style->WindowPadding;
		
		ImGui::SetNextWindowPos(ImVec2(deltaPos.x / 2, deltaPos.y / 2));
		ImGui::BeginChild("LoginChild", loginChildSize, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Text("Enter your encryption key");
			ImGui::NewLine();

			static char szKeyBuffer[MAX_PATH] = { '\0' };
			ImGui::InputText("##input_encryption_key", szKeyBuffer, sizeof(szKeyBuffer) / sizeof(*szKeyBuffer));
			ImGui::SameLine();
			ImGui::Button("Decrypt");

			ImGui::NewLine();
			ImGui::Button("Key lost");
			//ImGui::SameLine();
			//ImGui::Button("Create new file");

			/*
			ispolzovat kluch vosstanovleniya dlya:
			key -> decrypt
			revocation key -> key -> decrypt
			*/


		}

		ImGui::EndChild();
		ImGui::PopStyleColor();


		ImGui::PopStyleVar();
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void RenderGUI::RenderEffects()
{

}

void RenderGUI::SetupStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowRounding = 4;
	style->WindowBorderSize = 0;
	style->WindowShadowSize = margin;
	style->WindowMinSize = ImVec2(100, 100);
	style->WindowPadding = ImVec2(0, 0);

	style->FrameRounding = 4;
	style->FramePadding = ImVec2(8, 8);

	style->ChildRounding = 4;


	style->Colors[ImGuiCol_WindowBg] = colors::background;

	style->Colors[ImGuiCol_Text] = colors::text;

	style->Colors[ImGuiCol_ChildBg] = colors::transparent;
	style->Colors[ImGuiCol_DragDropTarget] = colors::text;

	style->Colors[ImGuiCol_Button] = colors::frame * ImVec4(1, 1, 1, 0.0f);
	style->Colors[ImGuiCol_ButtonHovered] = colors::frameHovered * ImVec4(1, 1, 1, 0.3f);
	style->Colors[ImGuiCol_ButtonActive] = colors::frameActive * ImVec4(1, 1, 1, 0.3f);

	style->Colors[ImGuiCol_BorderShadow] = colors::black;
	style->Colors[ImGuiCol_WindowShadow] = colors::black;

	style->Colors[ImGuiCol_TitleBg] = colors::transparent;
	style->Colors[ImGuiCol_TitleBgActive] = colors::transparent;
	style->Colors[ImGuiCol_TitleBgCollapsed] = colors::transparent;

	style->Colors[ImGuiCol_CheckMark] = colors::text;
	style->Colors[ImGuiCol_PopupBg] = colors::background;

	style->Colors[ImGuiCol_FrameBg] = colors::frame;
	style->Colors[ImGuiCol_FrameBgHovered] = colors::frameHovered;
	style->Colors[ImGuiCol_FrameBgActive] = colors::frameActive;

	style->Colors[ImGuiCol_SliderGrab] = colors::white;
	style->Colors[ImGuiCol_SliderGrabActive] = colors::text;

	style->Colors[ImGuiCol_ScrollbarBg] = colors::transparent;
	style->Colors[ImGuiCol_ScrollbarGrab] = colors::frame;
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = colors::frameActive;
	style->Colors[ImGuiCol_ScrollbarGrabActive] = colors::text;

	style->Colors[ImGuiCol_ModalWindowDimBg] = colors::transparent;
}

void RenderGUI::InitializeImGui()
{
	//std::string dir = manage_files::GetFolder();
	ImGuiIO* io = &ImGui::GetIO();

	io->IniFilename = NULL;
	io->LogFilename = NULL;

	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;

	//fonts::regular_14 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_regular, sizeof(ttf_bytes_inter_regular), 14.f, &fontConfig);
	//fonts::bold_16 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 16.f, &fontConfig);
	//fonts::bold_20 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 20.f, &fontConfig);
	//fonts::bold_28 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 28.f, &fontConfig);
}

bool RenderGUI::Initialize(const HWND& window, const WNDCLASSEXA& wc, const int& cmd_show)
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};

	ID3D11Device* pDevice{ nullptr };
	ID3D11DeviceContext* pContext{ nullptr };
	IDXGISwapChain* pSwap_chain{ nullptr };
	ID3D11RenderTargetView* pTarget_view{ nullptr };
	D3D_FEATURE_LEVEL level{};

	HRESULT hrCreateDevice = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap_chain,
		&pDevice,
		&level,
		&pContext
	);

	if (hrCreateDevice != S_OK)
		return false;

	ID3D11Texture2D* back_buffer{ nullptr };
	if (pSwap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer)) != S_OK)
		return false;

	if (back_buffer)
	{
		if (pDevice->CreateRenderTargetView(back_buffer, nullptr, &pTarget_view) != S_OK)
			return false;

		back_buffer->Release();
	}
	else return false;

	ShowWindow(window, cmd_show);
	UpdateWindow(window);

	// IMGUI 

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	SetupStyle();

	InitializeImGui();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

	bool inCycle = running || !show_animation.AnimationEnded(Animation::back);
	while (inCycle)
	{
		inCycle = running || !show_animation.AnimationEnded(Animation::back);
		MSG msg;
		while (PeekMessageA(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);

			if (msg.message == WM_QUIT)
				running = false;
		}

		if (!inCycle)
			break;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		RenderBody(window);

		ImGui::Render();
		constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
		pContext->OMSetRenderTargets(1U, &pTarget_view, nullptr);
		pContext->ClearRenderTargetView(pTarget_view, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		pSwap_chain->Present(1U, 0U);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	auto ReleaseDx = [&pSwap_chain, &pDevice, &pContext, &pTarget_view]() {
		if (pSwap_chain)	pSwap_chain->Release();
		if (pDevice)		pDevice->Release();
		if (pContext)		pContext->Release();
		if (pTarget_view)	pTarget_view->Release();
		};

	ReleaseDx();

	DestroyWindow(window);
	UnregisterClassA(wc.lpszClassName, wc.hInstance);

	return true;
}

bool RenderGUI::Running()
{
	return running;
}

