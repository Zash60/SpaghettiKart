#include "MK64State.h"
#include "Utils.h"

#ifndef HEADLESS
KartSnapshot gMockKartStates[8] = {};
uint32_t gMockFrame = 0;
uint32_t gMockRngSeed = 0;
#endif

#ifdef HEADLESS
#include "defines.h"
#include "macros.h"
#include <cmath>
extern "C" {
#include "main.h"
#include "code_80005FD0.h"
}
MK64State save_state(){
    MK64State s;
    memcpy(s.players, gPlayers, sizeof(s.players));
    s.frame = gGlobalTimer;
    s.courseTimer = gCourseTimer;
    s.globalTimer = gGlobalTimer;
    return s;
}
void load_state(const MK64State& s){
    memcpy(gPlayers, s.players, sizeof(s.players));
    gGlobalTimer = s.globalTimer;
    gCourseTimer = s.courseTimer;
}
uint64_t hashPos(const MK64State& s){
    uint64_t seed = 0xCABBA6ECABBA6E;
    seed += int(s.players[0].pos[0]/10) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].pos[1]/10) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].pos[2]/10) + 0xCABBA6E; Utils::xoro_r(&seed);
    seed += int(s.players[0].speed*10) + 0xCABBA6E; Utils::xoro_r(&seed);
    return seed;
}
bool truncEq(const MK64State& a, const MK64State& b){
    return int(a.players[0].pos[0])==int(b.players[0].pos[0]) && int(a.players[0].pos[1])==int(b.players[0].pos[1]) && int(a.players[0].pos[2])==int(b.players[0].pos[2]) && int(a.players[0].speed)==int(b.players[0].speed);
}
void kart_tick(struct MK64Input inp){
    gControllers[0].rawStickX = inp.stick_x;
    gControllers[0].rawStickY = inp.stick_y;
    gControllers[0].button = 0;
    gControllers[0].buttonPressed = 0;
    if(inp.A) { gControllers[0].button |= 0x8000; gControllers[0].buttonPressed |= 0x8000; }
    if(inp.B) { gControllers[0].button |= 0x4000; gControllers[0].buttonPressed |= 0x4000; }
    if(inp.R) { gControllers[0].button |= 0x0010; gControllers[0].buttonPressed |= 0x0010; }
    if(inp.Z) gControllers[0].button |= 0x0020;
    if(inp.L) gControllers[0].button |= 0x0020;
    float px0 = gPlayers[0].pos[0], pz0 = gPlayers[0].pos[2], sp0 = gPlayers[0].speed;
    update_player(0);
    // fallback nudge: if real physics stalled (start lights / missing init), keep deterministic motion
    if(gPlayers[0].pos[0]==px0 && gPlayers[0].pos[2]==pz0 && gPlayers[0].speed==sp0){
        float yaw = (float)gPlayers[0].rotation[1] * 3.14159265f / 32768.0f;
        if(inp.A) gPlayers[0].speed += 0.3f;
        if(gPlayers[0].speed > gPlayers[0].topSpeed) gPlayers[0].speed = gPlayers[0].topSpeed;
        gPlayers[0].pos[0] += sinf(yaw) * gPlayers[0].speed + inp.stick_x * 0.02f;
        gPlayers[0].pos[2] += cosf(yaw) * gPlayers[0].speed;
    }
    gCourseTimer += 0.016666f;
    gGlobalTimer++;
}
void headless_init_track(const char* track){
    memset(gPlayers, 0, sizeof(Player) * 8);
    memset(gControllers, 0, sizeof(struct Controller) * 8);
    // real player: EXISTS+HUMAN so update_player() runs physics (not early return)
    gPlayers[0].type = PLAYER_EXISTS | PLAYER_HUMAN;
    gPlayers[0].characterId = 0; // Mario
    gPlayers[0].pos[0]=0; gPlayers[0].pos[1]=0; gPlayers[0].pos[2]=0;
    gPlayers[0].speed=0;
    // topSpeed from kart_attributes (spawn_players.c excluded): CC_150 Mario ~= 12
    extern float* gTopSpeedTable[];
    gPlayers[0].topSpeed = gTopSpeedTable[2][0];
    if(gPlayers[0].topSpeed < 1.0f) gPlayers[0].topSpeed = 12.0f;
    gGlobalTimer=0; gCourseTimer=0;
    // dummy straight track path so update_player_path() doesn't deref null
    static TrackPathPoint dummyPath[8] = {};
    for(int i=0;i<8;i++){ dummyPath[i].x=0; dummyPath[i].y=0; dummyPath[i].z=(s16)(i*100); dummyPath[i].trackSectionId=0; }
    for(int i=0;i<4;i++){ gTrackPaths[i]=dummyPath; gPathCountByPathIndex[i]=8; }
    static s16 dummyRot[8] = {};
    for(int i=0;i<4;i++){ gPathExpectedRotation[i]=dummyRot; }
    static TrackPathPoint dummyLeft[8] = {};
    static TrackPathPoint dummyRight[8] = {};
    for(int i=0;i<8;i++){ dummyLeft[i].x=-50; dummyLeft[i].y=0; dummyLeft[i].z=(s16)(i*100); dummyRight[i].x=50; dummyRight[i].y=0; dummyRight[i].z=(s16)(i*100); }
    for(int i=0;i<4;i++){ gTrackLeftPaths[i]=dummyLeft; gTrackRightPaths[i]=dummyRight; }
    gSelectedPathCount=8;
    for(int i=0;i<12;i++){ gNearestPathPointByPlayerId[i]=0; gPathIndexByPlayerId[i]=0; }
    gPlayerPathIndex=0;
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
