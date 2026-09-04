#include "BfOverlay.h"
#include "port/BfBase.h"
#include "port/BfSim.h"
#include "port/BfMutator.h"
#include <imgui.h>

namespace BF {

static void DrawButtonLamp(const char* label, bool held) {
    ImVec4 col = held ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f) : ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, col);
    ImGui::Button(label, ImVec2(28, 22));
    ImGui::PopStyleColor();
    ImGui::SameLine();
}

void OverlayWindow::DrawElement() {
    // Prefered source: live recording tail > injected input > idle.
    BfInput in = { 0, 0, 0, 0, 0 };
    const char* src = "idle";
    if (Bf_IsRecording()) {
        const BfInput* d = Bf_BaseData();
        int n = Bf_BaseLen();
        if (n > 0) {
            in = d[n - 1];
        }
        src = "rec";
    } else if (Bf_GetLastInput(&in) == 0) {
        src = (Bf_IsPlaying() || Bf_IsSearching()) ? "sim" : "last";
    }

    ImGui::Text("BF [%s]", src);
    if (Bf_IsRecording()) {
        ImGui::SameLine();
        ImGui::Text("REC %d", Bf_BaseLen());
    }
    if (Bf_IsSearching()) {
        ImGui::SameLine();
        ImGui::Text("iter %d best %d", Bf_SearchIter(), Bf_BestTicks());
    }

    ImGui::Text("stick %+4d %+4d", (int)in.stickX, (int)in.stickY);
    int sx = in.stickX, sy = in.stickY;
    ImGui::SliderInt("X", &sx, -80, 80);
    ImGui::SliderInt("Y", &sy, -80, 80);

    DrawButtonLamp("A", (in.button & A_BUTTON) != 0);
    DrawButtonLamp("B", (in.button & B_BUTTON) != 0);
    DrawButtonLamp("Z", (in.button & Z_TRIG) != 0);
    DrawButtonLamp("R", (in.button & R_TRIG) != 0);
    ImGui::NewLine();
}

} // namespace BF
