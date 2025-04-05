#include <SDL3/SDL.h>
#include "ui.h"
#include "debug.h"

#include "imgui/imgui.h"

const double MAX_ANGLE_INCREASE_FACT = 2.0;
const double MAX_LENGTH_INCREASE_FACT = 3.0;

void treeConfigWindow(TreeSpecies &species, int &depth) {
    ImGui::Begin("Tree Config");
    ImGui::PushItemWidth(-165);
    ImGui::Text("Configure your own tree!!");

    ImGui::SliderInt("Depth", &depth, 1, 8);

    ImGui::SliderInt("Number of Branches", &species.numBranches, 1, 8);
    static const double zero = 0.0;
    static const double spreadMax = 3.14159;
    static const double lengthMax = 200.0;
    ImGui::SliderScalar("Branch Spread", ImGuiDataType_Double, 
            &species.branchSpread, &zero, &spreadMax, "%.3f");
    ImGui::SliderScalar("Base Branch Length", ImGuiDataType_Double, 
            &species.baseBranchLen, &zero, &lengthMax, "%.1f");

    ImGui::SliderScalar("Angle Increase Factor", ImGuiDataType_Double,
            &species.angleIncreaseFactor, &zero, &MAX_ANGLE_INCREASE_FACT, 
            "%.3f");

    ImGui::SliderScalar("Length Increase Factor", ImGuiDataType_Double,
            &species.lengthIncreaseFactor, &zero, &MAX_LENGTH_INCREASE_FACT, 
            "%.3f");

    ImGui::Checkbox("Has Trunk", &species.hasTrunk);

    ImGui::SeparatorText("Depth Biases");
    // Depth Bias sliders
    for (int i = 0; i < species.numBranches; i++) {
        char label[20];
        SDL_snprintf(label, sizeof(label), "Depth Bias %i", i);
        ImGui::SliderInt(label, species.depthBiases + i, 0, 10);
    }

    ImGui::End();
}

void debugInfoWindow(double delta) {
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", 1.0 / delta);
    ImGui::Text("Draw calls: %d", (int)Debug_getDrawCalls());
    ImGui::End();
}
