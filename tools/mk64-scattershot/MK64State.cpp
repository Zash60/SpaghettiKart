#include "MK64State.h"
#include "Utils.h"

#ifndef HEADLESS
KartSnapshot gMockKartStates[8] = {};
uint32_t gMockFrame = 0;
uint32_t gMockRngSeed = 0;
#endif

#ifdef HEADLESS
// Real Player struct but engine-free physics (MK64-accurate mock: stick->angle, A->accel, R->drift)
// Avoids linking full engine (GetWorld/FrameInterpolation) while using Player 0xDD8 layout
static MK64State gHeadlessState{};
MK64State save_state(){ return gHeadlessState; }
void load_state(const MK64State& s){ gHeadlessState = s; }
uint64_t hashPos(const MK64State& s){
    uint64_t seed = 0xCABBA6ECABBA6E;
    seed += int(s.players[0].pos[0]/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].pos[1]/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].pos[2]/100) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].speed*10) + 0xCABBA6E; Utils::xoro_r(&seed);
    return seed;
}
bool truncEq(const MK64State& a, const MK64State& b){
    return int(a.players[0].pos[0])==int(b.players[0].pos[0]) && int(a.players[0].pos[1])==int(b.players[0].pos[1]) && int(a.players[0].pos[2])==int(b.players[0].pos[2]) && int(a.players[0].speed)==int(b.players[0].speed);
}
void kart_tick(struct MK64Input inp){
    // MK64-ish: stick_x -80..80 -> yaw, stick_y -> pitch (unused), A accel, B brake, R drift
    float yaw = gHeadlessState.players[0].rotation[1];
    yaw += inp.stick_x * 0.015f; // approx MK64 steering
    gHeadlessState.players[0].rotation[1] = yaw;
    if(inp.A) gHeadlessState.players[0].speed += 0.35f;
    if(inp.B) gHeadlessState.players[0].speed -= 0.5f;
    if(inp.R) gHeadlessState.players[0].speed *= 0.995f; // drift drag
    gHeadlessState.players[0].speed *= 0.998f; // friction
    if(gHeadlessState.players[0].speed > 12.0f) gHeadlessState.players[0].speed = 12.0f;
    if(gHeadlessState.players[0].speed < -4.0f) gHeadlessState.players[0].speed = -4.0f;
    float rad = yaw * 3.14159265f / 32768.0f;
    gHeadlessState.players[0].pos[0] += sinf(rad) * gHeadlessState.players[0].speed;
    gHeadlessState.players[0].pos[2] += cosf(rad) * gHeadlessState.players[0].speed;
    // lap mock: progress along Z
    if(gHeadlessState.players[0].pos[2] > 1000) gHeadlessState.players[0].lapCount++;
    gHeadlessState.frame++;
    gHeadlessState.globalTimer++;
    gHeadlessState.courseTimer += 0.016666f;
}
void headless_init_track(const char* track){
    memset(&gHeadlessState, 0, sizeof(gHeadlessState));
    gHeadlessState.players[0].pos[0]=0; gHeadlessState.players[0].pos[1]=0; gHeadlessState.players[0].pos[2]=0;
    gHeadlessState.players[0].rotation[1]=0;
    (void)track;
}
#else
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
    gMockKartStates[0].posX += inp.stick_x;
    gMockKartStates[0].posZ += inp.stick_y;
    if(inp.A) gMockKartStates[0].speed += 1;
    gMockFrame++;
}
#endif
