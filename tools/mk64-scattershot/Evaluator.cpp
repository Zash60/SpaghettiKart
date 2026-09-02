#include "Evaluator.h"
namespace Evaluator {
 bool isFinished(const MK64State& s){ return s.karts[0].lap >= 1 && s.karts[0].checkpoint==0; }
 int progress(const MK64State& s){ return s.karts[0].checkpoint*1000000 + s.karts[0].posX/10 + s.karts[0].speed*10; }
 float score(const MK64State& s, int framesToFinish){ return isFinished(s) ? float(framesToFinish) : 1e9f - float(progress(s)); }
}
