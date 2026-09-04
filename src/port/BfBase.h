#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct BfInput {
    int16_t stickX;
    int16_t stickY;
    uint16_t button;
    uint16_t buttonPressed;
    uint16_t buttonDepressed;
};

struct BfRaceState;

void Bf_RecordStart(void);
void Bf_RecordStop(void);
void Bf_RecordTick(void);
int Bf_IsRecording(void);
const BfInput* Bf_BaseData(void);
int Bf_BaseLen(void);
void Bf_SetBase(const BfInput* d, int n);
void Bf_ClearBase(void);
// Search-root snapshot taken at RecordStart: candidates always replay the
// full base from THIS state (base[0] == record moment by construction).
// Returns NULL when no valid root exists.
struct BfRaceState* Bf_RootState(void);
// Same-race guard: 0 = current race matches the recording session.
int Bf_RootSessionOk(void);
// Re-anchor the search root at the current state (use after bf_loadresult
// at race start to keep optimizing a loaded base).
void Bf_Reanchor(void);

#ifdef __cplusplus
}
#endif
