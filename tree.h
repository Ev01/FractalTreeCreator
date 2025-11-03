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

void drawTreeRecursive(SDL_Renderer *renderer, double x, double y, 
        double angle, const TreeSpecies &species, double sway=0, 
        int maxDepth=4, unsigned int depth=0);

void buildTree(const TreeSpecies &species, float *vertices, int &verticesSize, unsigned int *indices,
                int &indicesSize, float sway, int maxDepth);

void buildTreeRecursive(const TreeSpecies &species, float *vertices, 
               int &verticesSize, unsigned int *indices, int &indicesSize, 
               int baseIndex, double angle, float sway, int maxDepth, unsigned int depth);

void saveConfig(const char *filename, const TreeSpecies &species, int depth, double sway);
void loadConfig(const char *filename, TreeLoadConfig *config);

/* Used with SDL_ShowSaveFileDialog to save to file */
void saveCallback(void* userdata, const char * const *filelist, int filter);
/* Used with SDL_ShowOpenFileDialog to load from file */
void loadCallback(void* userdata, const char * const *filelist, int filter);
