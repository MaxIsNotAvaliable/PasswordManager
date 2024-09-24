#pragma once
#include <base.h>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <future>

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
static inline ImVec2 operator*(const ImVec2& lhs, const float& rhs)
{
	return ImVec2(lhs.x * rhs, lhs.y * rhs);
}
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
}
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)
{
	return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}


struct fonts
{
	static inline ImFont* regular_14 = nullptr;
	static inline ImFont* bold_16 = nullptr;
	static inline ImFont* bold_20 = nullptr;
	static inline ImFont* bold_28 = nullptr;
};

struct colors
{
	static inline const ImVec4 darker = ImVec4(0.9f, 0.9f, 0.9f, 1);
	static inline const ImVec4 active = ImVec4(0.41f, 0.6f, 0.9f, 1);
	static inline const ImVec4 activeDark = ImVec4(0.1f, 0.17f, 0.27f, 1);
	static inline const ImVec4 white = ImVec4(1, 1, 1, 1);
	static inline const ImVec4 black = ImVec4(0, 0, 0, 1);
	static inline const ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	static inline const ImVec4 darkerAlpha = ImVec4(0.0f, 0.0f, 0.0f, 0.1f);

	static inline ImVec4 background = ImVec4(0.13f, 0.13f, 0.13f, 1);
	static inline ImVec4 backgroundPopUp = ImVec4(0.19f, 0.19f, 0.19f, 1);

	//static inline ImVec4 text = ImVec4(0.83f, 0.83f, 0.83f, 1);
	static inline ImVec4 text = ImVec4(1, 1, 1, 1);

	static inline ImVec4 frame = ImVec4(0.16f, 0.16f, 0.16f, 1);
	static inline ImVec4 frameHovered = ImVec4(0.19f, 0.19f, 0.19f, 1);
	static inline ImVec4 frameActive = ImVec4(0.22f, 0.22f, 0.22f, 1);
};

namespace tools
{
	static ImVec2 CalcItemSize(const char* text, ImVec2 size_arg = ImVec2())
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec2 textSz = ImGui::CalcTextSize(text);
		return ImGui::CalcItemSize(size_arg, textSz.x + style->FramePadding.x * 2.0f, textSz.y + style->FramePadding.y * 2.0f);
	}

	static ImVec2 CalcWindowSpace()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		return ImGui::GetContentRegionAvail() /*- style->WindowPadding * 2*/;
	}


	static ImVec2 CalcChildSize(ImVec2 size_arg = ImVec2())
	{
		const ImVec2 content_avail = ImGui::GetContentRegionAvail();
		ImVec2 size = ImFloor(size_arg);
		const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
		if (size.x <= 0.0f)
			size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
		if (size.y <= 0.0f)
			size.y = ImMax(content_avail.y + size.y, 4.0f);
		return size;
	}

	// use for drawlist
	static ImVec2 LocalToGlobalPos(ImVec2 localPos, bool countScroll = true)
	{
		ImVec2 windowPos = ImGui::GetWindowPos();
		if (countScroll)
			localPos = localPos - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
		return ImVec2(localPos.x + windowPos.x, localPos.y + windowPos.y);
	}
}

#define LINE_WIDTH 2.0f

namespace draw
{
	static void AddPrevButtonEffect(ImVec2 buttonSize = ImVec2(0, 0))
	{
		buttonSize = tools::CalcItemSize("", buttonSize);

		ImVec2 beginPos = tools::LocalToGlobalPos(ImGui::GetCursorStartPos());
		ImGui::GetCursorPos();

		ImVec2 cornersSizeH = ImVec2(5 * ImGui::IsItemHovered(), 0);
		ImVec2 cornersSizeV = ImVec2(0, 5 * ImGui::IsItemHovered());

		ImGui::GetWindowDrawList()->AddTriangleFilled(beginPos, beginPos + cornersSizeH, beginPos + cornersSizeV, ImGui::GetColorU32(colors::active));
		ImGui::GetWindowDrawList()->AddTriangleFilled(buttonSize + beginPos, buttonSize + beginPos - cornersSizeH, buttonSize + beginPos - cornersSizeV, ImGui::GetColorU32(colors::active));
	}

	static void AddButtonEffect(ImVec2 buttonPos, float value, ImVec2 buttonSize = ImVec2(0, 0))
	{
		buttonSize = tools::CalcItemSize("", buttonSize);

		ImGui::GetCursorPos();

		ImVec2 cornersSizeH = ImVec2(5 * value, 0);
		ImVec2 cornersSizeV = ImVec2(0, 5 * value);

		ImGui::GetWindowDrawList()->AddTriangleFilled(buttonPos, buttonPos + cornersSizeH, buttonPos + cornersSizeV, ImGui::GetColorU32(colors::active));
		ImGui::GetWindowDrawList()->AddTriangleFilled(buttonSize + buttonPos, buttonSize + buttonPos - cornersSizeH, buttonSize + buttonPos - cornersSizeV, ImGui::GetColorU32(colors::active));
	}

	static void AddButtonEffectArrow(ImVec2 buttonPos, float value, ImVec2 buttonSize = ImVec2(0, 0))
	{
		buttonSize = tools::CalcItemSize(0, buttonSize);

		ImGui::GetCursorPos();

		buttonPos = ImVec2(buttonPos.x + buttonSize.y / 2, buttonPos.y + buttonSize.y / 2);

		Vec2 angleV0 = Vec2(5, 0);
		ImVec2 angleV1 = *(ImVec2*)(&angleV0.Rotate(value));
		ImVec2 angleV2 = *(ImVec2*)(&angleV0.Rotate(120));
		ImVec2 angleV3 = *(ImVec2*)(&angleV0.Rotate(120));

		ImGui::GetWindowDrawList()->AddTriangleFilled(buttonPos + angleV1, buttonPos + angleV2, buttonPos + angleV3, ImGui::GetColorU32(colors::active));
	}

	static void Cross(ImVec2 center, float radius, ImVec4 color = colors::white)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImU32 col = ImGui::GetColorU32(color);

		ImRect rect = ImRect{ ImVec2(center.x - radius, center.y - radius) , ImVec2(center.x + radius, center.y + radius) };
		window->DrawList->AddLine(rect.Min, rect.Max, col, LINE_WIDTH);
		window->DrawList->AddLine(ImVec2(rect.Min.x, rect.Max.y), ImVec2(rect.Max.x, rect.Min.y), col, LINE_WIDTH);
	}

	static void Underline(const ImVec2& center, const float& width, const ImVec4& color = colors::white)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImVec2 size = ImVec2(width, 0);

		ImU32 col = ImGui::GetColorU32(color);
		window->DrawList->AddLine(center - size, center + size, col, LINE_WIDTH);
	}

	static void CornersMarks(ImVec2 center, float space, float direction = -1, ImVec4 color = colors::white)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		direction = clampf(-1, 1, direction);
		//space *= direction;
		float spaceDir = space * (direction * 0.7f + 0.3f);

		ImU32 col = ImGui::GetColorU32(color);
		ImVec2 p1 = ImVec2(center.x - spaceDir, center.y + spaceDir);
		ImVec2 p2 = ImVec2(center.x + spaceDir, center.y - spaceDir);

		//ImRect rect = ImRect{ ImVec2(center.x - space, center.y - space) , ImVec2(center.x + space, center.y + space) };

		ImVec2 points1[3] = { p1 + ImVec2(space, 0), p1, p1 - ImVec2(0, space) };
		ImVec2 points2[3] = { p2 - ImVec2(space, 0), p2, p2 + ImVec2(0, space) };

		window->DrawList->AddPolyline(points1, 3, col, NULL, LINE_WIDTH);
		window->DrawList->AddPolyline(points2, 3, col, NULL, LINE_WIDTH);
		//window->DrawList->AddLine(p1, p1 - ImVec2(0, space), col, LINE_WIDTH);
		//window->DrawList->AddLine(p1, p1 + ImVec2(space, 0), col, LINE_WIDTH);
		//window->DrawList->AddLine(p2, p2 + ImVec2(0, space), col, LINE_WIDTH);
		//window->DrawList->AddLine(p2, p2 - ImVec2(space, 0), col, LINE_WIDTH);
	}


	static void Pie(ImVec2 center, float value, float radius = 50, const char* extraText = 0)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		static std::vector<ImVec2> dots;
		static bool calculated = false;
		static const float PI = asinf(1) * 2;
		if (!calculated)
		{
			calculated = true;
			for (float t = -1; t < 1; t += 0.01f)
			{
				dots.insert(dots.begin(), ImVec2(sinf(t * PI), cosf(t * PI)));
			}
		}
		static std::vector<ImVec2> points = {  };
		for (size_t i = 0; i < dots.size(); i++)
		{
			points.insert(points.end(), ImVec2(dots[i].x * radius + center.x, dots[i].y * radius + center.y));
		}
		window->DrawList->AddPolyline(points.data(), (int)(dots.size() * value), ImGui::GetColorU32(colors::white), 0, 20);

		if (extraText)
		{
			char arr[32];
			sprintf_s(arr, "%s %d", extraText, int(value * 100));
			ImVec2 textSz = ImGui::CalcTextSize(arr);
			ImVec2 textPs = ImVec2(center.x - textSz.x / 2, center.y - textSz.y / 2);
			window->DrawList->AddText(textPs, ImGui::GetColorU32(colors::white), arr);
		}
	}

	static bool GradientBox(const char* label, const ImVec2 start_pos, const ImVec2 end_pos, ImVec4 bg_color_1, ImVec4 bg_color_2)
	{
		ImU32 bg_color32_1 = ImGui::GetColorU32(bg_color_1);
		ImU32 bg_color32_2 = ImGui::GetColorU32(bg_color_2);
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

		const ImRect bb(start_pos, end_pos);

		ImGuiButtonFlags flags = ImGuiButtonFlags_None;
		if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const bool is_gradient = bg_color32_1 != bg_color32_2;
		//if (held || hovered)
		//{
		//	// Modify colors (ultimately this can be prebaked in the style)
		//	float h_increase = (held && hovered) ? 0.02f : 0.02f;
		//	float v_increase = (held && hovered) ? 0.20f : 0.07f;

		//	ImVec4 bg1f = ImGui::ColorConvertU32ToFloat4(bg_color32_1);
		//	ImGui::ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
		//	bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
		//	bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
		//	ImGui::ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
		//	bg_color32_1 = ImGui::GetColorU32(bg1f);
		//	if (is_gradient)
		//	{
		//		ImVec4 bg2f = ImGui::ColorConvertU32ToFloat4(bg_color32_2);
		//		ImGui::ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
		//		bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
		//		bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
		//		ImGui::ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
		//		bg_color32_2 = ImGui::GetColorU32(bg2f);
		//	}
		//	else
		//	{
		//		bg_color32_2 = bg_color32_1;
		//	}
		//}
		//ImGui::RenderNavHighlight(bb, id);
		// ImRotate() method
		int vert_start_idx = window->DrawList->VtxBuffer.Size;
		window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color32_1, g.Style.FrameRounding);
		int vert_end_idx = window->DrawList->VtxBuffer.Size;
		if (is_gradient)
			ImGui::ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color32_1, bg_color32_2);
		/*if (g.Style.FrameBorderSize > 0.0f)
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);*/

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return pressed;
	}

	static void SemiCircle(ImDrawList* drawList, const ImVec2& center, float radius, ImU32 col, float p_min = 0.f, float p_max = 1.0f, int num_segments = 64)
	{
		if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
			return;

		float p_omax = p_max;
		float p_omin = p_min;
		if (p_max < p_min)
		{
			p_max = p_omin;
			p_min = p_omax;
		}

		if (num_segments <= 0)
		{
			// Use arc with automatic segment count
			drawList->_PathArcToFastEx(center, radius, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
			drawList->_Path.Size--;
		}
		else
		{
			// Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
			num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

			// Because we are filling a closed shape we remove 1 from the count of segments/points
			const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
			drawList->PathLineTo(center);
			drawList->PathArcTo(center, radius, a_max * p_min, a_max * p_max, num_segments - 1);
			drawList->PathLineTo(center);
		}

		drawList->PathFillConvex(col);
	}

	void SemiCircleShadow(ImDrawList* drawList, const ImVec2& obj_center, float obj_radius, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset = ImVec2(0, 0), float p_min = 0.f, float p_max = 1.0f, ImDrawFlags flags = 0, int num_segments = 64)
	{
		float p_omax = p_max;
		float p_omin = p_min;
		if (p_max < p_min)
		{
			p_max = p_omin;
			p_min = p_omax;
		}

		// Obtain segment count
		if (num_segments <= 0)
		{
			// Automatic segment count
			const int radius_idx = (int)obj_radius - 1;
			if (radius_idx < IM_ARRAYSIZE(drawList->_Data->CircleSegmentCounts))
				num_segments = drawList->_Data->CircleSegmentCounts[radius_idx]; // Use cached value
			else
				num_segments = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(obj_radius, drawList->_Data->CircleSegmentMaxError);
		}
		else
		{
			// Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
			num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);
		}

		// Generate a path describing the inner circle and copy it to our buffer
		IM_ASSERT(drawList->_Path.Size == 0);
		const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
		drawList->PathLineTo(obj_center);
		if (num_segments == 12)
			drawList->PathArcToFast(obj_center, obj_radius, 0, 12 - 1);
		else
			drawList->PathArcTo(obj_center, obj_radius, a_max * p_min, a_max * p_max, num_segments - 1);
		drawList->PathLineTo(obj_center);

		// Draw the shadow using the convex shape code
		drawList->AddShadowConvexPoly(drawList->_Path.Data, drawList->_Path.Size, shadow_col, shadow_thickness, shadow_offset, flags);
		drawList->_Path.Size = 0;
	}

	static void Clock(ImVec2 center, float timeprcnt, float radius, float outline_width, float outline_radius_offset = 0, int num_segments = 64)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = tools::LocalToGlobalPos(center);
		drawList->AddCircle(pos, radius + outline_radius_offset, ImGui::GetColorU32(colors::active), num_segments, outline_width * 2);
		draw::SemiCircleShadow(drawList, pos, radius, ImGui::GetColorU32(colors::black), 10, ImVec2(), -0.25f, timeprcnt - 0.25f, 0, num_segments);
		draw::SemiCircle(drawList, pos, radius, ImGui::GetColorU32(colors::white), -0.25f, timeprcnt - 0.25f, num_segments);
	}
}

namespace items
{
	bool ShadowButton(const char* label, ImVec2 size = ImVec2(), ImVec4 textColor = colors::text)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		ImGuiStyle* style = &ImGui::GetStyle();
		const ImVec2 calcSize = tools::CalcItemSize(label);

		if (size.x <= 0)
			size.x = calcSize.x;
		if (size.y <= 0)
			size.y = calcSize.y;

		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 max = min + size;

		const bool pressed = ImGui::Button(label, size);
		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();

		//ImU32 colorU32 = ImGui::GetColorU32(active ? style->Colors[ImGuiCol_ButtonActive] : (hovered ? style->Colors[ImGuiCol_ButtonHovered] : style->Colors[ImGuiCol_Button]));
		ImVec4 color = colors::black;
		color.w = 0.25f;
		ImU32 colorU32 = ImGui::GetColorU32(color);

		//ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		drawList->AddShadowRect(min, max, colorU32, 25, ImVec2());

		ImGui::PopStyleColor();
		return pressed;
	}

	void OpenNotify(const char* title)
	{
		ImGui::OpenPopup(title);
	}

	bool ShowNotify(const char* title, const char* label, bool* outResult = nullptr)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
		bool pressed = false;

		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.1f);
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::PushFont(fonts::bold_16);
			ImGui::TextColored(colors::white, title);
			ImGui::PopFont();

			ImGui::Text(label);
			ImGui::Separator();

			// --
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				pressed = true;
				if (outResult) *outResult = true;
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				pressed = true;
				if (outResult) *outResult = false;
			}
			// --
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		return pressed;
	}

	bool ShowNotifyLm(const char* title, const char* label, const std::function<bool()>& fnDesign)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
		bool pressed = false;

		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.1f);
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::PushFont(fonts::bold_16);
			ImGui::TextColored(colors::white, title);
			ImGui::PopFont();

			ImGui::Text(label);
			ImGui::Separator();

			pressed = fnDesign();

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		return pressed;
	}

	static void ShowFileMenu(bool* pOpen)
	{
		ImGui::MenuItem("File", NULL, pOpen, false);
		if (ImGui::MenuItem("New")) {}
		if (ImGui::MenuItem("Open", "Ctrl+O")) {}
		if (ImGui::BeginMenu("Open Recent"))
		{
			ImGui::MenuItem("fish_hat.c");
			ImGui::MenuItem("fish_hat.inl");
			ImGui::MenuItem("fish_hat.h");
			if (ImGui::BeginMenu("More.."))
			{
				ImGui::MenuItem("Hello");
				ImGui::MenuItem("Sailor");
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save", "Ctrl+S")) {}
		if (ImGui::MenuItem("Save As..")) {}

		ImGui::Separator();
		if (ImGui::BeginMenu("Options"))
		{
			static bool enabled = true;
			ImGui::MenuItem("Enabled", "", &enabled);
			ImGui::BeginChild("child", ImVec2(0, 60), true);
			for (int i = 0; i < 10; i++)
				ImGui::Text("Scrolling Text %d", i);
			ImGui::EndChild();
			static float f = 0.5f;
			static int n = 0;
			ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
			ImGui::InputFloat("Input", &f, 0.1f);
			ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Colors"))
		{
			float sz = ImGui::GetTextLineHeight();
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
				ImGui::Dummy(ImVec2(sz, sz));
				ImGui::SameLine();
				ImGui::MenuItem(name);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options")) // <-- Append!
		{
			static bool b = true;
			ImGui::Checkbox("SomeOption", &b);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Disabled", false)) // Disabled
		{
			IM_ASSERT(0);
		}
		if (ImGui::MenuItem("Checked", NULL, true)) {}
		if (ImGui::MenuItem("Quit", "Alt+F4")) {}
	}

	static void ShowExampleMenuFile(bool* pOpen)
	{
		ImGui::MenuItem("(demo menu)", NULL, pOpen, false);
		if (ImGui::MenuItem("New")) {}
		if (ImGui::MenuItem("Open", "Ctrl+O")) {}
		if (ImGui::BeginMenu("Open Recent"))
		{
			ImGui::MenuItem("fish_hat.c");
			ImGui::MenuItem("fish_hat.inl");
			ImGui::MenuItem("fish_hat.h");
			if (ImGui::BeginMenu("More.."))
			{
				ImGui::MenuItem("Hello");
				ImGui::MenuItem("Sailor");
				if (ImGui::BeginMenu("Recurse.."))
				{
					ShowExampleMenuFile(pOpen);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save", "Ctrl+S")) {}
		if (ImGui::MenuItem("Save As..")) {}

		ImGui::Separator();
		if (ImGui::BeginMenu("Options"))
		{
			static bool enabled = true;
			ImGui::MenuItem("Enabled", "", &enabled);
			ImGui::BeginChild("child", ImVec2(0, 60), true);
			for (int i = 0; i < 10; i++)
				ImGui::Text("Scrolling Text %d", i);
			ImGui::EndChild();
			static float f = 0.5f;
			static int n = 0;
			ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
			ImGui::InputFloat("Input", &f, 0.1f);
			ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Colors"))
		{
			float sz = ImGui::GetTextLineHeight();
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
				ImGui::Dummy(ImVec2(sz, sz));
				ImGui::SameLine();
				ImGui::MenuItem(name);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options")) // <-- Append!
		{
			static bool b = true;
			ImGui::Checkbox("SomeOption", &b);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Disabled", false)) // Disabled
		{
			IM_ASSERT(0);
		}
		if (ImGui::MenuItem("Checked", NULL, true)) {}
		if (ImGui::MenuItem("Quit", "Alt+F4")) {}
	}
}


namespace vertex
{
	struct gradientMap
	{
		ImVec4 color1;
		ImVec4 color2;
		int vert_start_idx;
		ImRect bb;
		ImGuiWindow* window;
		ImVec2 col_pos_0;
		ImVec2 col_pos_1;
	};

	class gradientMapStack
	{
	private:
		static inline std::vector<gradientMap> stack;

	public:
		static inline gradientMap* last = nullptr;
		static void Push(const gradientMap& m)
		{
			stack.insert(stack.end(), m);
			last = &stack.back();
		}
		static gradientMap Get()
		{
			if (stack.size() == 0)
				return gradientMap();

			gradientMap m = stack.back();
			stack.pop_back();

			if (stack.size() < 1) last = nullptr;
			else last = &stack.back();

			return m;
		}
	};

	//static vertex::gradientMap mapBuf;

	// Fills mapBuf with current parameters
	// 
	/*static vertex::gradientMap PushGradientMap(const ImVec4& bg_color_1 = ImVec4(1, 1, 1, 1), const ImVec4& bg_color_2 = ImVec4(0, 0, 0, 0), ImVec2 mapSize = ImVec2(1920, 1080), ImVec2 mapOffset = ImVec2(0, 0))
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return vertex::gradientMap();

		const ImRect bbScreen(mapOffset, ImVec2(mapSize.x + mapOffset.x, mapSize.y + mapOffset.y));

		int vert_start_idx = window->DrawList->VtxBuffer.Size;
		vertex::gradientMap map;
		map.bb = bbScreen;
		map.color1 = bg_color_1;
		map.color2 = bg_color_2;
		map.vert_start_idx = vert_start_idx;
		map.window = window;
		gradientMapStack::Push(map);
		return map;
	}

	static vertex::gradientMap PushGradientMap(const ImVec4& bg_color_1 = ImVec4(1, 1, 1, 1), const ImVec4& bg_color_2 = ImVec4(0, 0, 0, 0), const ImRect& rectMinMax = ImRect(0, 0, 100, 100))
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return vertex::gradientMap();

		int vert_start_idx = window->DrawList->VtxBuffer.Size;
		vertex::gradientMap map;
		map.bb = rectMinMax;
		map.color1 = bg_color_1;
		map.color2 = bg_color_2;
		map.vert_start_idx = vert_start_idx;
		map.window = window;
		gradientMapStack::Push(map);
		return map;
	}*/

	static vertex::gradientMap PushGradientMap(ImVec2 mapStart, ImVec2 mapEnd, const ImVec4& bg_color_1 = ImVec4(1, 1, 1, 1), const ImVec4& bg_color_2 = ImVec4(0, 0, 0, 0), ImVec2 col_pos_0 = ImVec2(0, 0), ImVec2 col_pos_1 = ImVec2(1, 1))
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return vertex::gradientMap();

		int vert_start_idx = window->DrawList->VtxBuffer.Size;
		vertex::gradientMap map;
		map.bb = ImRect(mapStart.x, mapStart.y, mapEnd.x, mapEnd.y);
		map.color1 = bg_color_1;
		map.color2 = bg_color_2;
		map.vert_start_idx = vert_start_idx;
		map.window = window;
		map.col_pos_0 = col_pos_0;
		map.col_pos_1 = col_pos_1;

		gradientMapStack::Push(map);
		return map;
	}

	// To create map use PushGradientMap
	static void PopGradientMap(gradientMap map = gradientMapStack::Get())
	{
		if (!map.window) return;
		int vert_end_idx = map.window->DrawList->VtxBuffer.Size;

		ImVec2 bbSize = map.bb.Max - map.bb.Min;

		ImVec2 p0 = map.bb.Min + map.col_pos_0 * bbSize;
		ImVec2 p1 = map.bb.Min + map.col_pos_1 * bbSize;

		ImGui::ShadeVertsLinearColorGradientKeepAlpha(map.window->DrawList, map.vert_start_idx, vert_end_idx, p0, p1, ImGui::GetColorU32(map.color1), ImGui::GetColorU32(map.color2));
	}

	// Rewrited ShadeVertsLinearColorGradientKeepAlpha fn
	static void MyPopGradientMap(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
	{
		ImVec2 gradient_extent = ImVec2(gradient_p1.x - gradient_p0.x, gradient_p1.y - gradient_p0.y);
		float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
		ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
		ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
		const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
		const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
		const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
		const int col0_a = (int)(col0 >> IM_COL32_A_SHIFT) & 0xFF;
		const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
		const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
		const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
		const int col_delta_a = ((int)(col1 >> IM_COL32_A_SHIFT) & 0xFF) - col0_a;
		for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
		{
			ImVec2 pos = ImVec2(vert->pos.x - gradient_p0.x, vert->pos.y - gradient_p0.y);
			float d = ImDot(pos, gradient_extent);
			float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
			int r = (int)(col0_r + col_delta_r * t);
			int g = (int)(col0_g + col_delta_g * t);
			int b = (int)(col0_b + col_delta_b * t);
			int a = (int)(col0_a + col_delta_a * t);
			vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (a << IM_COL32_A_SHIFT);
		}
	}

	static void MyPopGradientMap228(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, Vec2 gradient_p0, Vec2 gradient_p1, ImU32 col0, ImU32 col1)
	{
		Vec2 gradient_extent = gradient_p1 - gradient_p0;
		Vec2 center = gradient_extent / 2;

		float lenght = gradient_extent.Lenght();
		float gradient_inv_length2 = 1.0f / lenght;
		ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
		ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
		const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
		const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
		const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
		const int col0_a = (int)(col0 >> IM_COL32_A_SHIFT) & 0xFF;
		const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
		const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
		const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
		const int col_delta_a = ((int)(col1 >> IM_COL32_A_SHIFT) & 0xFF) - col0_a;
		for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
		{
			Vec2 pos = Vec2(vert->pos.x - gradient_p0.x, vert->pos.y - gradient_p0.y);
			float delta = pos.DistanceTo(center) / lenght;

			float d = pos.DotProduct(gradient_extent);
			float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
			t = delta;
			t = CLAMP(0.f, 1.f, delta);
			int r = (int)(col0_r + col_delta_r * t);
			int g = (int)(col0_g + col_delta_g * t);
			int b = (int)(col0_b + col_delta_b * t);
			int a = (int)(col0_a + col_delta_a * t);


			vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (a << IM_COL32_A_SHIFT);
		}
	}

	static void PopGradientMapLinear(gradientMap map = gradientMapStack::Get())
	{
		if (!map.window) return;
		int vert_end_idx = map.window->DrawList->VtxBuffer.Size;
		MyPopGradientMap(map.window->DrawList, map.vert_start_idx, vert_end_idx, map.bb.Min, map.bb.Max, ImGui::GetColorU32(map.color1), ImGui::GetColorU32(map.color2));
	}
	static void PopGradientMapLinear228(gradientMap map = gradientMapStack::Get())
	{
		if (!map.window) return;
		int vert_end_idx = map.window->DrawList->VtxBuffer.Size;
		MyPopGradientMap228(map.window->DrawList, map.vert_start_idx, vert_end_idx, *(Vec2*)(&map.bb.Min), *(Vec2*)(&map.bb.Max), ImGui::GetColorU32(map.color1), ImGui::GetColorU32(map.color2));
	}

	static void MyPopGradientMapAlpha(ImDrawList* draw_list, bool isRelativeAlpha, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
	{
		ImVec2 gradient_extent = ImVec2(gradient_p1.x - gradient_p0.x, gradient_p1.y - gradient_p0.y);
		float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
		ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
		ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;

		const int col0_a = (int)(col0 >> IM_COL32_A_SHIFT) & 0xFF;
		const int col1_a = (int)(col1 >> IM_COL32_A_SHIFT) & 0xFF;
		const int col_delta_a = col1_a - col0_a;
		for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
		{
			ImVec2 pos = ImVec2(vert->pos.x - gradient_p0.x, vert->pos.y - gradient_p0.y);
			float d = ImDot(pos, gradient_extent);
			float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
			int aOriginal = (vert->col & IM_COL32_A_MASK);
			//printf("orig alpha %d\n", aOriginal);
			int a = (int)(col0_a + col_delta_a * t);
			if (isRelativeAlpha)
				a = (int)(aOriginal * t);
			vert->col = (vert->col & 0x000000FF) | (vert->col & 0x0000FF00) | (vert->col & 0x00FF0000) | (a << IM_COL32_A_SHIFT);
		}
	}

	static void PopGradientMapAlpha(float alpha, bool isRelativeAlpha = false, gradientMap map = gradientMapStack::Get())
	{
		if (!map.window) return;
		ImVec4 color = ImVec4(0, 0, 0, alpha);
		int vert_end_idx = map.window->DrawList->VtxBuffer.Size;
		MyPopGradientMapAlpha(map.window->DrawList, isRelativeAlpha, map.vert_start_idx, vert_end_idx, map.bb.Min, map.bb.Max, ImGui::GetColorU32(color), ImGui::GetColorU32(color));
	}

}