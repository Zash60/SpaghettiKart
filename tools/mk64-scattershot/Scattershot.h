#pragma once
#include "MK64State.h"
#include <vector>
#include <mutex>
struct Segment { Segment* parent=nullptr; int depth=0; int numFrames=0; std::vector<MK64Input> inputs; MK64State endState; float score=1e9f; };
struct Block { uint64_t posHash=0; Segment* tailSeg=nullptr; int blockLength() const; };
struct GlobalState {
  int maxHashes=1<<20;
  std::vector<int> hashTab;
  std::vector<Block> blocks;
  std::vector<Segment*> segments;
  std::mutex mu;
  Block* bestBlock=nullptr;
  GlobalState(int maxH=1<<20);
  int findNewHashInx(uint64_t hashPos);
  int findBlock(uint64_t hashPos, int nMin, int nMax);
};
