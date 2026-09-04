#include "port/BfMutator.h"
#include "port/BfBase.h"
#include "port/BfSim.h"
#include "port/BfState.h"
#include <cstdio>
#include <vector>
#include <random>
#include <spdlog/spdlog.h>
#include <libultraship.h>

extern "C" {
#include "replays.h"
#include <defines.h>
}

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

// Tunables (TMInterface: no universal settings, tune per track).
static float sProb = 0.3f;   // chance per input in window to mutate
static int sSteerDiff = 8;   // max stick delta each direction
static int sMinFrame = 0;    // mutation window clamp start
static int sMaxFrame = -1;   // mutation window clamp end (-1 = base end)
static int sSimSpeed = 32;   // ticks per rendered frame while simulating

void Bf_ParamsReset(void) {
    sProb = 0.3f;
    sSteerDiff = 8;
    sMinFrame = 0;
    sMaxFrame = -1;
    sSimSpeed = 32;
}

static int NameEq(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

int Bf_ParamSet(const char* name, float value) {
    if (NameEq(name, "prob")) {
        sProb = value < 0.05f ? 0.05f : (value > 1.0f ? 1.0f : value);
        return 0;
    }
    if (NameEq(name, "steer")) {
        sSteerDiff = (int)(value < 1 ? 1 : (value > 40 ? 40 : value));
        return 0;
    }
    if (NameEq(name, "minframe")) {
        sMinFrame = (int)(value < 0 ? 0 : value);
        return 0;
    }
    if (NameEq(name, "maxframe")) {
        sMaxFrame = (int)value;
        return 0;
    }
    if (NameEq(name, "speed")) {
        sSimSpeed = (int)(value < 1 ? 1 : (value > 64 ? 64 : value));
        return 0;
    }
    return -1;
}

float Bf_ParamGet(const char* name) {
    if (NameEq(name, "prob")) {
        return sProb;
    }
    if (NameEq(name, "steer")) {
        return (float)sSteerDiff;
    }
    if (NameEq(name, "minframe")) {
        return (float)sMinFrame;
    }
    if (NameEq(name, "maxframe")) {
        return (float)sMaxFrame;
    }
    if (NameEq(name, "speed")) {
        return (float)sSimSpeed;
    }
    return -1.0f;
}

int Bf_SimSpeed(void) {
    return sSimSpeed;
}

int Bf_IsSearching(void) {
    return sSearching ? 1 : 0;
}

int Bf_SearchIter(void) {
    return sIter;
}

int Bf_BestTicks(void) {
    return sBestTicks;
}

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
    if (Bf_RootSessionOk() != 0) {
        SPDLOG_ERROR("BF race changed since recording; restart race, bf_record, then bf_start in the same race");
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

void Bf_Reset(void) {
    Bf_SearchStop();
    Bf_PlayBaseStop();
    Bf_RecordStop();
    Bf_ClearBase();
    sBest.clear();
    sCand.clear();
    sBestTicks = -1;
    sIter = 0;
    sPhase = 0;
    Bf_ParamsReset();
    SPDLOG_INFO("BF state reset (base cleared)");
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
        // Mutate: copy the best and perturb window [f0, f0+sWindow),
        // clamped to [sMinFrame, sMaxFrame]. Each input mutates with
        // probability sProb by up to +-sSteerDiff stick.
        sCand = sBest;
        int n = (int)sCand.size();
        int lo = sMinFrame < 0 ? 0 : (sMinFrame >= n ? n - 1 : sMinFrame);
        int hi = (sMaxFrame < 0 || sMaxFrame >= n) ? n : sMaxFrame + 1;
        int span = hi - lo;
        if (span < 1) {
            lo = 0;
            span = n;
        }
        int f0 = sIter == 0 ? lo : lo + (std::uniform_int_distribution<int>(0, span - 1)(sRng) % span);
        if (f0 + sWindow > hi) {
            f0 = hi - sWindow;
        }
        if (f0 < lo) {
            f0 = lo;
        }
        std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
        for (int f = f0; f < f0 + sWindow && f < hi; f++) {
            if (probDist(sRng) > sProb) {
                continue;
            }
            sCand[f].stickX +=
                (int16_t)std::uniform_int_distribution<int>(-sSteerDiff, sSteerDiff)(sRng);
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
    if (sBestTicks < 0) {
        // Baseline measurement (first sim): must finish, else abort loudly.
        if (r < 0) {
            SPDLOG_ERROR("BF base does not finish the race; record a finished race first");
            sSearching = false;
            sPhase = 0;
            return;
        }
        SPDLOG_INFO("BF baseline: {} ticks", r);
    } else if (r < 0) {
        static int sTimeouts = 0;
        if (++sTimeouts % 25 == 1) {
            SPDLOG_INFO("BF iter {}: candidate timeout ({} total)", sIter, sTimeouts);
        }
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
        fprintf(f, "%d %d %u %u %u\n", sBest[i].stickX, sBest[i].stickY, (unsigned)sBest[i].button,
                (unsigned)sBest[i].buttonPressed, (unsigned)sBest[i].buttonDepressed);
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
        unsigned b = 0, bp = 0, bd = 0;
        // v2 has pressed/depressed; v1 (3 fields) reads bp=bd=0.
        int got = fscanf(f, "%d %d %u %u %u\n", &sx, &sy, &b, &bp, &bd);
        if (got < 3) {
            fclose(f);
            return -1;
        }
        BfInput in;
        in.stickX = (int16_t)sx;
        in.stickY = (int16_t)sy;
        in.button = (uint16_t)b;
        in.buttonPressed = (uint16_t)bp;
        in.buttonDepressed = (uint16_t)bd;
        v.push_back(in);
    }
    fclose(f);
    Bf_SetBase(v.data(), (int)v.size());
    sBestTicks = ticks;
    return 0;
}

// Session-only BF ghost slot (RAM, no pak): the game's ghost buffers are
// never touched, so the player ghost can be restored at any time.
static uint32_t sBfGhost[0x1000];

static uint32_t BfEncodeEntry(const BfInput& in, unsigned run) {
    int sx = in.stickX;
    int sy = in.stickY;
    if (sx < -128) {
        sx = -128;
    }
    if (sx > 127) {
        sx = 127;
    }
    if (sy < -128) {
        sy = -128;
    }
    if (sy > 127) {
        sy = 127;
    }
    uint32_t e = ((uint32_t)(sx & 0xFF)) | ((uint32_t)((sy & 0xFF) << 8)) | ((uint32_t)(run << 16));
    if (in.button & A_BUTTON) {
        e |= REPLAY_A_BUTTON;
    }
    if (in.button & B_BUTTON) {
        e |= REPLAY_B_BUTTON;
    }
    if (in.button & Z_TRIG) {
        e |= REPLAY_Z_TRIG;
    }
    if (in.button & R_TRIG) {
        e |= REPLAY_R_TRIG;
    }
    return e;
}

int Bf_ExportGhost(void) {
    int n = Bf_BaseLen();
    if (n <= 0) {
        SPDLOG_ERROR("BF no base to export (record or load a result first)");
        return -1;
    }
    const BfInput* d = Bf_BaseData();
    int w = 0;
    int i = 0;
    while (i < n) {
        // Counter stores runLen-1 (matches save_player_replay RLE).
        unsigned run = 0;
        while (i + (int)run + 1 < n && run + 1 < 0x100 && d[i + run + 1].stickX == d[i].stickX &&
               d[i + run + 1].stickY == d[i].stickY && d[i + run + 1].button == d[i].button) {
            run++;
        }
        if (w >= 0x1000) {
            SPDLOG_ERROR("BF base too long for ghost slot ({} RLE entries)", w);
            return -1;
        }
        sBfGhost[w++] = BfEncodeEntry(d[i], run);
        i += run + 1;
    }
    sPlayerGhostReplay = sBfGhost;
    reset_player_ghost_state();
    SPDLOG_INFO("BF exported {} frames as session ghost ({} entries)", n, w);
    return 0;
}

void Bf_RestorePlayerGhost(void) {
    load_player_ghost();
    SPDLOG_INFO("BF restored player ghost");
}
