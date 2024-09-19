#include "renderGui.h"
//#include <render/drawing.h>
#include "drawing.h"

#include "../encryption/encrypt.h"
#include "../encryption/PasswordData.h"

#include <files_helper/files.h>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

static int currentTabId = 0;
static int nextTabId = currentTabId;

static Animation animation_content = Animation(Animation::forward, 0.25f);
static CPasswordDataManager passwordManager;

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

void RenderLoginPage()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec2 wndSize = ImGui::GetWindowSize();
	ImVec2 wndPos = ImGui::GetWindowPos();
	ImVec2 loginChildSize = ImVec2(340, 230);
	ImVec2 profilesChildSize = ImVec2(240, 430);
	ImVec2 deltaPos = wndSize - (ImVec2(loginChildSize.x, 0) + profilesChildSize) + style->WindowPadding;

	ImGui::SetNextWindowPos(wndPos + ImVec2(deltaPos.x / 2, deltaPos.y / 2));
	ImGui::BeginChild("SelectProfileChild", profilesChildSize, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		ImVec2 center = tools::LocalToGlobalPos(ImGui::GetCursorPos());
		
		//ImVec2 btnSize = tools::CalcItemSize("", ImVec2(ImGui::GetWindowWidth() - style->WindowPadding.x * 2, 0));
		//ImVec2 btnSize = ImVec2(ImGui::GetWindowWidth() - style->WindowPadding.x * 2, ImGui::GetFontSize());
		ImVec2 btnSize = tools::CalcItemSize("", ImVec2(tools::CalcWindowSpace().x, 0));
		ImGui::Button("Create new", btnSize);
		draw::AddPrevButtonEffect(btnSize);
		static std::string passwordsFolder = manage_files::GetFolder() + "\\passwords";
		if (!manage_files::folderExists(passwordsFolder) && ImGui::Button("+ create folder"))
		{
			manage_files::createDirectory(passwordsFolder);
		}
		else
		{
			static int selectedFile = -1;
			static std::vector<std::string> files = manage_files::GetFilesInDirectory(passwordsFolder, true);

			static Animation animationUpdate(9.9f);
			animationUpdate.Proceed();
			if (animationUpdate.StartAfter(animationUpdate, Animation::back, Animation::forward))
			{
				animationUpdate.SetDuration(1);
			}
			if (animationUpdate.AnimationEnded(Animation::forward))
			{
				animationUpdate.SetDuration(9.9f);
				animationUpdate.Start(Animation::back);
				files = manage_files::GetFilesInDirectory(passwordsFolder, true);
			}
			//if (animationUpdate.AnimationEnded(Animation::back))
			//{
			//	files = manage_files::GetFilesInDirectory(passwordsFolder, true);
			//	animationUpdate.ForceDirection(Animation::forward);
			//}
			//else if (animationUpdate.AnimationEnded())
			//{
			//	animationUpdate.Start(Animation::back);
			//}
			
			ImGui::Text("Update in %.1f sec...", animationUpdate.GetState() != Animation::back ? 0 : animationUpdate.GetValue() * animationUpdate.GetDuration());
			ImVec2 linePos = tools::LocalToGlobalPos(ImGui::GetCursorPos() + ImVec2(0, 0));
			ImGui::GetWindowDrawList()->AddRectFilled(linePos, linePos + ImVec2(tools::CalcWindowSpace().x * animationUpdate.GetValueSin(), 2), ImGui::GetColorU32(colors::active), style->WindowRounding);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style->FramePadding.y);
			bool needUpdate = false;
			
			//static float totalTime = 0;
			//totalTime += ImGui::GetIO().DeltaTime;

			for (size_t i = 0; i < files.size(); i++)
			{
				ImVec2 pos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
				if (ImGui::Button(files[i].c_str(), btnSize))
					selectedFile = i;
				if (selectedFile == i)
					draw::AddButtonEffectArrow(pos, 0, btnSize);
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (profilesChildSize.y - loginChildSize.y) / 2);
	ImGui::BeginChild("LoginChild", loginChildSize, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		ImGui::Text("Enter your encryption key");
		ImGui::NewLine();

		static bool visible = false;
		ImGuiInputTextFlags inputTextFlags = 0;
		if (!visible) inputTextFlags |= ImGuiInputTextFlags_Password;

		constexpr int maxPassKeyLen = 0x100;
		static char szKeyBuffer[maxPassKeyLen] = { '\0' };
		static char szFolderBuffer[maxPassKeyLen] = { '\0' };

		ImGui::InputText("##input_encryption_key", szKeyBuffer, sizeof(szKeyBuffer) / sizeof(*szKeyBuffer), inputTextFlags);
		ImGui::SameLine();
		ImGui::Checkbox("<o>##visible", &visible);

		bool isGoodLen = strlen(szKeyBuffer) >= 16;
		ImGui::Text("[%s] | [%i / %i]", isGoodLen ? "good" : "weak", int(strlen(szKeyBuffer)), maxPassKeyLen - 1);

		if (ImGui::Button("Generate key"))
		{
			for (size_t i = 0; i < 0x10; i++)
			{
				szKeyBuffer[i] = rand() % 0x5F + 0x20;
			}
			szKeyBuffer[0x10] = '\0';
		}
		ImGui::NewLine();

		if (ImGui::Button("Decrypt") && isGoodLen)
		{
			passwordManager.SetKey(szKeyBuffer);
			nextTabId = 1;
			animation_content.Start(Animation::back);
		}

		ImGui::SameLine();
		ImGui::Button("Key lost");

		/*
		ispolzovat kluch vosstanovleniya dlya:
		key -> decrypt
		revocation key -> key -> decrypt
		*/
	}
	ImGui::EndChild();
}

void RenderContentPage()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	ImGui::BeginChild("ContentChild", ImVec2(), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg * style->Alpha, colors::darkerAlpha);
		ImGui::BeginChild("LeftPage", ImVec2(300, 0));
		{
			static std::string passwordsFolder = manage_files::GetFolder() + "\\passwords";
			if (!manage_files::folderExists(passwordsFolder) && ImGui::Button("+ create folder"))
			{
				manage_files::createDirectory(passwordsFolder);
			}
			else
			{
				ImVec2 btnSize = ImVec2(ImGui::GetWindowWidth(), 0);
				ImGui::Button("+ New file", btnSize);
				std::vector<std::string> files = manage_files::GetFilesInDirectory(passwordsFolder, true);
				for (size_t i = 0; i < files.size(); i++)
				{
					ImGui::Button(files[i].c_str(), btnSize);
				}
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	ImGui::EndChild();
}


void RenderGUI::RenderBody(const HWND& window)
{
	animation_show.Proceed();
	animation_maximize.Proceed();
	animation_content.Proceed();

	//if (currentTabId != nextTabId)
	//	animation_content.Start();

	if (animation_content.StartAfter(animation_content, Animation::back, Animation::forward))
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

	ImVec2 windowSize = ImLerp(ImVec2(windowWidth, windowHeight), hWndSizeOverride, minf(maximizeValue * 2, 1));
	ImVec2 windowPosNormal = animation_maximize.IsRunning() ? ImVec2(windowPosX, windowPosY) : ImVec2(margin, margin);
	ImVec2 windowPosCenter = (hWndSizeOverride - hWndPosOverride);
	//ImVec2 windowPos = ImLerp(ImLerp(windowPosNormal, ImVec2(0, 0), maximizeValue), ImLerp(ImVec2(windowPosCenter.x / 2, windowPosCenter.y / 2), ImVec2(0, 0), maximizeValue), maximizeValue);
	ImVec2 windowPos = ImLerp(windowPosNormal, ImVec2(0, 0), maximizeValue);

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
			ImGui::GetWindowDrawList()->AddRectFilled(windowPos, windowPos + ImVec2(windowSize.x, RenderGUI::titleBarHeight), ImGui::GetColorU32(colors::background * colors::darker * colors::darker), style->WindowRounding);

			const ImVec2 titleBtnSize = ImVec2(30, 30);
			const ImVec2 injectBtnSz = ImVec2(170, 48);

			const float titleBtnWidth = titleBtnSize.x;
			const float titleBtnPd = titleBtnWidth / 4.5f;

			ImGui::SetCursorPosX(10);

			ImGui::Text("Password manager");
			ImGui::SameLine();

#if ALLOW_WINDOW_RESIZE
			constexpr float titleBarBtnCount = 3;
#endif
			constexpr float titleBarBtnCount = 2;

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - titleBtnWidth * titleBarBtnCount - style->ItemSpacing.x * maxf(titleBarBtnCount - 1, 1));
			ImVec2 HidePos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			ImVec2 MaximizePos = HidePos + ImVec2(titleBtnWidth + style->ItemSpacing.x, 0);
			ImVec2 CrossPos = MaximizePos + ImVec2(titleBtnWidth + style->ItemSpacing.x, 0);

			if (items::ShadowButton("##minimizeBtn", titleBtnSize))
			{
				animation_show.Start(Animation::back);
			}
			ImGui::SameLine();

			static MoveFrameForward<bool> isGoingToBig;
#if ALLOW_WINDOW_RESIZE
			MaximizePos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			if (items::ShadowButton("##maximizeBtn", titleBtnSize) && animation_maximize.AnimationEnded())
			{
				animation_maximize.Start();
				isGoingToBig.Set(animation_maximize.GetState() != Animation::back);
			}
			ImGui::SameLine();
#endif

			CrossPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			if (items::ShadowButton("##closeBtn", titleBtnSize))
			{
				animation_show.Start(Animation::back);
				running = false;
			}

			draw::Underline(ImVec2(HidePos.x + titleBtnWidth / 2, HidePos.y + titleBtnWidth / 2), titleBtnPd, colors::text);
			//draw::CornersMarks(ImVec2(MaximizePos.x + titleBtnWidth / 2, MaximizePos.y + titleBtnWidth / 2), titleBtnPd, -1 * maximizeClamp101, colors::text);
			draw::Cross(ImVec2(CrossPos.x + titleBtnWidth / 2, CrossPos.y + titleBtnWidth / 2), titleBtnPd * animation_show.GetValueSin(), colors::text);

			static bool lastTick = animation_maximize.IsRunning();
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
			//ImGui::Text("%.3f", RenderGUI::animation_maximize.GetValue());
		}


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::background * colors::darker);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animationValue * animation_content.GetValueSin());
		switch (currentTabId)
		{
		case 0:
			RenderLoginPage();
			break;

		case 1:
			RenderContentPage();
			break;

		default:
			break;
		}
		ImGui::PopStyleVar();

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

	pSwap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	ID3D11Texture2D* pBackBuffer = nullptr;
	//pSwap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	pSwap_chain->GetBuffer(0U, IID_PPV_ARGS(&pBackBuffer));

	assert(pBackBuffer, "pBackBuffer is equal to 0!");

	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTarget_view);
	pBackBuffer->Release();

	D3D11_VIEWPORT vp;
	vp.Width = static_cast<float>(windowWidth);
	vp.Height = static_cast<float>(windowHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1, &vp);
}

