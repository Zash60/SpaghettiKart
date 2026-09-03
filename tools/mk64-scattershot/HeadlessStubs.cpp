// Headless stubs for real update_player() without src/port + src/engine.
// Provides CM_* / Is* so SpaghettifyHeadless (minimal: no port/engine) links
// and update_player() runs the real MK64 code path.
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
struct Actor;

// --- CM_* used by code_80005FD0.c (signatures must match src/port/Game.h) ---
Properties* CM_GetProps() { return HeadlessProps(); }
void CM_VehicleCollision(s32 playerId, Player* player) { (void)playerId; (void)player; }
void CM_CrossingTrigger() {}
void CM_AICrossingBehaviour(s32 playerId) { (void)playerId; }
void CM_ClearVehicles() {}
size_t CM_FindActorIndex(struct Actor* actor) { (void)actor; return (size_t)-1; }
struct Actor* CM_GetActor(size_t index) { (void)index; return nullptr; }
void CM_TickBombKarts() {}
void CM_VehiclesTick() {}

// --- Is* track checks (port/Game.cpp) : headless = Mario Raceway generic, all false ---
bool IsYoshiValley() { return false; }
bool IsToadsTurnpike() { return false; }
bool IsPodiumCeremony() { return false; }
bool IsBowsersCastle() { return false; }
bool IsDkJungle() { return false; }
bool IsFrappeSnowland() { return false; }
bool IsKalimariDesert() { return false; }
bool IsLuigiRaceway() { return false; }
bool IsRainbowRoad() { return false; }
bool IsWarioStadium() { return false; }
bool IsKoopaTroopaBeach() { return false; }
bool IsRoyalRaceway() { return false; }
bool IsSkyscraper() { return false; }

} // extern "C"

#endif
