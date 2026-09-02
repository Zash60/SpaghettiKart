#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "Scattershot.h"
#include "ReplayWriter.h"
int main(int argc, char** argv){
 std::string course="mario_raceway"; int timeSec=5, threads=4, depth=30; std::string out="out";
 for(int i=1;i<argc;i++){ std::string a=argv[i]; if(a=="--course" && i+1<argc) course=argv[++i]; else if(a=="--time" && i+1<argc) timeSec=atoi(argv[++i]); else if(a=="--threads" && i+1<argc) threads=atoi(argv[++i]); else if(a=="--out" && i+1<argc) out=argv[++i]; else if(a=="--help"){ std::cout<<"mk64-scattershot --course mario_raceway --time 60 --threads 8 --out out\n"; return 0; } }
 std::cout<<"mk64-scattershot course="<<course<<" time="<<timeSec<<" threads="<<threads<<" depth="<<depth<<"\n";
 GlobalState g(1<<16);
 MK64State s0{}; s0.karts[0].posX=0; s0.karts[0].checkpoint=0;
 Segment* root=new Segment{nullptr,0,0,{},s0,1e9f};
 g.blocks.push_back({hashPos(s0), root});
 g.hashTab[g.findNewHashInx(hashPos(s0))] = 0;
 std::atomic<bool> stop{false};
 auto start=std::chrono::steady_clock::now();
 std::vector<std::thread> ths;
 for(int t=0;t<threads;t++){
  ths.emplace_back([&,t](){
   ThreadState ts{&g, uint64_t(42 + t*0x9e3779b1)};
   while(!stop){
    ts.scatter(depth);
   }
  });
 }
 while(true){
  auto now=std::chrono::steady_clock::now();
  int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now-start).count();
  if(elapsed>=timeSec) break;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::cout<<"bestBlocks="<<g.blocks.size()<<" best="<<(g.bestBlock? g.bestBlock->blockLength():-1)<<"\n";
 }
 stop=true;
 for(auto &th: ths) th.join();
 auto replay = collectInputs(g.bestBlock);
 std::string mkr = out+"/best.mkr";
 std::string js = out+"/best.json";
 // ensure out dir
 system(("mkdir -p "+out).c_str());
 writeMkr(replay, mkr);
 writeJson(g, replay.size(), js);
 std::cout<<"wrote "<<mkr<<" frames="<<replay.size()<<" blocks="<<g.blocks.size()<<"\n";
 return 0;
}
