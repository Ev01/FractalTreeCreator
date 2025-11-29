#pragma once
#include <SDL3/SDL.h>
#include "tree.h"

bool TreeConfigWindow(TreeSpecies &species, int &depth, double &sway, SDL_Window *window);

void DebugInfoWindow(double delta);
