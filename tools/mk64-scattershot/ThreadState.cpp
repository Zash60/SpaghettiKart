#include "ThreadState.h"
#include "InputGen.h"
#include "Evaluator.h"
void ThreadState::scatter(int depth){
 if(!g || g->blocks.empty()) return;
 // pick random block as seed
 Utils::xoro_r(&seed);
 int idx = seed % g->blocks.size();
 Block &blk = g->blocks[idx];
 load_state(blk.tailSeg->endState);
 std::vector<MK64Input> curInputs;
 curInputs.reserve(depth);
 for(int i=0;i<depth;i++){
   auto inp = InputGen::random(seed);
   kart_tick(inp);
   curInputs.push_back(inp);
 }
 MK64State cur = save_state();
 float sc = Evaluator::score(cur, cur.frame);
 uint64_t h = hashPos(cur);
 std::lock_guard<std::mutex> lk(g->mu);
 int bInx = g->findBlock(h, 0, (int)g->blocks.size());
 if(bInx >= (int)g->blocks.size() || bInx==-1){
   int hInx = g->findNewHashInx(h);
   if(hInx==-1) return;
   Segment* seg = new Segment{blk.tailSeg, blk.tailSeg->depth+1, depth, curInputs, cur, sc};
   g->segments.push_back(seg);
   Block nb{h, seg};
   g->hashTab[hInx] = (int)g->blocks.size();
   g->blocks.push_back(nb);
   if(!g->bestBlock || sc < g->bestBlock->tailSeg->score){
     if(Evaluator::isFinished(cur)) g->bestBlock = &g->blocks.back();
   }
 } else {
   Block &exist = g->blocks[bInx];
   if(sc < exist.tailSeg->score){
     Segment* seg = new Segment{exist.tailSeg->parent, exist.tailSeg->depth+1, depth, curInputs, cur, sc};
     g->segments.push_back(seg);
     exist.tailSeg = seg;
     exist.posHash = h;
   }
 }
}
