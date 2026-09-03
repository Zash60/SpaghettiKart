#pragma once
#include <string>
#include <vector>
#include <cstdint>
struct MK64Input { int8_t stick_x=0, stick_y=0; bool A=false,B=false,Z=false,R=false,L=false; };
bool LoadScattershotReplay(const std::string& path);
void DrawScattershotHud();
extern std::vector<MK64Input> gScattershotReplay;
extern int gScattershotReplayIndex;
extern bool gScattershotActive;
#ifdef __cplusplus
extern "C" {
#endif
void Scattershot_OverrideController(void);
int Scattershot_IsActive(void);
void Scattershot_Stop(void);
void Scattershot_AutoLoad(void);
#ifdef __cplusplus
}
#endif
