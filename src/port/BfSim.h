#pragma once
#include "port/BfBase.h"

void Bf_OverrideTick(void);
// Cooperative simulation: the game only advances ticks inside its own frame
// loop (src/main.c:724), so Begin arms the candidate and Poll reads the result.
void Bf_SimBegin(const BfInput* cand, int n, int maxTicks);
int Bf_SimPoll(void); // 0 = running, >0 = finish ticks, -1 = timeout/no finish
void Bf_SimEnd(void); // state restore + gTickLogic back + disarm
void Bf_PlayBaseStart(void);
void Bf_PlayBaseStop(void);
