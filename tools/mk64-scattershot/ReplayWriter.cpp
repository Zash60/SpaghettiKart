#include "ReplayWriter.h"
#include <fstream>
#include <algorithm>
std::vector<MK64Input> collectInputs(Block* best){
 std::vector<MK64Input> out;
 if(!best || !best->tailSeg) return out;
 Segment* cur=best->tailSeg;
 std::vector<Segment*> chain;
 while(cur){ chain.push_back(cur); cur=cur->parent; }
 std::reverse(chain.begin(), chain.end());
 for(auto s: chain) out.insert(out.end(), s->inputs.begin(), s->inputs.end());
 return out;
}
void writeMkr(const std::vector<MK64Input>& replay, const std::string& path){
 std::ofstream f(path, std::ios::binary);
 uint32_t magic=0x4D4B5230, ver=1, frames=replay.size();
 uint32_t course=0; // mario_raceway
 f.write((char*)&magic,sizeof(magic));
 f.write((char*)&ver,sizeof(ver));
 f.write((char*)&course,sizeof(course));
 f.write((char*)&frames,sizeof(frames));
 for(auto &i: replay){
  int8_t sx=i.stick_x, sy=i.stick_y;
  uint8_t btn = (i.A?1:0)|(i.B?2:0)|(i.R?4:0)|(i.Z?8:0)|(i.L?16:0);
  f.write((char*)&sx,1); f.write((char*)&sy,1); f.write((char*)&btn,1);
 }
}
std::vector<MK64Input> readMkr(const std::string& path){
 std::ifstream f(path, std::ios::binary);
 if(!f) return {};
 uint32_t magic, ver, course, frames;
 f.read((char*)&magic,4); f.read((char*)&ver,4); f.read((char*)&course,4); f.read((char*)&frames,4);
 std::vector<MK64Input> out; out.reserve(frames);
 for(uint32_t i=0;i<frames;i++){ int8_t sx,sy; uint8_t btn; f.read((char*)&sx,1); f.read((char*)&sy,1); f.read((char*)&btn,1); out.push_back({sx,sy, bool(btn&1), bool(btn&2), bool(btn&8), bool(btn&4), bool(btn&16)}); }
 return out;
}
void writeJson(const GlobalState& g, int totalFrames, const std::string& path, int characterId, float topSpeed, const std::string& course){
 std::ofstream f(path);
 int bestFrames = g.bestBlock ? g.bestBlock->blockLength() : totalFrames;
 const char* names[8] = {"Mario","Luigi","Yoshi","Toad","DK","Wario","Peach","Bowser"};
 const char* cname = (characterId>=0 && characterId<8) ? names[characterId] : "Unknown";
 f << "{\"frames\":"<<bestFrames<<",\"totalBlocks\":"<<g.blocks.size()<<",\"bestExists\":"<<(g.bestBlock? "true":"false")
   <<",\"course\":\""<<course<<"\",\"character\":\""<<cname<<"\",\"characterId\":"<<characterId
   <<",\"cc\":150,\"topSpeed\":"<<topSpeed<<"}";
}
