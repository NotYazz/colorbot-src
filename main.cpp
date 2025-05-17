#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <thread>
#include <atomic>
#include "overlay.h"
#include "detection_lib.h"

int fov = 150;
int x_offset = 0;
int y_offset = -5;
int hotkey = VK_XBUTTON2;
int smoothing = 4;
bool bot_enabled = true;
bool detection_circle = false;
bool stream_proof = true;

cv::Scalar color_lower(55, 200, 200);
cv::Scalar color_upper(70, 255, 255);

std::atomic<DetectionLib::Target> detected_target;

void drawOverlay(int screenWidth, int screenHeight) {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 center((float)screenWidth / 2.0f, (float)screenHeight / 2.0f);

    draw_list->AddCircle(
        center,
        (float)fov,
        IM_COL32(255, 255, 255, 255),
        64,
        2.0f
    );

    DetectionLib::Target target = detected_target.load();
    if (target.valid && target.pos.x != -1 && target.pos.y != -1) {
		if (detection_circle)
		{
			draw_list->AddCircle(
				ImVec2((float)target.pos.x, (float)target.pos.y),
				7.5f,
				IM_COL32(255, 0, 0, 255)
			);
		}
    }
}

void setStyle() {
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 7.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 4.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(5.0f, 2.0f);
	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
	style.IndentSpacing = 25.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1030042767524719f, 0.1030032485723495f, 0.1030032485723495f, 0.9356223344802856f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.9200000166893005f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.2899999916553497f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.239999994635582f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4000000059604645f, 0.4000000059604645f, 0.4000000059604645f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.3600000143051147f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 0.3300000131130219f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 0.3600000143051147f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.3499999940395355f);
}

void drawMenu() {
	ImGui::Begin("ACEWARE COLOR");

	if (ImGui::BeginTabBar("##tabs")) {
		if (ImGui::BeginTabItem("Aimbot")) {
			ImGui::Checkbox("Aimbot", &bot_enabled);
			if (bot_enabled)
			{
				ImGui::SeparatorText("Aimbot Settings");
				ImGui::SliderInt("FOV", &fov, 5, 1000);
				ImGui::SliderInt("X Offset", &x_offset, -100, 100);
				ImGui::SliderInt("Y Offset", &y_offset, -100, 100);
				ImGui::SliderInt("Smoothing", &smoothing, 0, 30);

				ImGui::Text("Aim key:");
				ImGui::RadioButton("Side Button 1", &hotkey, VK_XBUTTON2);
				ImGui::SameLine();
				ImGui::RadioButton("Side Button 2", &hotkey, VK_XBUTTON1);
				ImGui::SameLine();
				ImGui::RadioButton("LMB", &hotkey, VK_LBUTTON);
				ImGui::SameLine();
				ImGui::RadioButton("RMB", &hotkey, VK_RBUTTON);
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Misc")) {
			ImGui::Checkbox("Detection Circle", &detection_circle);
			ImGui::Checkbox("Stream Proof", &stream_proof);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}


void detectionThreadFunc() {
    while (true) {
        int offsetX = 0, offsetY = 0;
        cv::Mat region = DetectionLib::CaptureFOVRegion(fov, offsetX, offsetY);
        DetectionLib::Target target = DetectionLib::FindTargetInRegion(
            region, fov, x_offset + offsetX, y_offset + offsetY, color_lower, color_upper
        );
        detected_target.store(target);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void aimingThreadFunc() {
    while (true) {
        if (bot_enabled && (GetAsyncKeyState(hotkey) & 0x8000)) {
            DetectionLib::Target target = detected_target.load();
            if (target.valid && target.pos.x != -1 && target.pos.y != -1) {
                POINT mousePos;
                GetCursorPos(&mousePos);
                int dx = target.pos.x - mousePos.x;
                int dy = target.pos.y - mousePos.y;
                int smooth = smoothing < 1 ? 1 : smoothing;
                DetectionLib::MoveMouseRelativeSmooth(dx, dy, smooth);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    overlay::SetupWindow();
    if (!(overlay::CreateDeviceD3D(overlay::Window)))
        return 1;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    std::thread(detectionThreadFunc).detach();
    std::thread(aimingThreadFunc).detach();

	setStyle();

    while (!overlay::ShouldQuit) {
        overlay::Render();

		if (stream_proof)
		{
			SetWindowDisplayAffinity(overlay::Window, WDA_EXCLUDEFROMCAPTURE);
		}
		else {
			SetWindowDisplayAffinity(overlay::Window, WDA_NONE);
		}

        drawOverlay(screenWidth, screenHeight);
        drawMenu();
        overlay::EndRender();
    }

    overlay::CloseOverlay();
    return 0;
}
