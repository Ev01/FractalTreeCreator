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
};

const TreeSpecies TEST_SPECIES = {3, 0.30, 50, {1, 0, 1}, 1.3, 0.9};

void drawTreeRecursive(SDL_Renderer *renderer, double x, double y, 
        double angle, const TreeSpecies &species, int maxDepth=4, int depth=0);


// void drawTreeToSurfRecursive(SDL_Surface &surface, double x, double y, 
//        double angle, int maxDepth=5, int depth=0);
