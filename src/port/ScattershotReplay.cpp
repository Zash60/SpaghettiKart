#include "ScattershotReplay.h"
#include "main.h"
#include <libultraship.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <cstdio>
#include <cstdint>
struct MK64InputLocal { int8_t stick_x, stick_y; bool A,B,Z,R,L; };
std::vector<MK64Input> gScattershotReplay;
int gScattershotReplayIndex=0;
bool gScattershotActive=false;
bool LoadScattershotReplay(const std::string& path){
 std::ifstream f(path, std::ios::binary);
 if(!f) return false;
 uint32_t magic, ver, course, frames;
 f.read((char*)&magic,4); f.read((char*)&ver,4); f.read((char*)&course,4); f.read((char*)&frames,4);
 gScattershotReplay.clear(); gScattershotReplay.reserve(frames);
 for(uint32_t i=0;i<frames;i++){ int8_t sx,sy; uint8_t btn; f.read((char*)&sx,1); f.read((char*)&sy,1); f.read((char*)&btn,1); gScattershotReplay.push_back({sx,sy, bool(btn&1), bool(btn&2), bool(btn&8), bool(btn&4), bool(btn&16)}); }
 if(gScattershotReplay.empty()) return false;
 gScattershotReplayIndex=0;
 gScattershotActive=true;
 return true;
}
void DrawScattershotHud(){
 // stub: real impl will use ImGui to draw stick cross and timer
}
extern "C" void Scattershot_OverrideController(void){
 if(!gScattershotActive) return;
 if(gScattershotReplayIndex >= (int)gScattershotReplay.size()){ gScattershotActive=false; return; }
 const MK64Input& inp = gScattershotReplay[gScattershotReplayIndex++];
 gControllers[0].rawStickX = inp.stick_x;
 gControllers[0].rawStickY = inp.stick_y;
 gControllers[0].button = 0;
 if(inp.A) gControllers[0].button |= 0x8000;
 if(inp.B) gControllers[0].button |= 0x4000;
 if(inp.R) gControllers[0].button |= 0x0010;
 if(inp.Z) gControllers[0].button |= 0x0020;
 if(inp.L) gControllers[0].button |= 0x0020;
}
extern "C" int Scattershot_IsActive(void){ return gScattershotActive ? 1 : 0; }
extern "C" void Scattershot_Stop(void){ gScattershotActive=false; }
// Dumps the runtime collision mesh (Mario Raceway) for the headless bruteforcer.
// Format LE: u32 count, then per tri: 9x s16 xyz, u16 flags, u16 surface, 3x u16 vtxflags.
#include "racing/collision.h"
extern "C" int Scattershot_DumpCollision(const char* path){
 extern CollisionTriangle* gCollisionMesh;
 extern unsigned short gCollisionMeshCount;
 FILE* f = fopen(path, "wb");
 if(!f) return -1;
 uint32_t n = gCollisionMeshCount;
 fwrite(&n, 4, 1, f);
 for(uint32_t i = 0; i < n; i++){
  CollisionTriangle* t = &gCollisionMesh[i];
  int16_t v[9] = { t->vtx1->v.ob[0], t->vtx1->v.ob[1], t->vtx1->v.ob[2],
                   t->vtx2->v.ob[0], t->vtx2->v.ob[1], t->vtx2->v.ob[2],
                   t->vtx3->v.ob[0], t->vtx3->v.ob[1], t->vtx3->v.ob[2] };
  fwrite(v, 2, 9, f);
  fwrite(&t->flags, 2, 1, f);
  fwrite(&t->surfaceType, 2, 1, f);
  uint16_t vf[3] = { t->vtx1->v.flag, t->vtx2->v.flag, t->vtx3->v.flag };
  fwrite(vf, 2, 3, f);
 }
 fclose(f);
 return (int)n;
}
static bool TryLoadScattershotReplay(void){
 std::string appDir;
 try { appDir = Ship::Context::GetRawInstance()->GetAppDirectoryPath(); } catch (...) {}
 if (!appDir.empty()){
  SPDLOG_INFO("Scattershot: trying {}/best.mkr", appDir);
  if (LoadScattershotReplay(appDir + "/best.mkr")){ SPDLOG_INFO("Scattershot: loaded, frames={}", (int)gScattershotReplay.size()); return true; }
 }
 SPDLOG_INFO("Scattershot: trying /sdcard/Download/best.mkr");
 if (LoadScattershotReplay("/sdcard/Download/best.mkr")){ SPDLOG_INFO("Scattershot: loaded, frames={}", (int)gScattershotReplay.size()); return true; }
 SPDLOG_INFO("Scattershot: trying out/best.mkr");
 if (LoadScattershotReplay("out/best.mkr")){ SPDLOG_INFO("Scattershot: loaded fallback, frames={}", (int)gScattershotReplay.size()); return true; }
 SPDLOG_WARN("Scattershot: no replay file found (put best.mkr next to config.yml)");
 return false;
}
bool Scattershot_TryLoad(void){ return TryLoadScattershotReplay(); }
extern "C" void Scattershot_AutoLoad(void){
 SPDLOG_INFO("Scattershot AutoLoad: go! (autoload cvar={}, timer={})", CVarGetInteger("gScattershotAutoLoad", 1), gGlobalTimer);
 if (CVarGetInteger("gScattershotAutoLoad", 1) == 0) return;
 if (gScattershotActive) return;
 TryLoadScattershotReplay();
}
