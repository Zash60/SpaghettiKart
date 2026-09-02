#pragma once
#include <cstdint>
#include <cstring>
#include "InputGen.h"

struct KartSnapshot {
    int32_t posX = 0, posY = 0, posZ = 0;
    int16_t velX = 0, velY = 0, velZ = 0;
    int16_t angleY = 0;
    int16_t speed = 0;
    uint8_t lap = 0;
    uint8_t checkpoint = 0;
};

struct MK64State {
    KartSnapshot karts[8];
    uint32_t frame = 0;
    uint32_t rngSeed = 0;
};

MK64State save_state();
void load_state(const MK64State& s);
uint64_t hashPos(const MK64State& s);
bool truncEq(const MK64State& a, const MK64State& b);

// mock globals for headless test (real impl will use gKartStates etc)
extern KartSnapshot gMockKartStates[8];
extern uint32_t gMockFrame;
extern uint32_t gMockRngSeed;
void kart_tick(struct MK64Input inp); // forward decl for test
