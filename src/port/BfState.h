#pragma once

// Opaque snapshot of everything BfSim needs to rewind the race.
struct BfRaceState;

void BfState_Save(BfRaceState* out);
void BfState_Restore(const BfRaceState* in);
BfRaceState* BfState_Alloc(void);
void BfState_Free(BfRaceState* st);
