#include "port/BfMutator.h"
#include "port/BfBase.h"
#include "port/BfSim.h"
#include "port/BfState.h"
#include <cstdio>
#include <vector>
#include <random>
#include <spdlog/spdlog.h>
#include <libultraship.h>

static bool sSearching = false;
static int sWindow = 30;
static int sMaxIters = 200;
static int sIter = 0;
static int sBestTicks = -1;
static std::vector<BfInput> sBest;
static std::mt19937 sRng(12345);
static int sPhase = 0; // 0 idle/needs new candidate, 1 sim running
static std::vector<BfInput> sCand;
static char sPath[512] = { 0 };

static void CacheResultPath(void);

const char* Bf_ResultPath(void) {
    if (sPath[0] == '\0') {
        CacheResultPath();
    }
    return sPath;
}

static void CacheResultPath(void) {
    std::string p = Ship::Context::GetRawInstance()->GetAppDirectoryPath() + "/result.txt";
    snprintf(sPath, sizeof(sPath), "%s", p.c_str());
}

void Bf_SearchStart(int windowFrames, int maxIters) {
    int n = Bf_BaseLen();
    if (n <= 0) {
        SPDLOG_ERROR("BF no base recorded (bf_record 1, drive a finished race, bf_record 0)");
        return;
    }
    CacheResultPath();
    sBest.assign(Bf_BaseData(), Bf_BaseData() + n);
    // Measure the base first: run and count ticks to finish.
    sSearching = true;
    sIter = 0;
    sPhase = 0;
    sWindow = windowFrames;
    sMaxIters = maxIters;
    sBestTicks = -1;
    sCand = sBest;
    // TMInterface validation: the base MUST finish. If the baseline times
    // out, Bf_SearchFrame aborts with an explicit error.
    Bf_SimBegin(sCand.data(), (int)sCand.size(), (int)sCand.size() + 3600);
    sPhase = 1;
    SPDLOG_INFO("BF search started: {} base frames, window {}, maxIters {}", n, sWindow, sMaxIters);
}

void Bf_SearchStop(void) {
    if (sPhase == 1) {
        Bf_SimEnd();
    }
    sSearching = false;
    sPhase = 0;
    SPDLOG_INFO("BF search stopped");
}

void Bf_SearchFrame(void) {
    if (!sSearching) {
        return;
    }
    if (sPhase == 0) {
        if (sIter >= sMaxIters) {
            SPDLOG_INFO("BF done: {} iters, best {} ticks", sIter, sBestTicks);
            Bf_SearchStop();
            return;
        }
        // Mutate: copy the best and perturb window [f0, f0+sWindow).
        sCand = sBest;
        int n = (int)sCand.size();
        int f0 = sIter == 0 ? 0 : (std::uniform_int_distribution<int>(0, n - 1)(sRng) % n);
        if (f0 + sWindow > n) {
            f0 = n - sWindow;
        }
        if (f0 < 0) {
            f0 = 0;
        }
        for (int f = f0; f < f0 + sWindow && f < n; f++) {
            sCand[f].stickX += (int16_t)std::uniform_int_distribution<int>(-8, 8)(sRng);
            if (std::uniform_int_distribution<int>(0, 9)(sRng) == 0) {
                sCand[f].button ^= A_BUTTON;
            }
        }
        Bf_SimBegin(sCand.data(), n, n + 3600);
        sPhase = 1;
        return;
    }
    int r = Bf_SimPoll();
    if (r == 0) {
        return; // still simulating
    }
    Bf_SimEnd();
    if (sBestTicks < 0 && r < 0) {
        SPDLOG_ERROR("BF base does not finish the race; record a finished race first");
        sSearching = false;
        sPhase = 0;
        return;
    }
    if (r > 0 && (sBestTicks < 0 || r < sBestTicks)) {
        sBestTicks = r;
        sBest = sCand;
        Bf_SetBase(sBest.data(), (int)sBest.size());
        Bf_SaveResult(Bf_ResultPath());
        SPDLOG_INFO("BF new best: {} ticks, iter {}", r, sIter);
    }
    sIter++;
    sPhase = 0;
}

int Bf_SaveResult(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) {
        return -1;
    }
    fprintf(f, "%d %d\n", sBestTicks, (int)sBest.size());
    for (size_t i = 0; i < sBest.size(); i++) {
        fprintf(f, "%d %d %u\n", sBest[i].stickX, sBest[i].stickY, (unsigned)sBest[i].button);
    }
    fclose(f);
    return 0;
}

int Bf_LoadResult(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        return -1;
    }
    int ticks = -1, n = 0;
    if (fscanf(f, "%d %d\n", &ticks, &n) != 2 || n <= 0 || n > 60 * 60 * 10) {
        fclose(f);
        return -1;
    }
    std::vector<BfInput> v;
    v.reserve(n);
    for (int i = 0; i < n; i++) {
        int sx = 0, sy = 0;
        unsigned b = 0;
        if (fscanf(f, "%d %d %u\n", &sx, &sy, &b) != 3) {
            fclose(f);
            return -1;
        }
        BfInput in;
        in.stickX = (int16_t)sx;
        in.stickY = (int16_t)sy;
        in.button = (uint16_t)b;
        v.push_back(in);
    }
    fclose(f);
    Bf_SetBase(v.data(), (int)v.size());
    sBestTicks = ticks;
    return 0;
}
