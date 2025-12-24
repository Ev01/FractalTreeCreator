#pragma once

#include<SDL3/SDL.h>

struct TreeBuildInfo;

int TreeBuildThread(void *data);
void DeleteTreeBuildMutexes();
void InitTreeBuildMutexes();
bool IsCurrentlyBuildingTree();
void ClearJustFinishedBuilding();
bool JustFinishedBuilding();
void QuitTreeBuildThread();
void BuildTreeMultithread(TreeBuildInfo *buildInfo);
