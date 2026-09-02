#include "Evaluator.h"
namespace Evaluator {
#ifdef HEADLESS
 bool isFinished(const MK64State& s){ return s.players[0].lapCount >= 3; }
 int progress(const MK64State& s){ return s.players[0].lapCount*1000000 + int(s.players[0].pos[0]/10) + int(s.players[0].speed*10); }
#else
 bool isFinished(const MK64State& s){ return s.karts[0].lap >= 1 && s.karts[0].checkpoint==0; }
 int progress(const MK64State& s){ return s.karts[0].checkpoint*1000000 + s.karts[0].posX/10 + s.karts[0].speed*10; }
#endif
 float score(const MK64State& s, int framesToFinish){ return isFinished(s) ? float(framesToFinish) : 1e9f - float(progress(s)); }
}
