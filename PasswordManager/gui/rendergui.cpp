#include "renderGui.h"
//#include <render/drawing.h>
#include "drawing.h"

#include <files_helper/files.h>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

static int currentTabId = 0;
static int nextTabId = currentTabId;

static Animation contentAnim = Animation(Animation::forward, 0.25f);

template<typename T>
class MoveFrameForward
{
private:
	T m_value;
	int m_frameId = 0;

public:
	void Set(const T& value)
	{
		if (m_value != value)
		{
			m_frameId = 0;
			m_value = value;
		}
	};

	T Get(int frameId = 0)
	{
		m_frameId++;
		if (m_frameId - 1 < frameId)
			return T();
		return m_value;
	};
};

void RenderGUI::RenderBody(const HWND& window)
{
	//static MoveFrameForward<bool> mff;
	//if (mff.Get(3) == true)
	//	mff.Set(false);
	//else
	//	mff.Set(true);


	animation_show.Proceed();
	animation_maximize.Proceed();

	contentAnim.Proceed();
	if (contentAnim.StartAfter(contentAnim, Animation::back, Animation::forward))
		currentTabId = nextTabId;

	ImGuiStyle* style = &ImGui::GetStyle();
	ImGuiIO* io = &ImGui::GetIO();

	float maximizeValue = animation_maximize.GetValueSin();

	float maximizeClamp101 = (maximizeValue - 0.5f) * 2;
	//margin = lerpf(20, 0, maximizeValue);
	//style->WindowRounding = margin / 5;

	ImVec2 hWndPosOverride = ImLerp(ImVec2(RenderGUI::windowPosX, RenderGUI::windowPosY), ImVec2(0, 0), maximizeValue);
	ImVec2 hWndSizeOverride = ImLerp(ImVec2(RenderGUI::GetFullWidth(), RenderGUI::GetFullHeight()), ImVec2(1920, 1080), maximizeValue);

	//ImVec2 windowPos = ImLerp(ImVec2(margin, margin), ImVec2(0, 0), maximizeValue);
	//ImVec2 windowSize = ImLerp(ImVec2(windowWidth, windowHeight), ImVec2(1920, 1080), maximizeValue);

	/*
	static int runCount = 0;

	runCount += animation_maximize.IsRunning() ? 1 : -1;
	runCount = clampf(0, 2, runCount);
	*/

	ImVec2 windowPos = ImLerp(animation_maximize.IsRunning() ? ImVec2(windowPosX, windowPosY) : ImVec2(margin, margin), ImVec2(0, 0), maximizeValue);
	ImVec2 windowSize = ImLerp(ImVec2(windowWidth, windowHeight), hWndSizeOverride, maximizeValue);

	float animationValue = animation_show.GetValueSin();
	float alpha = Animation::lerp(0, 0.98f, animationValue);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
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

			constexpr float titleBarBtnCount = 3;

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - titleBtnWidth * titleBarBtnCount - style->ItemSpacing.x * maxf(titleBarBtnCount - 1, 1));
			ImVec2 HidePos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			ImVec2 MaximizePos = HidePos + ImVec2(titleBtnWidth + style->ItemSpacing.x, 0);
			ImVec2 CrossPos = MaximizePos + ImVec2(titleBtnWidth + style->ItemSpacing.x, 0);

			if (items::ShadowButton("##minimizeBtn", titleBtnSize))
			{
				animation_show.Start(Animation::back);
			}
			ImGui::SameLine();

			MaximizePos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			static MoveFrameForward<bool> isGoingToBig;
			if (items::ShadowButton("##maximizeBtn", titleBtnSize) && animation_maximize.AnimationEnded())
			{
				animation_maximize.Start();
				isGoingToBig.Set(animation_maximize.GetState() != Animation::back);

				//ImVec2 posOverride = ImLerp(ImVec2(RenderGUI::windowPosX, RenderGUI::windowPosY), ImVec2(0, 0), animation_maximize.GetState() != Animation::State::back);
				//ImVec2 sizeOverride = ImLerp(ImVec2(RenderGUI::GetFullWidth(), RenderGUI::GetFullHeight()), ImVec2(1920, 1080), animation_maximize.GetState() != Animation::State::back);
				//
				//SetWindowPos(window, NULL,
				//	posOverride.x, posOverride.y,
				//	sizeOverride.x, sizeOverride.y,
				//	SWP_SHOWWINDOW | SWP_NOZORDER);
			}
			ImGui::SameLine();

			CrossPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			if (items::ShadowButton("##closeBtn", titleBtnSize))
			{
				animation_show.Start(Animation::back);
				running = false;
			}

			draw::Underline(ImVec2(HidePos.x + titleBtnWidth / 2, HidePos.y + titleBtnWidth / 2), titleBtnPd, colors::text);
			draw::CornersMarks(ImVec2(MaximizePos.x + titleBtnWidth / 2, MaximizePos.y + titleBtnWidth / 2), titleBtnPd, -1 * maximizeClamp101, colors::text);
			draw::Cross(ImVec2(CrossPos.x + titleBtnWidth / 2, CrossPos.y + titleBtnWidth / 2), titleBtnPd * animation_show.GetValueSin(), colors::text);

			static bool lastTick = animation_maximize.IsRunning();
			if (animation_maximize.IsRunning() || lastTick)
			{
				//SetWindowPos(window, NULL,
				//	hWndPosOverride.x, hWndPosOverride.y,
				//	hWndSizeOverride.x, hWndSizeOverride.y,
				//	SWP_SHOWWINDOW | SWP_NOZORDER);
			}
			if (animation_maximize.AnimationEnded() && lastTick || isGoingToBig.Get(1))
			{
				ImVec2 posOverride = ImLerp(ImVec2(RenderGUI::windowPosX, RenderGUI::windowPosY), ImVec2(0, 0), animation_maximize.GetState() != Animation::State::back);
				ImVec2 sizeOverride = ImLerp(ImVec2(RenderGUI::GetFullWidth(), RenderGUI::GetFullHeight()), ImVec2(1920, 1080), animation_maximize.GetState() != Animation::State::back);

				SetWindowPos(window, NULL,
					posOverride.x, posOverride.y,
					sizeOverride.x, sizeOverride.y,
					SWP_SHOWWINDOW | SWP_NOZORDER);
			}
			lastTick = animation_maximize.IsRunning();
			ImGui::Text("%.3f", RenderGUI::animation_maximize.GetValue());
		}


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::background * colors::darker);

		ImVec2 wndSize = ImGui::GetWindowSize();
		ImVec2 loginChildSize = ImVec2(340, 230);
		ImVec2 deltaPos = wndSize - loginChildSize + style->WindowPadding;

		ImGui::SetNextWindowPos(windowPos + ImVec2(deltaPos.x / 2, deltaPos.y / 2));
		ImGui::BeginChild("LoginChild", loginChildSize, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			ImGui::Text("Enter your encryption key");
			ImGui::NewLine();

			static char szKeyBuffer[MAX_PATH] = { '\0' };
			ImGui::InputText("##input_encryption_key", szKeyBuffer, sizeof(szKeyBuffer) / sizeof(*szKeyBuffer));
			//ImGui::SameLine();

			if (ImGui::Button("Decrypt"))
			{

			}
			ImGui::SameLine();
			if (ImGui::Button("Encrypt"))
			{

			}

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

ImVec2 RenderGUI::GetFullSize()
{
	return ImVec2(GetFullWidth(), GetFullHeight());
}

int RenderGUI::GetFullWidth()
{
	return RenderGUI::windowWidth + RenderGUI::margin * 2;
}

int RenderGUI::GetFullHeight()
{
	return RenderGUI::windowHeight + RenderGUI::margin * 2;
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

	initialized = true;

	bool inCycle = running || !animation_show.AnimationEnded(Animation::back);
	while (inCycle)
	{
		inCycle = running || !animation_show.AnimationEnded(Animation::back);
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

		ImGuiIO& io = ImGui::GetIO();
		float maximizeValue = animation_maximize.GetValueSin();
		ImVec2 newSize = ImLerp(ImVec2(RenderGUI::GetFullWidth(), RenderGUI::GetFullHeight()), ImVec2(1920, 1080), maximizeValue);
		io.DisplaySize.x = newSize.x;
		io.DisplaySize.y = newSize.y;



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

	auto ReleaseDx = []() {
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

void RenderGUI::ResizeRenderTarget()
{
	if (!initialized) return;
	if (pTarget_view) {
		pTarget_view->Release();
		pTarget_view = nullptr;
	}

	// Resize the swap chain
	pSwap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	// Create a new render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	pSwap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTarget_view);
	pBackBuffer->Release();

	// Update viewport dimensions
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<float>(windowWidth);
	vp.Height = static_cast<float>(windowHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1, &vp);

}

