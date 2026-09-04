// Headless stubs for real update_player() with whitelist minimal lib.
// SpaghettifyHeadless = code_80005FD0 + code_800029B0 + buffers + data only.
#include <cstdint>
#include <cstring>

#ifdef HEADLESS

#include "engine/tracks/Track.h"
#include "MarioRacewayPath.inc"

// Bump allocator for get_next_available_memory_addr (racing/memory.c excluded)
static uint8_t gHeadlessPool[4*1024*1024] = {};
static size_t gHeadlessPoolOff = 0;

static Properties gHeadlessProps{};
static bool gHeadlessPropsInit = false;

static Properties* HeadlessProps() {
    if (!gHeadlessPropsInit) {
        memset(&gHeadlessProps, 0, sizeof(gHeadlessProps));
        // Real Mario Raceway props (src/engine/tracks/MarioRaceway.cpp)
        gHeadlessProps.AIMaximumSeparation = 50.0f;
        gHeadlessProps.AIMinimumSeparation = 0.3f;
        gHeadlessProps.PathSizes.unk0 = 499;
        gHeadlessProps.PathSizes.unk2 = 1;
        gHeadlessProps.PathSizes.unk4 = 1;
        gHeadlessProps.PathSizes.unk6 = 1;
        gHeadlessProps.PathSizes.unk8 = 1;
        gHeadlessProps.PathTable2[0] = (TrackPathPoint*)gMarioRacewayPathSrc;
        gHeadlessProps.PathTable2[1] = nullptr;
        gHeadlessProps.PathTable2[2] = nullptr;
        gHeadlessProps.PathTable2[3] = nullptr;
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
#include "main.h"
#include "camera.h"
#include "racing/actors_extended.h"
#include "effects.h"
#include "render_player.h"
struct Actor;
struct FakeItemBox;
struct BananaBunchParent;

// --- globals from main.c/menus.c (excluded from whitelist) ---
s8 gPlayerCount = 1;
s8 gCharacterSelections[8] = {0, 1, 2, 3, 4, 5, 6, 7};
s8 gDemoUseController = 0;
bool gTrackMapInit = false;
Player gPlayers[8] = {};
Player* gPlayerOne = &gPlayers[0];
Player* gPlayerTwo = &gPlayers[1];
Player* gPlayerThree = &gPlayers[2];
Player* gPlayerFour = &gPlayers[3];
Player* gPlayerFive = &gPlayers[4];
Player* gPlayerSix = &gPlayers[5];
Player* gPlayerSeven = &gPlayers[6];
Player* gPlayerEight = &gPlayers[7];
struct Controller gControllers[8] = {};
struct Controller* gControllerOne = &gControllers[0];
struct Controller* gControllerThree = &gControllers[2];
s32 gGlobalTimer = 0;
f32 gCourseTimer = 0.0f;
s32 gActiveScreenMode = 0;
u16 gDemoMode = 0;
Camera* camera1 = nullptr;
Camera* camera2 = nullptr;
s32 gModeSelection = 0;
s32 gCCSelection = 2; // CC_150
bool gIsPlayerTripleAButtonCombo[8] = {};
CollisionGrid gCollisionGrid[1024] = {}; // main.c sizing: 32x32 grid
static uint16_t gHeadlessCollisionIndices[16384] = {};

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

// --- audio (src/audio, excluded) signatures from src/audio/external.h ---
void func_800C9060(u8 a, u32 b) { (void)a;(void)b; }
void func_800C90F4(u8 a, u32 b) { (void)a;(void)b; }
void func_800C92CC(u8 a, u32 b) { (void)a;(void)b; }
void func_800C98B8(Vec3f a, Vec3f b, u32 c) { (void)a;(void)b;(void)c; }
void func_800C9D80(Vec3f a, Vec3f b, u32 c) { (void)a;(void)b;(void)c; }
void func_800C9EF4(Vec3f a, u32 b) { (void)a;(void)b; }
void func_800CA270(void) {}
void func_800CAC60(s32 a) { (void)a; }
void func_800CAD40(s32 a) { (void)a; }
void func_800CA008(u8 a, u8 b) { (void)a;(void)b; }
void func_800CA0A0(void) {}
void func_800CA0B8(void) {}
void func_800CA0CC(void) {}
void func_800CB134(void) {}
void func_800CB14C(void) {}
void play_sound2(s32 a) { (void)a; }
void play_sequence2(u16 a) { (void)a; }
void func_80092C80(void) {}
void func_8009265C(void) {}
void func_8009E5BC(void) {}
void func_8009E088(s32 a, s32 b) { (void)a;(void)b; }

void* get_next_available_memory_addr(uintptr_t size){
    size = (size + 15) & ~(uintptr_t)15;
    void* p = &gHeadlessPool[gHeadlessPoolOff];
    gHeadlessPoolOff += size;
    return p;
}
// Passthrough (engine/RaceManager.cpp excluded; headless never mirrors)
void add_triangle_to_collision_mesh(Vtx* a, Vtx* b, Vtx* c, Vtx** o1, Vtx** o2, Vtx** o3){
    *o1=a; *o2=b; *o3=c;
}

#include "MarioRacewayCollision.inc"

// Real collision pools (mesh data dumped in-game from Mario Raceway)
static Vtx gHeadlessVtx[1501*3] = {};
static CollisionTriangle gHeadlessMesh[1536] = {};
static bool gHeadlessColBuilt = false;

extern "C" {
#include "racing/collision.h"
void add_collision_triangle(Vtx* vtx1, Vtx* vtx2, Vtx* vtx3, int8_t surfaceType, uint16_t sectionId);
void Scattershot_BuildCollision(void){
    if(gHeadlessColBuilt) return;
    gHeadlessColBuilt = true;
    extern CollisionTriangle* gCollisionMesh;
    extern uint16_t* gCollisionIndices;
    extern u16 gCollisionMeshCount;
    // Rebuild the real mesh with the game's own function (identical semantics)
    gCollisionMesh = gHeadlessMesh;
    gCollisionMeshCount = 0;
    for(int i = 0; i < kHeadlessColCount; i++){
        Vtx* v1 = &gHeadlessVtx[i*3+0];
        Vtx* v2 = &gHeadlessVtx[i*3+1];
        Vtx* v3 = &gHeadlessVtx[i*3+2];
        const int16_t* c = kHeadlessColVtx[i];
        v1->v.ob[0]=c[0]; v1->v.ob[1]=c[1]; v1->v.ob[2]=c[2];
        v2->v.ob[0]=c[3]; v2->v.ob[1]=c[4]; v2->v.ob[2]=c[5];
        v3->v.ob[0]=c[6]; v3->v.ob[1]=c[7]; v3->v.ob[2]=c[8];
        const uint16_t* m = kHeadlessColMeta[i];
        v1->v.flag=m[2]; v2->v.flag=m[3]; v3->v.flag=m[4];
        add_collision_triangle(v1, v2, v3, (int8_t)m[1], (uint16_t)(m[0] & 0xFF));
    }
    gCollisionIndices = gHeadlessCollisionIndices;
    generate_collision_grid();
}
} // extern C

// --- misc port/engine signatures from src/port/Game.h + include/mk64.h ---
bool GameEngine_OTRSigCheck(const char* p) { (void)p; return false; }
void FrameInterpolation_DontInterpolateCamera(void) {}
int OTRGetGameRenderWidth(void) { return 320; }
int OTRGetRectDimensionFromLeftEdge(int v) { (void)v; return 0; }
void CM_DrawObjects(s32 cameraId) { (void)cameraId; }
void CM_DrawParticles(s32 cameraId) { (void)cameraId; }
void CM_DrawTrackObjects(s32 cameraId) { (void)cameraId; }

// --- items (src/racing/actors_extended, excluded deps) ---
s32 use_green_shell_item(Player* p) { (void)p; return 0; }
s32 use_red_shell_item(Player* p) { (void)p; return 0; }
s32 use_blue_shell_item(Player* p) { (void)p; return 0; }
s32 use_triple_shell_item(Player* p, s16 a) { (void)p;(void)a; return 0; }
s32 use_banana_item(Player* p) { (void)p; return 0; }
s32 use_banana_bunch_item(Player* p) { (void)p; return 0; }
void use_thunder_item(Player* p) { (void)p; }
s32 use_fake_itembox_item(Player* p) { (void)p; return 0; }
void drop_banana_in_banana_bunch(struct BananaBunchParent* p) { (void)p; }
void func_802A1064(struct FakeItemBox* p) { (void)p; }
void func_8008F104(Player* p, s8 a) { (void)p;(void)a; }
void move_s32_towards(s32* a, s32 b, f32 c) { (void)a;(void)b;(void)c; if(a) *a=b; }

} // extern "C"

#endif
