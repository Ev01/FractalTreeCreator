#pragma once
#include<SDL3/SDL.h>

#define MAX_NUM_BRANCHES 8

struct TreeSpecies {
    int numBranches;
    double branchSpread;
    double baseBranchLen;
    int depthBiases[MAX_NUM_BRANCHES];
    double angleIncreaseFactor = 1;
    double lengthIncreaseFactor = 1;
    bool hasTrunk = 1;
    double lengthBiases[MAX_NUM_BRANCHES];
};

struct TreeSaveConfig {
    TreeSpecies species;
    int depth;
    double sway;
};


struct TreeLoadConfig {
    TreeSpecies *species;
    int *depth;
    double *sway;
};

const TreeSpecies TEST_SPECIES = {3, 0.30, 50, {1, 0, 1}, 1.3, 0.9};

void drawTreeRecursive(SDL_Renderer *renderer, double x, double y, 
        double angle, const TreeSpecies &species, double sway=0, 
        int maxDepth=4, unsigned int depth=0);

void saveConfig(const char *filename, const TreeSpecies &species, int depth, double sway);
void loadConfig(const char *filename, TreeLoadConfig *config);

void saveCallback(void* userdata, const char * const *filelist, int filter);
void loadCallback(void* userdata, const char * const *filelist, int filter);
