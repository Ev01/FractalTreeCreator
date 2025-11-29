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

    bool operator==(const TreeSpecies &other) const;
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

const TreeSpecies TEST_SPECIES = {2, 0.30, 50, {0, 0}, 1.0, 1.0};

void BuildTree(const TreeSpecies &species, float *vertices, int &verticesSize,
        unsigned int *indices, int &indicesSize, float sway, int maxDepth);

void BuildTreeRecursive(const TreeSpecies &species, float *vertices, 
               int &verticesSize, unsigned int *indices, int &indicesSize, 
               int baseIndex, double angle, float sway, int maxDepth,
               unsigned int depth);

void SaveConfig(const char *filename, const TreeSpecies &species, int depth,
                double sway);
void LoadConfig(const char *filename, TreeLoadConfig *config);

/* Used with SDL_ShowSaveFileDialog to save to file */
void SaveCallback(void* userdata, const char * const *filelist, int filter);
/* Used with SDL_ShowOpenFileDialog to load from file */
void LoadCallback(void* userdata, const char * const *filelist, int filter);

double GetTimeToLastBuild();

// The recently loaded variable is set to true when the user loads a new tree.
// It can be set to false again by calling clearTreeRecentlyLoaded(). This is
// used for rebuilding the tree when a new species is loaded.
bool GetTreeRecentlyLoaded();
void ClearTreeRecentlyLoaded();
