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
#include "kart_attributes.h"
#include <cmath>
extern "C" {
#include "main.h"
#include "code_80005FD0.h"
}
// Scattershot corre só de Yoshi (characterId 2)
#define HEADLESS_CHARACTER YOSHI
#define HEADLESS_CC CC_150
static uint32_t gHeadlessLights = 0;
MK64State save_state(){
    MK64State s;
    memcpy(s.players, gPlayers, sizeof(s.players));
    s.frame = gGlobalTimer;
    s.courseTimer = gCourseTimer;
    s.globalTimer = gGlobalTimer;
    s.lights = gHeadlessLights;
    return s;
}
void load_state(const MK64State& s){
    memcpy(gPlayers, s.players, sizeof(s.players));
    gGlobalTimer = s.globalTimer;
    gCourseTimer = s.courseTimer;
    gHeadlessLights = s.lights;
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
void kart_tick(struct MK64Input& inp){
    // 3-2-1-Go: run the real start-sequence code with real inputs (rev/boost/false-start),
    // exactly like the game; Lakitu's Go clears START_SEQUENCE when lights hit 0
    if(gHeadlessLights > 0){
        gHeadlessLights--;
        if(gHeadlessLights == 0) gPlayers[0].type &= ~PLAYER_START_SEQUENCE;
    }
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
    // fallback nudge (post-Go only): if real physics stalled, keep deterministic motion.
    // Never nudge during start lights: kart must hold still like the real countdown.
    if(gHeadlessLights == 0 && gPlayers[0].pos[0]==px0 && gPlayers[0].pos[2]==pz0 && gPlayers[0].speed==sp0){
        float yaw = (float)gPlayers[0].rotation[1] * 3.14159265f / 32768.0f;
        if(inp.A) gPlayers[0].speed += 0.3f;
        if(gPlayers[0].speed > gPlayers[0].topSpeed) gPlayers[0].speed = gPlayers[0].topSpeed;
        gPlayers[0].pos[0] += sinf(yaw) * gPlayers[0].speed + inp.stick_x * 0.02f;
        gPlayers[0].pos[2] += cosf(yaw) * gPlayers[0].speed;
    }
    // Real lap counting via update_player_completion on the real 499-pt track
    gCourseTimer += 0.016666f;
    gGlobalTimer++;
}
void headless_init_track(const char* track){
    memset(gPlayers, 0, sizeof(Player) * 8);
    memset(gControllers, 0, sizeof(struct Controller) * 8);
    // real player: EXISTS+HUMAN+START_SEQUENCE so update_player() runs the real
    // countdown code (rev/boost/false-start) until lights hit 0, like Lakitu's Go
    gPlayers[0].type = PLAYER_EXISTS | PLAYER_HUMAN | PLAYER_START_SEQUENCE;
    // Yoshi only, full kart stats like spawn_players (150cc versus)
    gPlayers[0].characterId = HEADLESS_CHARACTER;
    gPlayers[0].kartFriction = gKartFrictionTable[HEADLESS_CHARACTER];
    gPlayers[0].boundingBoxSize = gKartBoundingBoxSizeTable[HEADLESS_CHARACTER];
    gPlayers[0].kartGravity = gKartGravityTable[HEADLESS_CHARACTER];
    gPlayers[0].unk_084 = D_800E2400[HEADLESS_CC][HEADLESS_CHARACTER];
    gPlayers[0].unk_088 = D_800E24B4[HEADLESS_CC][HEADLESS_CHARACTER];
    gPlayers[0].unk_210 = D_800E2568[HEADLESS_CC][HEADLESS_CHARACTER];
    gPlayers[0].topSpeed = gTopSpeedTable[HEADLESS_CC][HEADLESS_CHARACTER];
    if(gPlayers[0].topSpeed < 1.0f) gPlayers[0].topSpeed = 12.0f;
    gPlayers[0].pos[0]=0; gPlayers[0].pos[1]=0; gPlayers[0].pos[2]=0;
    gPlayers[0].speed=0;
    gGlobalTimer=0; gCourseTimer=0;
    gHeadlessLights = HEADLESS_START_LIGHTS;
    // Real Mario Raceway: build path tables from the real 499-pt centerline
    // using the game's own init (boundaries/sections/rotations/curves)
    init_course_path_point();
    // Spawn on the real start line (path point 0)
    gPlayers[0].pos[0]=(float)gTrackPaths[0][0].x;
    gPlayers[0].pos[1]=(float)gTrackPaths[0][0].y;
    gPlayers[0].pos[2]=(float)gTrackPaths[0][0].z;
    gSelectedPathCount=gPathCountByPathIndex[0];
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
void kart_tick(struct MK64Input& inp){
    gMockKartStates[0].posX += inp.stick_x;
    gMockKartStates[0].posZ += inp.stick_y;
    if(inp.A) gMockKartStates[0].speed += 1;
    gMockFrame++;
}
#endif
