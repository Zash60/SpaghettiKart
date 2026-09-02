#include "MK64State.h"
#include "Utils.h"

KartSnapshot gMockKartStates[8] = {};
uint32_t gMockFrame = 0;
uint32_t gMockRngSeed = 0;

MK64State save_state(){
    MK64State s;
    memcpy(s.karts, gMockKartStates, sizeof(gMockKartStates));
    s.frame = gMockFrame;
    s.rngSeed = gMockRngSeed;
    return s;
}
void load_state(const MK64State& s){
    memcpy(gMockKartStates, s.karts, sizeof(gMockKartStates));
    gMockFrame = s.frame;
    gMockRngSeed = s.rngSeed;
}
uint64_t hashPos(const MK64State& s){
    uint64_t seed = 0xCABBA6ECABBA6E;
    seed += (s.karts[0].posX/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += (s.karts[0].posY/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += (s.karts[0].posZ/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += s.karts[0].speed*10 + 0xCABBA6E; Utils::xoro_r(&seed);
    return seed;
}
bool truncEq(const MK64State& a, const MK64State& b){
    return a.karts[0].posX==b.karts[0].posX && a.karts[0].posY==b.karts[0].posY && a.karts[0].posZ==b.karts[0].posZ && a.karts[0].speed==b.karts[0].speed;
}
void kart_tick(struct MK64Input inp){
    // stub: move kart a bit based on input for test
    gMockKartStates[0].posX += inp.stick_x;
    gMockKartStates[0].posZ += inp.stick_y;
    if(inp.A) gMockKartStates[0].speed += 1;
    gMockFrame++;
}
