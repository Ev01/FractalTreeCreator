#include<SDL3/SDL.h>
#include <string>
#include <cstdio>
//#include <iostream>

#include "tree.h"
#include "render.h"
#include "debug.h"
#include "vendored/nlohmann/json.hpp"

using json = nlohmann::json;

double timeToLastBuild = 0.0;

bool TreeSpecies::operator==(const TreeSpecies &other) const
{
    //double err = 0.0001;
    bool depthBiasesEqual;
    if (numBranches == other.numBranches) {
        for (int i = 0; i < numBranches; i++) {
            if (depthBiases[i] != other.depthBiases[i]) {
                return false;
            }
            if (lengthBiases[i] != other.lengthBiases[i]) {
                return false;
            }
        }
    }
    return numBranches == other.numBranches
            && branchSpread == other.branchSpread
            && baseBranchLen == other.baseBranchLen
            && angleIncreaseFactor == other.angleIncreaseFactor
            && lengthIncreaseFactor == other.lengthIncreaseFactor
            && hasTrunk == other.hasTrunk;
}


void buildTreeRecursive(const TreeSpecies &species, float *vertices,
        int &verticesSize, unsigned int *indices, int &indicesSize,
        int baseIndex, double angle, float sway, int maxDepth,
        unsigned int depth)
{
    SDL_assert(species.numBranches <= MAX_NUM_BRANCHES);

    if (depth >= maxDepth) return;

    float length = species.baseBranchLen;
    float branchLength, branchAngle, branchEndX, branchEndY;
    float newBranchSpread = species.branchSpread;
    float baseX = vertices[baseIndex * 2];
    float baseY = vertices[baseIndex * 2 + 1];

    unsigned int depthFromTrunk = 0;
    if (depth > 0) {
        depthFromTrunk = depth - 1;
        newBranchSpread *= SDL_pow(species.angleIncreaseFactor, depthFromTrunk);
        length *= SDL_pow(species.lengthIncreaseFactor, depth);
    }

    int numBranches = species.numBranches;
    // Just draw the trunk this time, which is one branch
    if (species.hasTrunk && depth == 0) {
        numBranches = 1;
    }

    for (int i=0; i < numBranches; i++) {
        if (verticesSize + 2 >= MAX_VERTICES || indicesSize + 2 >= MAX_INDICES) {
            return;
        }

        branchLength = length;
        branchAngle = angle
            + newBranchSpread * ((double) i - (double)(numBranches-1) / 2);

        int newDepth = depth + 1;
        if (!species.hasTrunk || depth > 0) {
            // Increase length based on length bias if this is not the trunk
            branchLength *= (species.lengthBiases[i] + 1);
            // Increase depth based on depth bias if this is not the trunk
            newDepth += species.depthBiases[i];
        }

        double swayAngleMod = SDL_sin(branchAngle) * sway;
        branchAngle += swayAngleMod;

        branchEndX = baseX + SDL_cos(branchAngle) * branchLength;
        branchEndY = baseY - SDL_sin(branchAngle) * branchLength;
        int endIndex = verticesSize / 2;
        // Add the end point of the branch to the vertices
        vertices[verticesSize++] = branchEndX;
        vertices[verticesSize++] = branchEndY;
        // Add the branch line
        indices[indicesSize++] = baseIndex;
        indices[indicesSize++] = endIndex;


        buildTreeRecursive(species, vertices, verticesSize, indices, 
                indicesSize, endIndex, branchAngle, sway, maxDepth, newDepth);

        //Debug_incDrawCalls();
    }
}

void buildTree(const TreeSpecies &species, float *vertices, int &verticesSize, 
        unsigned int *indices, int &indicesSize, float sway, int maxDepth)
{
    SDL_assert(species.numBranches <= MAX_NUM_BRANCHES);
    Uint64 startTime = SDL_GetTicksNS();
    vertices[0] = 0.0;
    vertices[1] = -0.5;
    verticesSize = 2;
    buildTreeRecursive(species, vertices, verticesSize, indices, indicesSize, 
              0, -SDL_PI_D/2.0, sway, maxDepth, 0);
    Uint64 endTime = SDL_GetTicksNS();
    timeToLastBuild = (double)(endTime - startTime) / SDL_NS_PER_SECOND;
}
    

void saveCallback(void* userdata, const char * const *filelist, int filter) 
{
    TreeSaveConfig *config = (TreeSaveConfig*) userdata;
    //SDL_Log("%d", config->depth);
    //SDL_Log("%s", filelist[0]);
    saveConfig(filelist[0], config->species, config->depth, config->sway);
    SDL_free(config);
}

void loadCallback(void* userdata, const char * const *filelist, int filter) 
{
    TreeLoadConfig *config = (TreeLoadConfig*) userdata;
    loadConfig(filelist[0], config);
    SDL_free(config);
}


void saveConfig(const char *filename, const TreeSpecies &species, int depth,
        double sway) 
{
    SDL_IOStream *user = SDL_IOFromFile(filename, "w");
    if (user == NULL) {
        SDL_Log("Couldn't open file");
    } else {

        json j;
        j["numBranches"] = species.numBranches;
        j["branchSpread"] = species.branchSpread;
        j["angleIncreaseFactor"] = species.angleIncreaseFactor;
        j["lengthIncreaseFactor"] = species.lengthIncreaseFactor;
        j["hasTrunk"] = species.hasTrunk;
        j["baseBranchLen"] = species.baseBranchLen;
        j["sway"] = sway;
        j["depth"] = depth;
        for (int i = 0; i < MAX_NUM_BRANCHES; i++) {
            char label[20];
            SDL_snprintf(label, sizeof(label), "depthBias%i", i);
            j[label] = species.depthBiases[i];
            SDL_snprintf(label, sizeof(label), "lengthBias%i", i);
            j[label] = species.lengthBiases[i];
        }

        std::string str = j.dump();

        char* toSave = str.data();

        size_t len = str.size();//sizeof(toSave);
        SDL_WriteIO(user, toSave, len);

        SDL_CloseIO(user);
    }
}

void loadConfig(const char *filename, TreeLoadConfig *config) 
{
    SDL_IOStream *file = SDL_IOFromFile(filename, "r");
    double *sway = config->sway;
    int *depth = config->depth;
    TreeSpecies *species = config->species;

    if (file == NULL) {
        SDL_Log("Couldn't open file");
    } else {
        //char data[1024];
        // NOTE: the program crashes if this is a char of size 1. I don't know
        // why.
        char c[2];
        std::string str;
        size_t bytesRead;
        int i = 0;
        
        do {
            bytesRead = SDL_ReadIO(file, c, 1);
            //SDL_Log("bytes read: %i", bytesRead);
            if (bytesRead) {
                str += c[0];
                //SDL_Log("add char %s", c);
                i++;
            }
        } while (bytesRead);
        

        if (SDL_GetIOStatus(file) != SDL_IO_STATUS_EOF) {
            SDL_Log("Error reading file: %s", SDL_GetError());
            SDL_CloseIO(file);
            return;
        }
        
        //SDL_Log("Read file");
        /* 
        for (int i = 0; i < str.size(); i++) {
            data[i] = str[i];
        }

        SDL_Log("string is %s", data);
        */
        
        json j = json::parse(str);
        *depth = j["depth"];
        *sway = j["sway"];
         
        species->numBranches = j["numBranches"];
        species->branchSpread = j["branchSpread"];
        species->angleIncreaseFactor = j["angleIncreaseFactor"];
        species->lengthIncreaseFactor = j["lengthIncreaseFactor"];
        species->hasTrunk = j["hasTrunk"];
        species->baseBranchLen = j["baseBranchLen"];
        for (int i = 0; i < MAX_NUM_BRANCHES; i++) {
            char label[20];
            SDL_snprintf(label, sizeof(label), "depthBias%i", i);
            species->depthBiases[i] = j[label];
            SDL_snprintf(label, sizeof(label), "lengthBias%i", i);
            species->lengthBiases[i] = j[label];
        }
    }
    SDL_CloseIO(file);
}


double getTimeToLastBuild()
{
    return timeToLastBuild;
}
