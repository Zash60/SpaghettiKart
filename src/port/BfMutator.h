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

#ifdef __cplusplus
}
#endif
