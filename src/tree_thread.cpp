#include "tree_thread.h"
#include "tree.h"

#include <SDL3/SDL.h>

SDL_Mutex *buildingMutex;
SDL_Mutex *justFinishedBuildingMutex;
SDL_Condition *shouldBuildCond;
bool shouldBuild = false;
TreeBuildInfo *currentBuildInfo;
bool isCurrentlyBuilding = false;
bool shouldQuitThread = false;
bool justFinishedBuilding = false;

void InitTreeBuildMutexes()
{
    justFinishedBuildingMutex = SDL_CreateMutex();
    buildingMutex = SDL_CreateMutex();
    shouldBuildCond = SDL_CreateCondition();
}

void DeleteTreeBuildMutexes()
{
    SDL_DestroyMutex(justFinishedBuildingMutex);
    SDL_DestroyMutex(buildingMutex);
    SDL_DestroyCondition(shouldBuildCond);
}

int TreeBuildThread(void *data)
{
    while (!shouldQuitThread) {
        SDL_LockMutex(buildingMutex);
        SDL_WaitCondition(shouldBuildCond, buildingMutex);
        if (shouldQuitThread) {
            SDL_UnlockMutex(buildingMutex);
            return 0;
        }
        isCurrentlyBuilding = true;
        Uint64 startTime = SDL_GetTicksNS();
        BuildTree(currentBuildInfo->species, currentBuildInfo->vertices, currentBuildInfo->verticesSize,
                  currentBuildInfo->indices, currentBuildInfo->indicesSize, currentBuildInfo->sway,
                  currentBuildInfo->maxDepth);
        Uint64 endTime = SDL_GetTicksNS();
        //timeToLastBuild = (double)(endTime - startTime) / SDL_NS_PER_SECOND;
        isCurrentlyBuilding = false;
        SDL_LockMutex(justFinishedBuildingMutex);
        justFinishedBuilding = true;
        SDL_UnlockMutex(justFinishedBuildingMutex);
        SDL_UnlockMutex(buildingMutex);
    }
    return 0;
}
bool JustFinishedBuilding()
{
    return justFinishedBuilding;
}
void ClearJustFinishedBuilding()
{
    SDL_LockMutex(justFinishedBuildingMutex);
    justFinishedBuilding = false;
    SDL_UnlockMutex(justFinishedBuildingMutex);
}

void QuitTreeBuildThread()
{
    shouldQuitThread = true;
    SDL_SignalCondition(shouldBuildCond);
}

bool IsCurrentlyBuildingTree()
{
    return isCurrentlyBuilding;
}


void BuildTreeMultithread(TreeBuildInfo *buildInfo)
{
    if (!IsCurrentlyBuildingTree()) {
        currentBuildInfo = buildInfo;
        SDL_SignalCondition(shouldBuildCond);
    }
}
