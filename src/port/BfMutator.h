#pragma once

void Bf_SearchStart(int windowFrames, int maxIters);
void Bf_SearchStop(void);
void Bf_SearchFrame(void);
int Bf_SaveResult(const char* path);
int Bf_LoadResult(const char* path);
const char* Bf_ResultPath(void);
