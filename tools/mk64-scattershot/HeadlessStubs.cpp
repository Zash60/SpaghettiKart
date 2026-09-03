// Headless stubs for real update_player() with whitelist minimal lib.
// SpaghettifyHeadless = code_80005FD0 + code_800029B0 + buffers + data only.
#include <cstdint>
#include <cstring>

#ifdef HEADLESS

#include "engine/tracks/Track.h"

static Properties gHeadlessProps{};
static bool gHeadlessPropsInit = false;

static Properties* HeadlessProps() {
    if (!gHeadlessPropsInit) {
        memset(&gHeadlessProps, 0, sizeof(gHeadlessProps));
        gHeadlessProps.AIMaximumSeparation = 0.0f;
        gHeadlessProps.AIMinimumSeparation = 0.0f;
        gHeadlessProps.NormalTargetSpeed[0] = 8.0f;
        gHeadlessProps.NormalTargetSpeed[1] = 8.0f;
        gHeadlessProps.NormalTargetSpeed[2] = 8.0f;
        gHeadlessProps.NormalTargetSpeed[3] = 8.0f;
        gHeadlessProps.CurveTargetSpeed[0] = 6.0f;
        gHeadlessProps.CurveTargetSpeed[1] = 6.0f;
        gHeadlessProps.CurveTargetSpeed[2] = 6.0f;
        gHeadlessProps.CurveTargetSpeed[3] = 6.0f;
        gHeadlessProps.OffTrackTargetSpeed[0] = 4.0f;
        gHeadlessProps.OffTrackTargetSpeed[1] = 4.0f;
        gHeadlessProps.OffTrackTargetSpeed[2] = 4.0f;
        gHeadlessProps.OffTrackTargetSpeed[3] = 4.0f;
        gHeadlessProps.D_0D0096B8[0] = 8.0f;
        gHeadlessProps.D_0D0096B8[1] = 8.0f;
        gHeadlessProps.D_0D0096B8[2] = 8.0f;
        gHeadlessProps.D_0D0096B8[3] = 8.0f;
        gHeadlessPropsInit = true;
    }
    return &gHeadlessProps;
}

extern "C" {
#include "common_structs.h"
#include "audio/external.h"
#include "menus.h"
struct Actor;

// --- globals from menus.c (excluded) ---
s8 gPlayerCount = 1;
s8 gCharacterSelections[8] = {0, 1, 2, 3, 4, 5, 6, 7};
s8 gDemoUseController = 0;
bool gTrackMapInit = false;

// --- CM_* (src/port/Game.cpp, excluded) ---
Properties* CM_GetProps() { return HeadlessProps(); }
void CM_VehicleCollision(s32 playerId, Player* player) { (void)playerId; (void)player; }
void CM_CrossingTrigger() {}
void CM_AICrossingBehaviour(s32 playerId) { (void)playerId; }
void CM_ClearVehicles() {}
size_t CM_FindActorIndex(struct Actor* actor) { (void)actor; return (size_t)-1; }
struct Actor* CM_GetActor(size_t index) { (void)index; return nullptr; }
void CM_TickBombKarts() {}
void CM_VehiclesTick() {}
void CM_CleanWorld() {}
void LoadTrack() {}
void Editor_SetLevelDimensions(s16 a, s16 b, s16 c, s16 d, s16 e, s16 f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; }
void SelectPodiumCeremony() {}
size_t GetCupCursorPosition() { return 0; }
int16_t RaceManager_GetRandomCPUItem(uint32_t rank) { (void)rank; return -1; }

// --- Is* (src/port/Game.cpp) ---
bool IsYoshiValley() { return false; }
bool IsToadsTurnpike() { return false; }
bool IsPodiumCeremony() { return false; }
bool IsBowsersCastle() { return false; }
bool IsDkJungle() { return false; }
bool IsFrappeSnowland() { return false; }
bool IsKalimariDesert() { return false; }
bool IsLuigiRaceway() { return false; }
bool IsMarioRaceway() { return true; }
bool IsRainbowRoad() { return false; }
bool IsWarioStadium() { return false; }
bool IsKoopaTroopaBeach() { return false; }
bool IsRoyalRaceway() { return false; }
bool IsSkyscraper() { return false; }
bool IsChocoMountain() { return false; }
bool IsBansheeBoardwalk() { return false; }

// --- audio (src/audio, excluded) ---
void func_800C9060(u8 a, u32 b) { (void)a;(void)b; }
void func_800C90F4(u8 a, u32 b) { (void)a;(void)b; }
void func_800C92CC(u8 a, u32 b) { (void)a;(void)b; }
void func_800C98B8(Vec3f a, Vec3f b, u32 c) { (void)a;(void)b;(void)c; }
void func_800C9D80(Vec3f a, Vec3f b, u32 c) { (void)a;(void)b;(void)c; }
void func_800C9EF4(Vec3f a, u32 b) { (void)a;(void)b; }
void func_800CA270(void) {}
void func_800CAC60(s32 a) { (void)a; }
void func_800CAD40(s32 a) { (void)a; }
void func_800CA008(s32 a) { (void)a; }
void func_800CA0A0(s32 a) { (void)a; }
void func_800CA0B8(s32 a) { (void)a; }
void func_800CA0CC(s32 a) { (void)a; }
void func_800CB134(s32 a) { (void)a; }
void func_800CB14C(s32 a) { (void)a; }
void play_sound2(u8 a, u32 b) { (void)a;(void)b; }
void play_sequence2(u8 a) { (void)a; }
void func_80092C80(void) {}
void func_8009265C(void) {}
void func_8009E5BC(void) {}
void func_8009E088(void) {}

// --- misc port/engine ---
bool GameEngine_OTRSigCheck(const char* p) { (void)p; return false; }
void FrameInterpolation_DontInterpolateCamera(void) {}
int OTRGetGameRenderWidth(void) { return 320; }
int OTRGetRectDimensionFromLeftEdge(int v) { (void)v; return 0; }
void gSPDisplayList(void* a, void* b) { (void)a;(void)b; }
void gSPVertex(void* a, int b, int c) { (void)a;(void)b;(void)c; }
void CM_DrawObjects(void* a) { (void)a; }
void CM_DrawParticles(void* a) { (void)a; }
void CM_DrawTrackObjects(void* a) { (void)a; }

} // extern "C"

#endif
