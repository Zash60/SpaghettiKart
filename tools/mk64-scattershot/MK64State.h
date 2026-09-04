#pragma once
#include <cstdint>
#include <cstring>
#include "InputGen.h"

#ifdef HEADLESS
// Real physics: use actual Player struct from game
#include "common_structs.h"
#define HEADLESS_START_LIGHTS 121 // measured 3-2-1-Go frames (STAGING timer=486 -> Go timer=607)
struct MK64State {
    Player players[8];
    uint32_t frame = 0;
    uint32_t rngSeed = 0;
    float courseTimer = 0;
    s32 globalTimer = 0;
    uint32_t lights = 0; // countdown frames remaining before Go
};
#else
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
#endif

MK64State save_state();
void load_state(const MK64State& s);
uint64_t hashPos(const MK64State& s);
bool truncEq(const MK64State& a, const MK64State& b);

// mock globals for non-headless test (real impl will use gPlayers etc)
#ifndef HEADLESS
extern KartSnapshot gMockKartStates[8];
extern uint32_t gMockFrame;
extern uint32_t gMockRngSeed;
#endif
void kart_tick(struct MK64Input& inp); // neutralized in-place during start lights
#ifdef HEADLESS
void headless_init_track(const char* track);
#ifdef __cplusplus
extern "C" {
#endif
void Scattershot_BuildCollision(void);
#ifdef __cplusplus
}
#endif
#endif
