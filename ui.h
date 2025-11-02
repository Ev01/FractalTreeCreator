#pragma once
#include <SDL3/SDL.h>
#include "tree.h"

bool treeConfigWindow(TreeSpecies &species, int &depth, double &sway, SDL_Window *window);

void debugInfoWindow(double delta);
