#include "port/BfBase.h"
#include "port/BfState.h"
#include <vector>

#include <macros.h>
#include <defines.h>
#include "common_structs.h"
#include "main.h"
#include "code_80005FD0.h"

static std::vector<BfInput> sBase;
static bool sRecording = false;
static BfRaceState* sRoot = nullptr;
static int sRootTimer = 0;
static int sRootLap = 0;

void Bf_RecordStart(void) {
    sBase.clear();
    if (sRoot) {
        BfState_Free(sRoot);
    }
    sRoot = BfState_Alloc();
    BfState_Save(sRoot);
    sRootTimer = gGlobalTimer;
    sRootLap = gLapCountByPlayerId[0];
    sRecording = true;
}

void Bf_RecordStop(void) {
    sRecording = false;
}

void Bf_RecordTick(void) {
    if (!sRecording) {
        return;
    }
    BfInput in;
    in.stickX = gControllers[0].rawStickX;
    in.stickY = gControllers[0].rawStickY;
    in.button = gControllers[0].button;
    in.buttonPressed = gControllers[0].buttonPressed;
    in.buttonDepressed = gControllers[0].buttonDepressed;
    sBase.push_back(in);
}

const BfInput* Bf_BaseData(void) {
    return sBase.data();
}

int Bf_BaseLen(void) {
    return (int)sBase.size();
}

void Bf_SetBase(const BfInput* d, int n) {
    sBase.assign(d, d + n);
}

BfRaceState* Bf_RootState(void) {
    return sRoot;
}

int Bf_RootSessionOk(void) {
    if (!sRoot) {
        return -1;
    }
    // gGlobalTimer is monotonic within a race; a smaller value means the
    // race was restarted (or left) after recording. Same for lap count.
    if (gGlobalTimer < sRootTimer || gLapCountByPlayerId[0] < sRootLap) {
        return -1;
    }
    return 0;
}

void Bf_Reanchor(void) {
    if (sRoot) {
        BfState_Free(sRoot);
    }
    sRoot = BfState_Alloc();
    BfState_Save(sRoot);
    sRootTimer = gGlobalTimer;
    sRootLap = gLapCountByPlayerId[0];
}
