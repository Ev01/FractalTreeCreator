#include<SDL3/SDL.h>

#include "tree.h"

void drawTreeRecursive(SDL_Renderer *renderer, double x, double y, double angle, 
        const TreeSpecies &species, int maxDepth, int depth) {

    if (depth >= maxDepth) return;
    double length = species.baseBranchLen;

    double branchAngle, branchX, branchY;
    double newBranchSpread = species.branchSpread;
    if (depth > 0) {
        newBranchSpread *= SDL_pow(species.angleIncreaseFactor, depth);
        length *= SDL_pow(species.lengthIncreaseFactor, depth);
    }

    for (int i=0; i < species.numBranches; i++) {
        branchAngle = angle
            + newBranchSpread * ((double) i - (double)(species.numBranches-1) / 2);
           // * ((double) i / species.numBranches - (double) species.numBranches / 2.0);
        branchX = x + SDL_cos(branchAngle) * length;
        branchY = y - SDL_sin(branchAngle) * length;
        drawTreeRecursive(renderer, branchX, branchY, branchAngle, species, 
                maxDepth, depth+1+species.depthBiases[i]);
        SDL_RenderLine(renderer, x, y, branchX, branchY);
    }
}

/*
void drawTreeToSurfRecursive(SDL_Surface &surface, double x, double y, 
        double angle, int maxDepth=6, int depth=0) {

    double length = 50.0 - (double)depth/maxDepth * 25.0;
    double angleLeft = angle - 0.3;
    double angleRight = angle + 0.5;
    double leftX2 = x + SDL_cos(angleLeft) * length;
    double leftY2 = y - SDL_sin(angleLeft) * length;
    double rightX2 = x + SDL_cos(angleRight) * length;
    double rightY2 = y - SDL_sin(angleRight) * length;
    if (depth < maxDepth) {
        drawTreeToSurfRecursive(surface, leftX2, leftY2, 
                angleLeft, maxDepth, depth+1);
        drawTreeToSurfRecursive(surface, rightX2, rightY2, 
                angleRight, maxDepth, depth+1);
    }
    SDL_RenderLine(renderer, x, y, leftX2, leftY2);
    SDL_RenderLine(renderer, x, y, rightX2, rightY2);

*/
