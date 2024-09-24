#include "renderGui.h"
//#include <render/drawing.h>
#include "drawing.h"

#include "../encryption/encrypt.h"
#include "../encryption/PasswordData.h"

#include <clipboard/clipboardxx.hpp>

#include <files_helper/files.h>

#include <byte_data/fonts.h>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

static int currentTabId = 0;
static int nextTabId = currentTabId;

static Animation animation_content = Animation(Animation::forward, 0.25f);
static CPasswordDataManager passwordManager;
static size_t passwordManagerHash = 0;

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

void SetContentPageID(int page)
{
	nextTabId = page;
	animation_content.Start(Animation::back);
}

int FilterInputTextOnlyPassSymbols(ImGuiInputTextCallbackData* data)
{
	ImWchar c = data->EventChar;
	auto isCharAllowed = [&](char c, const std::string& allowedChars)
		{
			for (size_t i = 0; i < allowedChars.size(); i++)
			{
				if (allowedChars[i] == c)
					return true;
			}
			return false;
		};

	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') ||
		(isCharAllowed(c, "~!@#$%^&*()_+`-=/|\\,.<>?;:\"\'[]{}")))
	{
		return 0;
	}

	return 1;
};


void RenderLoginPage()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec2 wndSize = ImGui::GetWindowSize();
	ImVec2 wndPos = ImGui::GetWindowPos();
	ImVec2 loginChildSize = ImVec2(340, 230);
	ImVec2 profilesChildSize = ImVec2(240, 430);
	ImVec2 deltaPos = wndSize - (ImVec2(loginChildSize.x, 0) + profilesChildSize) + style->WindowPadding;

	static int selectedFile = -1;
	static std::string selectedFileName;
	static std::string passwordsFolder = manage_files::GetFolder() + "\\passwords";
	manage_files::createDirectory(passwordsFolder);
	static std::vector<std::string> files = manage_files::GetFilesInDirectory(passwordsFolder, true);

	ImGui::SetNextWindowPos(wndPos + ImVec2(deltaPos.x / 2, deltaPos.y / 2));
	ImGui::BeginChild("SelectProfileChild", profilesChildSize, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		ImVec2 center = tools::LocalToGlobalPos(ImGui::GetCursorPos());
		ImVec2 btnSize = tools::CalcItemSize("", ImVec2(tools::CalcWindowSpace().x, 0));

		ImVec2 btnPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
		if (ImGui::Button("Create new", btnSize))
		{
			selectedFile = -1;
		}
		if (selectedFile == -1)
		{
			draw::AddPrevButtonEffect(btnSize);
			draw::AddButtonEffectArrow(btnPos, 0, btnSize);
		}

		if (!manage_files::folderExists(passwordsFolder) && ImGui::Button("+ create folder"))
		{
			manage_files::createDirectory(passwordsFolder);
		}
		else
		{
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

			ImGui::Text("Update in %.1f sec...", animationUpdate.GetState() != Animation::back ? 0 : animationUpdate.GetValue() * animationUpdate.GetDuration());
			ImVec2 linePos = tools::LocalToGlobalPos(ImGui::GetCursorPos() + ImVec2(0, 0));
			ImGui::GetWindowDrawList()->AddRectFilled(linePos, linePos + ImVec2(tools::CalcWindowSpace().x * animationUpdate.GetValueSin(), 2), ImGui::GetColorU32(colors::active), style->WindowRounding);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style->FramePadding.y);
			bool needUpdate = false;

			for (size_t i = 0; i < files.size(); i++)
			{
				ImVec2 pos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
				if (ImGui::Button(files[i].c_str(), btnSize))
				{
					selectedFile = i;
					selectedFileName = files[i];
				}
				if (selectedFile == i)
				{
					draw::AddButtonEffectArrow(pos, 0, btnSize);
				}
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
		ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue;
		if (!visible) inputTextFlags |= ImGuiInputTextFlags_Password;

		constexpr int maxPassKeyLen = 0x21;
		static char szKeyBuffer[maxPassKeyLen] = { '\0' };
		static char szFolderBuffer[maxPassKeyLen] = { '\0' };
		bool isPressedEnter = false;
		if (ImGui::InputText("##input_encryption_key", szKeyBuffer, sizeof(szKeyBuffer) / sizeof(*szKeyBuffer), inputTextFlags, FilterInputTextOnlyPassSymbols))
		{
			isPressedEnter = true;
		}
		ImGui::SameLine();
		ImGui::Checkbox("<o>##visible", &visible);

		bool isGoodLen = strlen(szKeyBuffer) >= 8;
		ImGui::Text("[%s] | [%i / %i]", isGoodLen ? "good" : "weak", int(strlen(szKeyBuffer)), maxPassKeyLen - 1);

		ImGui::NewLine();
		bool isDisabled = selectedFile == -1;

		if (!isDisabled)
		{
			ImGui::BeginDisabled();
			ImGui::Button("Generate key");
			ImGui::EndDisabled();
		}
		else if (ImGui::Button("Generate key"))
		{
			for (size_t i = 0; i < maxPassKeyLen; i++)
			{
				szKeyBuffer[i] = rand() % 0x5F + 0x20;
			}
			szKeyBuffer[maxPassKeyLen - 1] = '\0';
		}

		ImGui::SameLine();



		//if (isDisabled)
		//{
		//	ImGui::BeginDisabled();
		//	ImGui::Button("Key lost");
		//	ImGui::EndDisabled();
		//}
		//else if (ImGui::Button("Key lost")) { }

		ImGui::SameLine();

		if (!isGoodLen)
		{
			ImGui::BeginDisabled();
			ImGui::Button("Continue");
			ImGui::EndDisabled();
		}
		else if (ImGui::Button("Continue") || isPressedEnter)
		{
			if (isDisabled)
				selectedFileName = "New file.enc";

			passwordManager.SetKey(szKeyBuffer);

			passwordManager.SetFilename(selectedFileName);
			if (!isDisabled)
				passwordManager.DecryptAndOpenData();
			passwordManagerHash = std::hash<CPasswordDataManager>{}(passwordManager);
			if (!passwordManager.CheckForStrings())
			{
				items::OpenNotify("Data corruption");
			}
			else
			{
				SetContentPageID(1);
				ZeroMemory(szKeyBuffer, sizeof(szKeyBuffer));
			}
		}
		items::ShowNotify("Data corruption", "Data corrupted or invalid key was used to this file!");

	}
	ImGui::EndChild();
}

void RenderContentPage()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	static char szSaveFilename[MAX_PATH] = { '\0' };
	static int nextSelectedItem = -1;
	static int selectedItem = -1;

	static Animation animation_page(0.1f);
	animation_page.Proceed();

	if (animation_page.StartAfter(animation_page, Animation::back, Animation::forward))
		selectedItem = nextSelectedItem;


	auto SetChildPage = [&](int page)
		{
			nextSelectedItem = page;
			animation_page.Start(Animation::back);
		};

	static bool isLastSavePopupOpen = false;
	bool isGoingToSave = !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId) && ImGui::GetKeyData(ImGuiKey_S)->DownDuration > 0 && ImGui::GetKeyData(ImGuiKey_LeftCtrl)->DownDuration > 0;
	if (isGoingToSave && !ImGui::IsPopupOpen("File save dialog"))
	{
		items::OpenNotify("File save dialog");
		std::string filename = passwordManager.GetFilename();
		sprintf_s(szSaveFilename, sizeof(szSaveFilename), filename.c_str());
	}

	std::function<bool()> fnDesign = [&]()
		{
			ImGui::Text("Input filename");

			ImGui::InputText("##Input filename", szSaveFilename, sizeof(szSaveFilename), ImGuiInputTextFlags_AutoSelectAll);

			if (ImGui::Button("Cancel##cancelbtnsavepopup") || ImGui::GetKeyData(ImGuiKey_Escape)->Down)
			{
				ImGui::CloseCurrentPopup();
				return false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				ImGui::CloseCurrentPopup();
				return true;
			}
			return false;
		};

	if (items::ShowNotifyLm("File save dialog", "Do you want to save file?", fnDesign))
	{
		passwordManager.SetFilename(szSaveFilename);
		passwordManager.EncryptAndSaveData();
	}

	isLastSavePopupOpen = ImGui::IsPopupOpen("File save dialog");

	if (ImGui::Button("<<< Back to file list"))
	{
		SetContentPageID(0);
		selectedItem = -1;
	}

	ImGui::BeginChild("ContentChild", ImVec2(), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		auto& passwordList = passwordManager.GetList();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::darkerAlpha);
		ImGui::BeginChild("LeftPage", ImVec2(300, 0), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{

			ImVec2 btnSize = tools::CalcItemSize("", ImVec2(tools::CalcWindowSpace().x, 0));
			if ((btnSize.y + style->ItemSpacing.y) * (passwordList.size() + 1) > tools::CalcWindowSpace().y)
				btnSize.x -= style->ScrollbarSize;
			ImVec2 btnPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());

			if (ImGui::Button("+ Create new +", btnSize))
			{
				Password_t pwd;
				pwd.SetTitleName("New");
				passwordList.push_back(pwd);
			}
			if (ImGui::IsItemHovered())
			{
				draw::AddPrevButtonEffect(btnSize);
			}
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::transparent);
			ImGui::BeginChild("innerListChild");
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30, 0));
				for (size_t i = 0; i < passwordList.size(); i++)
				{
					btnPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
					char btnNameCpy[sizeof(passwordList[i].szTitleName)] = { '\0' };
					memcpy_s(btnNameCpy, sizeof(btnNameCpy) - 1, passwordList[i].szTitleName, sizeof(btnNameCpy) - 1);
					char btnName[sizeof(passwordList[i].szTitleName) * 3] = { '\0' };
					sprintf_s(btnName, "%s##sbtnid%i", strlen(btnNameCpy) == 0 ? ">> Missing name" : btnNameCpy, int(i));
					if (ImGui::Button(btnName, btnSize))
					{
						SetChildPage(nextSelectedItem == i ? -1 : i);
						//selectedItem = selectedItem == i ? -1 : i;
					}
					if (selectedItem == i)
					{
						draw::AddButtonEffectArrow(btnPos, 0, btnSize);
					}
				}
				ImGui::PopStyleVar(2);
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("RightPage", ImVec2(0, 0), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			if (selectedItem >= 0)
			{
				auto pushInputStyle = [&]()
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImLerp(colors::transparent, colors::text, animation_page.GetValueSin()));
					};

				auto popInputStyle = [&]()
					{
						ImGui::PopStyleColor();
					};
				
				static bool visibleLogin, visiblePass = false;

				ImGui::Text("Title name");
				
				pushInputStyle();
				ImGui::InputText("##Title name", passwordList[selectedItem].szTitleName, sizeof(passwordList[selectedItem].szTitleName));
				popInputStyle();
				
				ImGui::NewLine();

				ImGui::Text("Account login");
				pushInputStyle();
				ImGui::InputText("##Account login", passwordList[selectedItem].szLogin, sizeof(passwordList[selectedItem].szLogin), visibleLogin ? 0 : ImGuiInputTextFlags_Password);
				popInputStyle();
				ImGui::SameLine();
				if (ImGui::Button("Copy to clipboard##copylogin"))
				{
					clipboardxx::clipboard() << passwordList[selectedItem].szLogin;
				}
				ImGui::SameLine();
				ImGui::Checkbox("<o>##visibleLogin", &visibleLogin);
				ImGui::NewLine();

				ImGui::Text("Account password");
				pushInputStyle();
				ImGui::InputText("##Account password", passwordList[selectedItem].szPassword, sizeof(passwordList[selectedItem].szPassword), visiblePass ? 0 : ImGuiInputTextFlags_Password);
				popInputStyle();
				ImGui::SameLine();
				if (ImGui::Button("Copy to clipboard##copypassword"))
				{
					clipboardxx::clipboard() << passwordList[selectedItem].szPassword;
				}
				ImGui::SameLine();
				ImGui::Checkbox("<o>##visiblePass", &visiblePass);
				ImGui::NewLine();

				ImGui::Text("Additional desctiption");
				pushInputStyle();
				ImGui::InputTextMultiline("##Additional description", passwordList[selectedItem].szDescription, sizeof(passwordList[selectedItem].szDescription));
				popInputStyle();

				ImGui::SetCursorPosY(ImGui::GetWindowHeight() - tools::CalcItemSize("").y - style->WindowPadding.y);
				if (ImGui::Button("Delete"))
				{
					items::OpenNotify("Delete");
				}
				

				std::function<bool()> fnDeletePopup = [&]()
					{
						ImGui::Text("Delete");

						if (ImGui::Button("Cancel##cancelbtndelpopup") || ImGui::GetKeyData(ImGuiKey_Escape)->Down)
						{
							ImGui::CloseCurrentPopup();
							return false;
						}
						ImGui::SameLine();
						if (ImGui::Button("Delete this row"))
						{
							ImGui::CloseCurrentPopup();
							return true;
						}
						return false;
					};


				if (items::ShowNotifyLm("Delete", "Do you want to delete account data? \nThere is no way back", fnDeletePopup))
				{
					passwordList.erase(passwordList.begin() + selectedItem);
					//selectedItem = min(selectedItem, int(passwordList.size()) - 1);
					selectedItem = -1;
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

	ImVec2 hWndPosOverride = ImLerp(ImVec2(RenderGUI::windowPosX, RenderGUI::windowPosY), ImVec2(0, 0), maximizeValue);
	ImVec2 hWndSizeOverride = ImLerp(ImVec2(RenderGUI::GetFullWidth(), RenderGUI::GetFullHeight()), ImVec2(1920, 1080), maximizeValue);

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
			//ImGui::Text("Password manager");
			ImGui::BeginDisabled();
			ImGui::Button("Password manager##Password manager btn as title bar??? xDDDDDDDD");
			ImGui::EndDisabled();
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

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Latin
		0x0400, 0x044F, // Cyrillic
		0,
	};


	fonts::regular_14 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_regular), 14.f, &fontConfig, ranges);
	fonts::bold_16 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 16.f, &fontConfig, ranges);
	fonts::bold_20 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 20.f, &fontConfig, ranges);
	fonts::bold_28 = io->Fonts->AddFontFromMemoryTTF(ttf_bytes_inter_bold, sizeof(ttf_bytes_inter_bold), 28.f, &fontConfig, ranges);
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

	assert(pBackBuffer);

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

