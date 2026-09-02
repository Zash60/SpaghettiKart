#pragma once
#include "Scattershot.h"
struct ThreadState { GlobalState* g=nullptr; uint64_t seed=0; void scatter(int depth); };
