#pragma once
#include <stdint.h>

struct BfInput {
    int16_t stickX;
    int16_t stickY;
    uint16_t button;
};

void Bf_RecordStart(void);
void Bf_RecordStop(void);
void Bf_RecordTick(void);
const BfInput* Bf_BaseData(void);
int Bf_BaseLen(void);
void Bf_SetBase(const BfInput* d, int n);
