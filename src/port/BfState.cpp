#include "port/BfState.h"
#include <cstdint>
#include <cstring>

#include <macros.h>
#include <defines.h>
#include "common_structs.h"
#include "main.h"
#include "code_80005FD0.h"
#include "waypoints.h"

#define BF_MAGIC 0x00464253u // "BFS\0"

// Layout mirrors the real globals:
//   Player gPlayers[NUM_PLAYERS]            (src/main.c:73, NUM_PLAYERS=8)
//   struct Controller gControllers[NUM_PLAYERS] (src/main.c:63)
//   s32 gLapCountByPlayerId[10]             (src/code_80005FD0.c:173)
//   u16 gNearestPathPointByPlayerId[12]     (src/code_80005FD0.c:178)
//   u16 gPathIndexByPlayerId[12]            (src/code_80005FD0.c:195)
struct BfRaceState {
    uint32_t magic;
    Player players[8];
    struct Controller controllers[8];
    int32_t lapCount[10];
    uint16_t pathIndex[12];
    uint16_t nearestPath[12];
};

void BfState_Save(BfRaceState* out) {
    out->magic = BF_MAGIC;
    memcpy(out->players, gPlayers, sizeof(out->players));
    memcpy(out->controllers, gControllers, sizeof(out->controllers));
    memcpy(out->lapCount, gLapCountByPlayerId, sizeof(out->lapCount));
    memcpy(out->pathIndex, gPathIndexByPlayerId, sizeof(out->pathIndex));
    memcpy(out->nearestPath, gNearestPathPointByPlayerId, sizeof(out->nearestPath));
}

void BfState_Restore(const BfRaceState* in) {
    if (in->magic != BF_MAGIC) {
        return;
    }
    memcpy(gPlayers, in->players, sizeof(in->players));
    memcpy(gControllers, in->controllers, sizeof(in->controllers));
    memcpy(gLapCountByPlayerId, in->lapCount, sizeof(in->lapCount));
    memcpy(gPathIndexByPlayerId, in->pathIndex, sizeof(in->pathIndex));
    memcpy(gNearestPathPointByPlayerId, in->nearestPath, sizeof(in->nearestPath));
}

BfRaceState* BfState_Alloc(void) {
    return new BfRaceState();
}

void BfState_Free(BfRaceState* st) {
    delete st;
}
