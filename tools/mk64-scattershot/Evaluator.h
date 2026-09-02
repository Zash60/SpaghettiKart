#pragma once
#include "MK64State.h"
namespace Evaluator {
 bool isFinished(const MK64State& s);
 int progress(const MK64State& s);
 float score(const MK64State& s, int framesToFinish);
}
