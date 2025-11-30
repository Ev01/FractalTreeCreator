#include <SDL3/SDL.h>
#include "ui.h"
#include "tree.h"
#include "render.h"

#include "../vendored/imgui/imgui.h"

const double MAX_ANGLE_INCREASE_FACT = 2.0;
const double MAX_LENGTH_INCREASE_FACT = 3.0;
const double MIN_SWAY = -SDL_PI_D / 4.0;
const double MAX_SWAY = SDL_PI_D / 4.0;

SDL_DialogFileFilter fileFilters[] = {
    {"Tree file", "tree"},
    {"All files", "*"}
};
const int numFileFilters = 2;

bool TreeConfigWindow(TreeSpecies &species, int &depth, double &sway, SDL_Window *window) {
    TreeSpecies beforeSpecies = species;

    ImGui::Begin("Tree Config");
    ImGui::PushItemWidth(-165);
    ImGui::Text("Configure your own tree!!");

    ImGui::SliderInt("Depth", &depth, 1, 12);

    ImGui::SliderInt("Number of Branches", &species.numBranches, 1, 8, 
            "%d", ImGuiSliderFlags_ClampOnInput);

    static const double zero = 0.0;
    static const double MIN_LENGTH_BIAS = -1.0;
    static const double MAX_LENGTH_BIAS = 3.0;
    static const double spreadMax = 3.14159;
    static const double lengthMax = 500.0;
    ImGui::SliderScalar("Branch Spread", ImGuiDataType_Double, 
            &species.branchSpread, &zero, &spreadMax, "%.3f");
    ImGui::SliderScalar("Sway", ImGuiDataType_Double, 
            &sway, &MIN_SWAY, &MAX_SWAY, "%.3f");
    ImGui::SliderScalar("Base Branch Length", ImGuiDataType_Double, 
            &species.baseBranchLen, &zero, &lengthMax, "%.2f");

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

    ImGui::SeparatorText("Length Biases");
    // Length Bias sliders
    for (int i = 0; i < species.numBranches; i++) {
        char label[20];
        SDL_snprintf(label, sizeof(label), "Length Bias %i", i);
        ImGui::SliderScalar(label, ImGuiDataType_Double, 
                species.lengthBiases + i, 
                &MIN_LENGTH_BIAS, &MAX_LENGTH_BIAS, "%.3f");
    }

    if (ImGui::Button("Save")) {
        TreeSaveConfig config = {species, depth, sway};
        void *dest = SDL_malloc(sizeof(config));
        SDL_memcpy(dest, &config, sizeof(config));
        
        SDL_ShowSaveFileDialog(SaveCallback, dest, window,
                fileFilters, numFileFilters, NULL);
        //saveConfig(species, depth, sway);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        //loadConfig(species, depth, sway);
        TreeLoadConfig config = {&species, &depth, &sway};
        void *data = SDL_malloc(sizeof(config));
        SDL_memcpy(data, &config, sizeof(config));

        SDL_ShowOpenFileDialog(LoadCallback, data, window,
                fileFilters, numFileFilters, NULL, false);
    }
    

    ImGui::End();

    bool configChanged = !(beforeSpecies == species);
    return configChanged;
}

void DebugInfoWindow(double delta) {
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", 1.0 / delta);

    DrawInfo &treeDrawInfo = Render::GetTreeDrawInfo();

    ImGui::Text("Vertices: %d", treeDrawInfo.verticesSize / 2);
    ImGui::Text("Num Branches: %d", treeDrawInfo.indicesSize / 2);
    ImGui::Text("Indices: %d", treeDrawInfo.indicesSize);
    ImGui::Text("Time to last build: %.3f", GetTimeToLastBuild());
    //ImGui::Text("Draw calls: %d", (int)Debug_getDrawCalls());
    ImGui::End();
}

void RenderSettingsWindow()
{
    ImGui::Begin("Render Settings");

    const char *sampleOptions[] = {"Disabled", "4x MSAA", "8x MSAA", "16x MSAA"};
    int currentSampleOption = 0;
    int samples = Render::GetMSAASamples();
    switch (samples) {
        case 1:  currentSampleOption = 0; break;
        case 4:  currentSampleOption = 1; break;
        case 8:  currentSampleOption = 2; break;
        case 16: currentSampleOption = 3; break;
    }
    
    ImGui::Combo("Anti-Aliasing", &currentSampleOption, sampleOptions, 4);
    switch (currentSampleOption) {
        case 0: samples = 1;  break;
        case 1: samples = 4;  break;
        case 2: samples = 8;  break;
        case 3: samples = 16; break;
    }
    Render::SetMSAASamples(samples);


    ImGui::End();
}
