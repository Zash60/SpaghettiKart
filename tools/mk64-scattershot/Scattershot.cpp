#include "Scattershot.h"
#include "Utils.h"
GlobalState::GlobalState(int maxH): maxHashes(maxH), hashTab(maxH, -1) { blocks.reserve(maxH); segments.reserve(maxH); }
int GlobalState::findNewHashInx(uint64_t hashPos){
 uint64_t tmp=hashPos;
 for(int i=0;i<100;i++){ int inx=tmp%maxHashes; if(hashTab[inx]==-1) return inx; Utils::xoro_r(&tmp); }
 return -1;
}
int GlobalState::findBlock(uint64_t hashPos, int nMin, int nMax){
 uint64_t tmp=hashPos;
 for(int i=0;i<100;i++){ int inx=tmp%maxHashes; int b=hashTab[inx]; if(b==-1) return nMax; if(b>=nMin && b<nMax){ if(blocks[b].posHash==hashPos) return b; } Utils::xoro_r(&tmp); }
 return -1;
}
int Block::blockLength() const {
 int len=0; Segment* cur=tailSeg; while(cur){ len+=cur->numFrames; cur=cur->parent; } return len;
}
