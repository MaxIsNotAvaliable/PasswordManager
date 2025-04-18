#include "renderGui.h"
#include "drawing.h"
#include "notify.h"

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
		if (ImGui::Button("Use empty", btnSize))
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
			static Animation animationUpdate(2.9f);
			animationUpdate.Proceed();
			if (animationUpdate.StartAfter(animationUpdate, Animation::back, Animation::forward))
			{
				animationUpdate.SetDuration(1);
			}
			if (animationUpdate.AnimationEnded(Animation::forward))
			{
				animationUpdate.SetDuration(2.9f);
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

				if (ImGui::BeginPopupContextItem(std::format("ContextMenu##file{}", i).c_str()))
				{
					if (ImGui::MenuItem("Show in explorer"))
					{
						manage_files::openDir(passwordsFolder);
					}
					if (ImGui::MenuItem("Open file"))
					{
						manage_files::openDir(passwordsFolder + "\\" + files[i]);
					}

					ImGui::EndPopup();
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
			NotifyManager::AddNotifyToQueue("Key generated!");
			for (size_t i = 0; i < maxPassKeyLen; i++)
			{
				szKeyBuffer[i] = rand() % 0x5F + 0x20;
			}
			szKeyBuffer[maxPassKeyLen - 1] = '\0';
		}

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
				selectedFileName = manage_files::FindAvailableFilename(passwordsFolder, "New file.enc");

			passwordManager.GetList().clear();
			passwordManager.SetKey(szKeyBuffer);

			passwordManager.SetFilename(selectedFileName);
			if (!isDisabled)
				passwordManager.DecryptAndOpenData();

			if (!passwordManager.CheckForStrings())
			{
				items::OpenNotify("Data corruption");
				NotifyManager::AddNotifyToQueue("Invalid key!", "Encryption");
				passwordManager.ClearData();
			}
			else
			{
				NotifyManager::AddNotifyToQueue("Loading file...", "Encryption");
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
	static int prevSelectedItem = -1;
	static int nextSelectedItem = -1;
	static int selectedItem = -1;

	static Animation animation_page(0.1f);
	animation_page.Proceed();

	if (animation_page.StartAfter(animation_page, Animation::back, Animation::forward))
		selectedItem = nextSelectedItem;


	auto SetChildPage = [&](int page)
		{
			prevSelectedItem = nextSelectedItem;
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

	int popupResult = items::ShowNotifyLm("File save dialog", "Do you want to save file?", [&]()
		{
			return items::DefaultNotifySave(szSaveFilename, sizeof(szSaveFilename));
		});

	if (popupResult == 1)
	{
		passwordManager.SetFilename(szSaveFilename);
		passwordManager.EncryptAndSaveData();
	}

	isLastSavePopupOpen = ImGui::IsPopupOpen("File save dialog");

	if (ImGui::Button("<<< Back to file list"))
	{
		if (!passwordManager.IsFileUpdated())
		{
			SetContentPageID(0);
			selectedItem = -1;
			passwordManager.ClearData();
		}
		else
		{
			items::OpenNotify("Save file and go back");
			std::string filename = passwordManager.GetFilename();
			sprintf_s(szSaveFilename, sizeof(szSaveFilename), filename.c_str());
		}
	}

	int savePopupResult = items::ShowNotifyLm("Save file and go back", "Do you want to save file?", [&]()
		{
			return items::DefaultNotifySave(szSaveFilename, sizeof(szSaveFilename));
		});
	if (savePopupResult == 1)
	{
		passwordManager.SetFilename(szSaveFilename);
		passwordManager.EncryptAndSaveData();
		SetContentPageID(0);
		selectedItem = -1;
		passwordManager.ClearData();
	}
	if (savePopupResult == 2)
	{
		SetContentPageID(0);
		selectedItem = -1;
		passwordManager.ClearData();
	}


	ImGui::BeginChild("ContentChild", ImVec2(), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
	{
		auto& passwordList = passwordManager.GetList();

		auto SwapItems = [&](int index1, int index2)
			{
				if (index1 >= passwordList.size() || index2 >= passwordList.size())
					return;

				if (nextSelectedItem == index2 || nextSelectedItem != index1)
					SetChildPage(index2);
				std::swap(passwordList[index1], passwordList[index2]);
			};

		auto MoveItemUp = [&](int i)
			{
				SwapItems(i, i - 1);
			};

		auto MoveItemDown = [&](int i)
			{
				SwapItems(i, i + 1);
			};


		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::darkerAlpha);
		ImGui::BeginChild("LeftPage", ImVec2(300, 0), 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{

			ImVec2 btnSize = tools::CalcItemSize("", ImVec2(tools::CalcWindowSpace().x, 0));
			if ((btnSize.y + style->ItemSpacing.y) * (passwordList.size() + 1) > tools::CalcWindowSpace().y)
				btnSize.x -= style->ScrollbarSize;
			ImVec2 btnPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			ImVec2 currentBtnPos = btnPos;
			ImVec2 nextBtnPos = btnPos;

			if (ImGui::Button("+ Create new +", btnSize))
			{
				Password_t pwd;
				pwd.SetTitleName("*** New ***");
				passwordList.push_back(pwd);
				SetChildPage(int(passwordList.size()) - 1);
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

					if (selectedItem == i)
					{
						currentBtnPos = btnPos;
					}
					if (nextSelectedItem == i)
					{
						nextBtnPos = btnPos;
					}


					if (ImGui::Button(std::format("{}##sbtnid{}", strlen(passwordList[i].szTitleName) == 0 ? ">> Missing name" : passwordList[i].szTitleName, i).c_str(), btnSize))
					{
						SetChildPage(nextSelectedItem == i ? -1 : i);
					}

					if (ImGui::BeginPopupContextItem(std::format("ContextMenu##{}", i).c_str()))
					{
						if (ImGui::MenuItem("Edit"))
						{
							SetChildPage(i);
						}

						if (ImGui::MenuItem("Move Up", nullptr, false, i > 0))
						{
							MoveItemUp(i);
						}

						if (ImGui::MenuItem("Move Down", nullptr, false, i < passwordList.size() - 1))
						{
							MoveItemDown(i);
						}

						//static std::vector<std::string> names = {"-/-/", "/-/-"};
						//if (ImGui::MenuItem(std::format("{}##emptybtnaboab", names[(time(0) / 2) % 2]).c_str(), nullptr, false, false))

						if (ImGui::MenuItem(std::format("---##emptybtnaboab").c_str(), nullptr, false, false))
						{
							//DuplicateItem(i);
						}

						if (ImGui::MenuItem("Delete", nullptr, false, false))
						{
							items::OpenNotify("Delete");
						}

						ImGui::EndPopup();
					}


					if (selectedItem == i)
					{
						draw::AddButtonEffectArrow(btnPos + ImVec2(20, 0) * (animation_page.GetValue() - 1), 0, btnSize);
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
			bool useAlpha = (nextSelectedItem == -1 && selectedItem != -1) || (nextSelectedItem != -1 && prevSelectedItem == -1);

			if (useAlpha)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animation_page.GetValue());
			}

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


				auto inputAnimationStyled = [&](const char* label, std::function<char* (int id, size_t& textSize)> getTextFn, ImGuiInputTextFlags extraFlags = 0)
					{
						size_t selectedItemSize, nextItemSize;
						char* currentText = getTextFn(selectedItem, selectedItemSize);
						if (animation_page.IsRunning() && nextSelectedItem != -1)
						{
							char* nextText = getTextFn(nextSelectedItem, nextItemSize);
							std::string animStr = LerpString(currentText, nextText, 1 - animation_page.GetValue());
							ImGui::InputText(label, animStr.data(), animStr.size(), ImGuiInputTextFlags_ReadOnly | extraFlags);
						}
						else
						{
							ImGui::InputText(label, currentText, selectedItemSize, extraFlags);
						}
					};

				auto inputMultilineAnimationStyled = [&](const char* label, std::function<char* (int id, size_t& textSize)> getTextFn, ImGuiInputTextFlags extraFlags = 0)
					{
						size_t selectedItemSize, nextItemSize;
						char* currentText = getTextFn(selectedItem, selectedItemSize);
						if (animation_page.IsRunning() && nextSelectedItem != -1)
						{
							char* nextText = getTextFn(nextSelectedItem, nextItemSize);
							std::string animStr = LerpString(currentText, nextText, 1 - animation_page.GetValue());
							ImGui::InputTextMultiline(label, animStr.data(), animStr.size(), ImVec2(), ImGuiInputTextFlags_ReadOnly | extraFlags);
						}
						else
						{
							ImGui::InputTextMultiline(label, currentText, selectedItemSize, ImVec2(), extraFlags);
						}
					};



				static bool visibleLogin = true, visiblePass = false;

				ImGui::Text("Title name");
				inputAnimationStyled("##Title name", [&](int id, size_t& textSize)
					{
						textSize = sizeof(passwordList[id].szTitleName);
						return passwordList[id].szTitleName;
					});

				ImGui::NewLine();

				ImGui::Text("Account login");
				inputAnimationStyled("##Account login", [&](int id, size_t& textSize) 
					{
						textSize = sizeof(passwordList[id].szLogin); 
						return passwordList[id].szLogin;
					}, visibleLogin ? 0 : ImGuiInputTextFlags_Password);

				ImGui::SameLine();
				if (ImGui::Button("Copy to clipboard##copylogin"))
				{
					clipboardxx::clipboard() << passwordList[selectedItem].szLogin;
					NotifyManager::AddNotifyToQueue("Login copied to clipboard!");
				}
				ImGui::SameLine();
				ImGui::Checkbox("<o>##visibleLogin", &visibleLogin);
				ImGui::NewLine();

				ImGui::Text("Account password");
				inputAnimationStyled("##Account password", [&](int id, size_t& textSize)
					{
						textSize = sizeof(passwordList[id].szPassword);
						return passwordList[id].szPassword;
					}, visiblePass ? 0 : ImGuiInputTextFlags_Password);

				ImGui::SameLine();
				if (ImGui::Button("Copy to clipboard##copypassword"))
				{
					NotifyManager::AddNotifyToQueue("Password copied to clipboard!");
					clipboardxx::clipboard() << passwordList[selectedItem].szPassword;
				}
				ImGui::SameLine();
				ImGui::Checkbox("<o>##visiblePass", &visiblePass);
				ImGui::NewLine();

				ImGui::Text("Additional description");
				inputMultilineAnimationStyled("##Additional description", [&](int id, size_t& textSize) 
					{
						textSize = sizeof(passwordList[id].szDescription);
						return passwordList[id].szDescription;
					});

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
					selectedItem = -1;
				}
			}

			if (useAlpha)
			{
				ImGui::PopStyleVar();
			}

		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	ImGui::EndChild();
}

void RenderNotifies(const ImVec2& windowSize)
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoResize |
		//ImGuiWindowFlags_NoFocusOnAppearing |
		//ImGuiWindowFlags_front
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_AlwaysAutoResize;

	auto CalcItemsHeight = [](int itemAmount)
		{
			return tools::CalcItemSize("").y * itemAmount + ImGui::GetStyle().ItemSpacing.y * (itemAmount - 1);
		};

	ImVec2 winSize = ImVec2(200, CalcItemsHeight(2) + style->WindowPadding.y);
	float margin = 10;

	std::vector<Notify> notifyList = NotifyManager::GetList();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2());

	for (size_t i = 0; i < notifyList.size(); i++)
	{
		std::string title = std::format("{} {}", notifyList[i].GetTitle(), i);

		float animValue = notifyList.back().animation.GetValueInOutSin();
		animValue = sqrtf(animValue);

		ImVec2 startPos = ImVec2(
			windowSize.x,
			windowSize.y - margin - Animation::lerp(0.01f, winSize.y, animValue) - (winSize.y + margin) * (notifyList.size() - i - 1)
		);

		ImVec2 endPos = ImVec2(
			windowSize.x - margin - winSize.x,
			windowSize.y - margin - Animation::lerp(0.01f, winSize.y, animValue) - (winSize.y + margin) * (notifyList.size() - i - 1)
		);
		ImVec2 resPos = ImLerp(startPos, endPos, notifyList[i].animation.GetValue());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

		ImGui::SetNextWindowPos(resPos);
		ImGui::SetNextWindowSize(winSize);

		ImGui::SetNextWindowBgAlpha(0.4f);

		ImGui::Begin(title.c_str(), 0, windowFlags);
		{
			float itemHeight = tools::CalcItemSize("Notify").y;
			//draw::Pie(ImVec2(resPos.x + itemHeight / 2 + style->ItemSpacing.x, resPos.y + itemHeight / 2 + style->ItemSpacing.y), notifyList[i].GetLifeValue(NotifyManager::maxLifeTime), itemHeight / 4);
			draw::PieDiagram(ImVec2(resPos.x + itemHeight / 2 + style->ItemSpacing.x, resPos.y + itemHeight / 2 + style->ItemSpacing.y), notifyList[i].GetLifeValue(NotifyManager::maxLifeTime), itemHeight / 4, itemHeight / 5);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + itemHeight / 2 + style->ItemSpacing.x * 2);
			ImGui::TextColored(colors::active, notifyList[i].GetTitle().c_str());
			ImGui::Text(notifyList[i].GetText().c_str());
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
	}

	ImGui::PopStyleVar();
}

void RenderGUI::RenderBody(const HWND& window)
{
	animation_show.Proceed();
	animation_maximize.Proceed();
	animation_content.Proceed();

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

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animationValue);
	RenderNotifies(windowSize);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowBgAlpha(alpha);


	const float fontSize = ImGui::GetFontSize();

	constexpr ImGuiWindowFlags guiFlags =
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;

	ImGui::BringWindowToDisplayBack(ImGui::FindWindowByName("Password Manager"));

	if (ImGui::Begin("Password Manager", &running, guiFlags))
	{
		// title bar
		{
			ImGui::GetWindowDrawList()->AddRectFilled(windowPos, windowPos + ImVec2(windowSize.x, RenderGUI::titleBarHeight), ImGui::GetColorU32(colors::background * colors::darker * colors::darker), style->WindowRounding);

			const ImVec2 titleBtnSize = ImVec2(30, 30);
			const ImVec2 injectBtnSz = ImVec2(170, 48);

			const float titleBtnWidth = titleBtnSize.x;
			const float titleBtnPd = titleBtnWidth / 4.5f;

			ImGui::SetCursorPosX(10);
			ImGui::BeginDisabled();
			std::string fileNameFormatted = passwordManager.GetFilename().length() > 0 ? " - " + passwordManager.GetFilename() : "";
			std::string fileUpdated = passwordManager.GetFilename().length() > 0 && passwordManager.IsFileUpdated() ? "*" : "";
			ImGui::Button(std::format("Password manager{}##disabled-button-as-title", fileNameFormatted + fileUpdated).c_str());
			ImGui::EndDisabled();
			ImGui::SameLine();

#if ALLOW_WINDOW_RESIZE
			constexpr float titleBarBtnCount = 3;
#else
			constexpr float titleBarBtnCount = 2;
#endif

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

			static char szSaveFilename[MAX_PATH] = { '\0' };
			CrossPos = tools::LocalToGlobalPos(ImGui::GetCursorPos());
			if (items::ShadowButton("##closeBtn", titleBtnSize))
			{
				if (passwordManager.GetList().size() == 0 || !passwordManager.IsFileUpdated())
				{
					animation_show.Start(Animation::back);
					running = false;
				}
				else
				{
					items::OpenNotify("File save dialog and exit");
					std::string filename = passwordManager.GetFilename();
					sprintf_s(szSaveFilename, sizeof(szSaveFilename), filename.c_str());
				}
			}

			int popupResult = items::ShowNotifyLm("File save dialog and exit", "Do you want to save file?", [&]() {
				return items::DefaultNotifySave(szSaveFilename, sizeof(szSaveFilename));
				});

			if (popupResult == 1)
			{
				passwordManager.SetFilename(szSaveFilename);
				passwordManager.EncryptAndSaveData();
				RenderGUI::Kill();
			}
			if (popupResult == 2)
			{
				RenderGUI::Kill();
			}

			draw::Underline(ImVec2(HidePos.x + titleBtnWidth / 2, HidePos.y + titleBtnWidth / 2), titleBtnPd, colors::text);
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

void RenderGUI::Kill()
{
	animation_show.Start(Animation::back);
	running = false;
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

