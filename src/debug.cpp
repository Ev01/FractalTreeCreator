#include "debug.h"

unsigned int frameDrawCalls = 0;

unsigned int Debug_getDrawCalls() {
    return frameDrawCalls;
}

void Debug_incDrawCalls() {
    frameDrawCalls++;
}

void Debug_newFrame() {
    frameDrawCalls = 0;
}
