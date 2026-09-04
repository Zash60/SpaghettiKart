#include "port/BfSim.h"
#include "port/BfMutator.h"
#include "port/BfState.h"
#include <macros.h>
#include <defines.h>
#include "common_structs.h"
#include "main.h"
#include "code_80005FD0.h"

static bool sPlaying = false;
static int sPlayIdx = 0;
static const BfInput* sCand = nullptr;
static int sCandLen = 0;
static int sCandIdx = 0;
static int sSimTicks = 0;
static int sSimMax = 0;
static int sSimResult = 0; // 0 running, >0 finish, -1 timeout
static int sSavedTickLogic = 2;
static BfRaceState* sSimReturn = nullptr; // where the game was when sim began
static BfInput sLastInput = { 0, 0, 0, 0, 0 };
static int sLastInputValid = 0;

void Bf_PlayBaseStart(void) {
    sPlaying = true;
    sPlayIdx = 0;
}

void Bf_PlayBaseStop(void) {
    sPlaying = false;
}

static void Inject(const BfInput& in) {
    gControllers[0].rawStickX = in.stickX;
    gControllers[0].rawStickY = in.stickY;
    gControllers[0].button = in.button;
    gControllers[0].buttonPressed = in.buttonPressed;
    gControllers[0].buttonDepressed = in.buttonDepressed;
    sLastInput = in;
    sLastInputValid = 1;
}

int Bf_IsPlaying(void) {
    return sPlaying ? 1 : 0;
}

int Bf_GetLastInput(BfInput* out) {
    if (!sLastInputValid || !out) {
        return -1;
    }
    *out = sLastInput;
    return 0;
}

void Bf_SimBegin(const BfInput* cand, int n, int maxTicks) {
    BfRaceState* root = Bf_RootState();
    if (!root) {
        sSimResult = -1;
        return;
    }
    // Park the live state aside, run the candidate from the search root
    // (record moment == base[0] by construction), restore on SimEnd.
    if (sSimReturn) {
        BfState_Free(sSimReturn);
    }
    sSimReturn = BfState_Alloc();
    BfState_Save(sSimReturn);
    BfState_Restore(root);
    sSavedTickLogic = gTickLogic;
    gTickLogic = Bf_SimSpeed();
    sCand = cand;
    sCandLen = n;
    sCandIdx = 0;
    sSimTicks = 0;
    sSimMax = maxTicks;
    sSimResult = 0;
}

int Bf_SimPoll(void) {
    return sSimResult;
}

void Bf_SimEnd(void) {
    if (sSimReturn) {
        BfState_Restore(sSimReturn);
        BfState_Free(sSimReturn);
        sSimReturn = nullptr;
    }
    gTickLogic = sSavedTickLogic;
    sCand = nullptr;
}

void Bf_OverrideTick(void) {
    // Drive the search state machine at most once per frame.
    static int sLastSearchFrame = -1;
    if (gGlobalTimer != sLastSearchFrame) {
        sLastSearchFrame = gGlobalTimer;
        Bf_SearchFrame();
    }
    if (sPlaying) {
        const BfInput* b = Bf_BaseData();
        int n = Bf_BaseLen();
        if (sPlayIdx < n) {
            Inject(b[sPlayIdx]);
        }
        sPlayIdx++;
        return;
    }
    if (!sCand || sSimResult != 0) {
        return;
    }
    if (sCandIdx < sCandLen) {
        Inject(sCand[sCandIdx]);
        sCandIdx++;
    }
    sSimTicks++;
    if (gLapCountByPlayerId[0] >= 3) {
        sSimResult = sSimTicks;
        return;
    }
    if (sSimTicks >= sSimMax) {
        sSimResult = -1;
    }
}
