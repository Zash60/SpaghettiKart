#include "ScattershotReplay.h"
#include <fstream>
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
