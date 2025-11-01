#include<SDL3/SDL.h>
#include <string>
#include <cstdio>
//#include <iostream>

#include "tree.h"
#include "debug.h"
#include "vendored/nlohmann/json.hpp"

using json = nlohmann::json;

void drawTreeRecursive(SDL_Renderer *renderer, double x, double y, double angle, 
        const TreeSpecies &species, double sway, 
        int maxDepth, unsigned int depth) {

    SDL_assert(species.numBranches <= MAX_NUM_BRANCHES);

    if (depth >= maxDepth) return;
    double length = species.baseBranchLen;

    double branchLength, branchAngle, branchX, branchY;
    double newBranchSpread = species.branchSpread;
    unsigned int depthFromTrunk = 0;
    if (depth > 0) {
        depthFromTrunk = depth - 1;
        newBranchSpread *= SDL_pow(species.angleIncreaseFactor, depthFromTrunk);
        length *= SDL_pow(species.lengthIncreaseFactor, depth);
    }
    else if (species.hasTrunk) {
        // Draw one branch when depth is 0. This is the trunk
        double swayAngleMod = sway;
        angle += swayAngleMod;
        branchX = x + SDL_cos(angle) * length;
        branchY = y - SDL_sin(angle) * length;
        drawTreeRecursive(renderer, branchX, branchY, angle, species, sway, 
                maxDepth, depth+1);
        SDL_RenderLine(renderer, x, y, branchX, branchY);
        Debug_incDrawCalls();
        return;
    }
    

    for (int i=0; i < species.numBranches; i++) {
        branchLength = length * (species.lengthBiases[i] + 1);
        branchAngle = angle
            + newBranchSpread * ((double) i - (double)(species.numBranches-1) / 2);

        double swayAngleMod = SDL_sin(branchAngle) * sway;
        branchAngle += swayAngleMod;

        branchX = x + SDL_cos(branchAngle) * branchLength;
        branchY = y - SDL_sin(branchAngle) * branchLength;
        drawTreeRecursive(renderer, branchX, branchY, branchAngle, species, sway,
                maxDepth, depth+1+species.depthBiases[i]);
        SDL_RenderLine(renderer, x, y, branchX, branchY);
        Debug_incDrawCalls();
    }
}

void saveCallback(void* userdata, const char * const *filelist, int filter) {
    TreeSaveConfig *config = (TreeSaveConfig*) userdata;
    //SDL_Log("%d", config->depth);
    //SDL_Log("%s", filelist[0]);
    saveConfig(filelist[0], config->species, config->depth, config->sway);
    SDL_free(config);
}

void loadCallback(void* userdata, const char * const *filelist, int filter) {
    TreeLoadConfig *config = (TreeLoadConfig*) userdata;
    loadConfig(filelist[0], config);
    SDL_free(config);
}


void saveConfig(const char *filename, const TreeSpecies &species, int depth, double sway) {
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

void loadConfig(const char *filename, TreeLoadConfig *config) {
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

