#include "port/BfBase.h"
#include <vector>

#include <macros.h>
#include <defines.h>
#include "common_structs.h"
#include "main.h"

static std::vector<BfInput> sBase;
static bool sRecording = false;

void Bf_RecordStart(void) {
    sBase.clear();
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
