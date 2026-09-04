#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void Bf_SearchStart(int windowFrames, int maxIters);
void Bf_SearchStop(void);
void Bf_SearchFrame(void);
int Bf_SaveResult(const char* path);
int Bf_LoadResult(const char* path);
const char* Bf_ResultPath(void);
// Session ghost slots (RAM only, no pak): BF best <-> player ghost.
int Bf_ExportGhost(void);    // 0 ok, -1 no base / too long
void Bf_RestorePlayerGhost(void);
// Tunable search params (TMInterface-style: no universal settings).
// Names: "prob" (0.05-1.0), "steer" (1-40), "minframe", "maxframe" (-1 = end),
// "speed" (1-64 ticks/frame).
void Bf_ParamsReset(void);
int Bf_ParamSet(const char* name, float value); // 0 ok, -1 unknown
float Bf_ParamGet(const char* name);
int Bf_SimSpeed(void);
// Full reset: stops everything, clears base/root/best.
void Bf_Reset(void);
int Bf_IsSearching(void);
int Bf_SearchIter(void);
int Bf_BestTicks(void);

#ifdef __cplusplus
}
#endif
