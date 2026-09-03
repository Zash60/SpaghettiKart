#include "MK64State.h"
#include "Utils.h"

#ifndef HEADLESS
KartSnapshot gMockKartStates[8] = {};
uint32_t gMockFrame = 0;
uint32_t gMockRngSeed = 0;
#endif

#ifdef HEADLESS
#include "main.h"
#include "code_80005FD0.h"
extern "C" {
 extern s32 gGlobalTimer;
 extern float gCourseTimer;
}
MK64State save_state(){
    MK64State s;
    memcpy(s.players, gPlayers, sizeof(gPlayers));
    s.frame = gGlobalTimer;
    s.courseTimer = gCourseTimer;
    s.globalTimer = gGlobalTimer;
    return s;
}
void load_state(const MK64State& s){
    memcpy(gPlayers, s.players, sizeof(gPlayers));
    gGlobalTimer = s.globalTimer;
    gCourseTimer = s.courseTimer;
}
uint64_t hashPos(const MK64State& s){
    uint64_t seed = 0xCABBA6ECABBA6E;
    // pos is Vec3f float, scale down
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
    // Map MK64Input to N64 controller
    gControllers[0].rawStickX = inp.stick_x;
    gControllers[0].rawStickY = inp.stick_y;
    gControllers[0].button = 0;
    if(inp.A) gControllers[0].button |= 0x8000; // A_BUTTON
    if(inp.B) gControllers[0].button |= 0x4000; // B_BUTTON
    if(inp.R) gControllers[0].button |= 0x0010; // R_TRIG
    if(inp.Z) gControllers[0].button |= 0x0020; // Z_TRIG
    if(inp.L) gControllers[0].button |= 0x0020;
    // One frame tick: update player 0 (human) and global timers
    update_player(0);
    // update_vehicles and timers
    gCourseTimer += 0.016666f; // approx 60fps
    gGlobalTimer++;
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
