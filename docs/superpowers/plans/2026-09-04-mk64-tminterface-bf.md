# MK64 Bruteforce estilo TMInterface Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove todo o Scattershot e adicionar BF integrado ao jogo que grava a volta do jogador, re-simula rápido com save states e encontra inputs melhores (só tempo final), salvando `result.txt`.

**Architecture:** Remoção primeiro (1 commit). Depois `src/port/bf/` com 4 arquivos focados: `BfState` (snapshot/restore), `BfBase` (grava base ao vivo), `BfSim` (injeção por tick + `gTickLogic` alto), `BfMutator` (hill-climbing + `result.txt` + UI Developer). Hook de injeção no topo de `func_8028F474` — mesmo ponto que o Scattershot usava e funcionava.

**Tech Stack:** C/C++ no tree SpaghettiKart, `struct Controller` (`include/common_structs.h:66`), `gTickLogic` (`src/main.h:162`, loop `src/main.c:724`), menu `src/port/ui/PortMenu.cpp`, `SPDLOG_INFO` p/ log.

**Spec:** `docs/superpowers/specs/2026-09-04-mk64-tminterface-bruteforce-design.md`

## Global Constraints

- Repo C++ sem harness de testes: verificação = build desktop + `grep` + comportamento via log/console. Sem pytest.
- Nunca commitar segredos; só arquivos listados em cada task.
- Um commit por task, mensagem curta no estilo do repo (`fix:`, `feat:`, `docs:`, `chore:`).
- Desktop build de verificação: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)` (ajustar ao comando que o repo usa se existir preset).
- Fim de corrida detectado por `gLapCountByPlayerId[0] >= 3`; tempo medido em ticks contados pelo runner (não depende de globais de timer).

---

### Task 1: Remoção total do Scattershot

**Files:**
- Delete: `tools/mk64-scattershot/` (dir inteiro), `.github/workflows/scattershot.yml`, `src/port/ScattershotReplay.h`, `src/port/ScattershotReplay.cpp`, `docs/superpowers/specs/2026-09-02-scattershot-mk64-design.md`, `docs/superpowers/plans/2026-09-02-scattershot-mk64.md`
- Modify: `src/racing/race_logic.c` (3 trechos), `src/port/ui/PortMenu.cpp` (include + seção), `CMakeLists.txt` (bloco `if(SCATTERSHOT_HEADLESS...)`)
- Test: `grep -ri scatter --include='*' . | grep -v '.git/'` deve sair vazio + build desktop passa.

**Interfaces:**
- Consumes: nada (só deleta).
- Produces: tree sem referências a scatter; `func_8028F474`, `start_race`, marcador STAGING limpos.

- [ ] **Step 1: Deletar arquivos e diretórios**

```bash
git rm -r tools/mk64-scattershot .github/workflows/scattershot.yml src/port/ScattershotReplay.h src/port/ScattershotReplay.cpp docs/superpowers/specs/2026-09-02-scattershot-mk64-design.md docs/superpowers/plans/2026-09-02-scattershot-mk64.md
```

- [ ] **Step 2: Limpar `src/racing/race_logic.c` (3 trechos)**

Trecho A — em `start_race`, trocar:
```c
    if (gRaceState == RACE_STAGING) {
        gRaceState = RACE_IN_PROGRESS;
        {
            extern void Scattershot_AutoLoad(void);
            Scattershot_AutoLoad();
        }
    }
```
por:
```c
    if (gRaceState == RACE_STAGING) {
        gRaceState = RACE_IN_PROGRESS;
    }
```

Trecho B — em `func_8028F474`, trocar:
```c
void func_8028F474(void) {
    s32 i;

    {
        extern void Scattershot_OverrideController(void);
        Scattershot_OverrideController();
    }
    switch (gRaceState) {
```
por:
```c
void func_8028F474(void) {
    s32 i;

    switch (gRaceState) {
```

Trecho C — no marcador STAGING, trocar:
```c
                LUSLOG_DEBUG("Scattershot marker STAGING timer=%d", gGlobalTimer);
                {
                    extern void Scattershot_AutoLoad(void);
                    Scattershot_AutoLoad();
                }
```
por:
```c
                LUSLOG_DEBUG("STAGING timer=%d", gGlobalTimer);
```

- [ ] **Step 3: Limpar `src/port/ui/PortMenu.cpp` e `CMakeLists.txt`**

Em `PortMenu.cpp`: deletar a linha `#include "port/ScattershotReplay.h"` e o bloco inteiro desde `path = { "Developer", "Scattershot", SECTION_COLUMN_1 };` até o `AddWidget(... "Auto-load on Go" ...)` + linha `.Options(CheckboxOptions()...DefaultValue(true));` + `}` de fechamento da seção (manter o `}` da função `AddDeveloperTools` — conferir pelo contexto: o próximo bloco é `void PortMenu::AddSceneVisibility()`).

Em `CMakeLists.txt`: deletar o bloco
```cmake
if(SCATTERSHOT_HEADLESS AND NOT ANDROID)
  add_subdirectory(tools/mk64-scattershot)
endif()
```

- [ ] **Step 4: Verificar zero referências + build**

```bash
grep -ri scatter . --exclude-dir=.git --exclude-dir=build; echo "grep exit: $?"
```
Expected: exit 1 (nada encontrado).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
```
Expected: build completo sem erro.

- [ ] **Step 5: Commit**

```bash
git add -A && git commit -m "chore: remove all scattershot code, tool, CI and docs"
```

---

### Task 2: `BfState` — snapshot/restore do estado de corrida

**Files:**
- Create: `src/port/bf/BfState.h`, `src/port/bf/BfState.cpp` (compilam via glob `src/port/*.cpp`? NÃO — subdir não entra no glob da linha 97 do CMakeLists; então registrar explicitamente OU colocar os arquivos em `src/port/` direto).

Decisão: criar em `src/port/BfState.{h,cpp}`, `src/port/BfBase.{h,cpp}`, `src/port/BfSim.{h,cpp}`, `src/port/BfMutator.{h,cpp}` para cair no glob existente sem mexer no build.

- Test: build passa + comando de console `bf_test_state` (registrado temporariamente nesta task e removido na Task 5) que faz snapshot, avança 60 ticks, restore, e compara `gPlayers[0].pos` — log `BF state roundtrip OK/FAIL`.

**Interfaces:**
- Consumes: `gPlayers`, `gControllers` (`src/main.h`), `gLapCountByPlayerId`, `gPathIndexByPlayerId`, `gNearestPathPointByPlayerId` (globais do jogo).
- Produces:
  - `void BfState_Save(BfRaceState* out);`
  - `void BfState_Restore(const BfRaceState* in);`
  - `struct BfRaceState` opaco no header (definido no .cpp), header só declara ponteiro + funções.

- [ ] **Step 1: Escrever `src/port/BfState.h`**

```cpp
#pragma once

// Opaque snapshot of everything BfSim needs to rewind the race.
struct BfRaceState;

void BfState_Save(BfRaceState* out);
void BfState_Restore(const BfRaceState* in);
BfRaceState* BfState_Alloc(void);
void BfState_Free(BfRaceState* st);
```

- [ ] **Step 2: Escrever `src/port/BfState.cpp`**

```cpp
#include "port/BfState.h"
#include <cstring>

extern "C" {
#include "common_structs.h"
#include "main.h"
#include "code_80005FD0.h" // gPlayers if declared here; else rely on main.h
}

#define BF_MAX_PLAYERS 8
#define BF_MAGIC 0x00464253u // "BFS\0"

// Confirmed globals from prior work: gPlayers, gControllers,
// gLapCountByPlayerId, gPathIndexByPlayerId, gNearestPathPointByPlayerId.
struct BfRaceState {
    uint32_t magic;
    Player players[BF_MAX_PLAYERS];
    struct Controller controllers[BF_MAX_PLAYERS];
    int32_t lapCount[BF_MAX_PLAYERS];
    int32_t pathIndex[12];
    int32_t nearestPath[12];
};

void BfState_Save(BfRaceState* out) {
    out->magic = BF_MAGIC;
    memcpy(out->players, gPlayers, sizeof(out->players));
    memcpy(out->controllers, gControllers, sizeof(out->controllers));
    memcpy(out->lapCount, gLapCountByPlayerId, sizeof(out->lapCount));
    memcpy(out->pathIndex, gPathIndexByPlayerId, sizeof(out->pathIndex));
    memcpy(out->nearestPath, gNearestPathPointByPlayerId, sizeof(out->nearestPath));
}

void BfState_Restore(const BfRaceState* in) {
    if (in->magic != BF_MAGIC) { return; }
    memcpy(gPlayers, in->players, sizeof(in->players));
    memcpy(gControllers, in->controllers, sizeof(in->controllers));
    memcpy(const_cast<int32_t*>(gLapCountByPlayerId), in->lapCount, sizeof(in->lapCount));
    memcpy(gPathIndexByPlayerId, in->pathIndex, sizeof(in->pathIndex));
    memcpy(gNearestPathPointByPlayerId, in->nearestPath, sizeof(in->nearestPath));
}

BfRaceState* BfState_Alloc(void) { return new BfRaceState(); }
void BfState_Free(BfRaceState* st) { delete st; }
```

Nota ao executor: se algum global tiver tipo diferente de `int32_t[]` (ex. `s16`), ajustar o `memcpy` com `sizeof` do destino real — compilar e corrigir pelo erro do compilador.

- [ ] **Step 3: Build**

Run: `cmake --build build -j$(nproc)`
Expected: PASS (arquivos novos entram pelo glob `src/port/*.cpp`).

- [ ] **Step 4: Commit**

```bash
git add src/port/BfState.h src/port/BfState.cpp && git commit -m "feat: add BfState race snapshot/restore"
```

---

### Task 3: `BfBase` — grava a volta do jogador como base

**Files:**
- Create: `src/port/BfBase.h`, `src/port/BfBase.cpp`
- Test: build + jogar 1 volta time-trial com `bf_record 1` no console → log `BF recorded N frames`; `bf_record 0` → `BF base ready: N frames`.

**Interfaces:**
- Consumes: `struct Controller` (`include/common_structs.h:66`: `rawStickX`, `rawStickY`, `button`), `Bf_RecordTick` chamado do mesmo ponto do hook da Task 4 (topo de `func_8028F474`, antes do switch).
- Produces:
  - `struct BfInput { int16_t stickX; int16_t stickY; uint16_t button; };`
  - `void Bf_RecordTick(void);` (amostra `gControllers[0]` se gravando)
  - `void Bf_RecordStart(void); void Bf_RecordStop(void);`
  - `const BfInput* Bf_BaseData(void); int Bf_BaseLen(void);`
  - `void Bf_SetBase(const BfInput* d, int n);` (usado pelo Mutator e pelo load de result.txt)

- [ ] **Step 1: Escrever `src/port/BfBase.h`**

```cpp
#pragma once
#include <stdint.h>

struct BfInput { int16_t stickX; int16_t stickY; uint16_t button; };

void Bf_RecordStart(void);
void Bf_RecordStop(void);
void Bf_RecordTick(void);
const BfInput* Bf_BaseData(void);
int Bf_BaseLen(void);
void Bf_SetBase(const BfInput* d, int n);
```

- [ ] **Step 2: Escrever `src/port/BfBase.cpp`**

```cpp
#include "port/BfBase.h"
#include <vector>

extern "C" {
#include "common_structs.h"
#include "main.h"
}

static std::vector<BfInput> sBase;
static bool sRecording = false;

void Bf_RecordStart(void) { sBase.clear(); sRecording = true; }
void Bf_RecordStop(void) { sRecording = false; }

void Bf_RecordTick(void) {
    if (!sRecording) { return; }
    BfInput in;
    in.stickX = gControllers[0].rawStickX;
    in.stickY = gControllers[0].rawStickY;
    in.button = gControllers[0].button;
    sBase.push_back(in);
}

const BfInput* Bf_BaseData(void) { return sBase.data(); }
int Bf_BaseLen(void) { return (int)sBase.size(); }

void Bf_SetBase(const BfInput* d, int n) {
    sBase.assign(d, d + n);
}
```

- [ ] **Step 3: Registrar comandos de console temporários**

Usar o mesmo mecanismo de comandos que o repo já tem para console (procurar como outros comandos `load`/`help` são registrados; ex. `Ship::Context` ou tabela de comandos em `src/port`). Registrar `bf_record <0|1>` chamando `Bf_RecordStart/Stop` + `SPDLOG_INFO("BF recorded {} frames", Bf_BaseLen())`. (Comandos definitivos ficam na Task 5; estes são andaime de teste.)

- [ ] **Step 4: Build + teste manual**

Run: `cmake --build build -j$(nproc)` Expected: PASS.
Manual: abrir time trial 1P, `bf_record 1`, jogar até cruzar a linha (3 voltas), `bf_record 0` → log mostra `BF base ready: N frames` com N > 1000.

- [ ] **Step 5: Commit**

```bash
git add src/port/BfBase.h src/port/BfBase.cpp && git commit -m "feat: add BfBase live input recording"
```

(O andaime de console desta task pode ir no mesmo commit; será consolidado na Task 5.)

---

### Task 4: `BfSim` — injeção por tick + simulação rápida

**Files:**
- Modify: `src/racing/race_logic.c` (hook no topo de `func_8028F474`), `src/main.c` (salvar/restaurar `gTickLogic` — via funções, sem editar o loop).
- Create: `src/port/BfSim.h`, `src/port/BfSim.cpp`
- Test: build + replay curto via `bf_playbase` (andaime): kart anda sozinho igual ao gravado → determinismo visual; log `BF sim ticks=X finished=0/1`.

**Interfaces:**
- Consumes: `Bf_BaseData/Len` (Task 3), `BfState_Save/Restore` (Task 2), `gTickLogic` (`src/main.h:162`), `gLapCountByPlayerId`.
- Produces:
  - `void Bf_OverrideTick(void);` (chamado no topo de `func_8028F474`)
  - `void Bf_SimBegin(const BfInput* cand, int n, int maxTicks);` + `int Bf_SimPoll(void);` + `void Bf_SimEnd(void);` (ciclo cooperativo: Begin salva estado e põe `gTickLogic=32`, Poll retorna 0 rodando / >0 ticks do finish / -1 timeout, End restaura tudo)
  - `void Bf_PlayBaseStart(void); void Bf_PlayBaseStop(void);` (andaime p/ teste desta task)

- [ ] **Step 1: Escrever `src/port/BfSim.h`**

```cpp
#pragma once
#include "port/BfBase.h"

void Bf_OverrideTick(void);
// Simulação cooperativa: o jogo só avança ticks no próprio loop de frames
// (src/main.c:724), então Begin arma o candidato e Poll lê o resultado.
void Bf_SimBegin(const BfInput* cand, int n, int maxTicks);
int Bf_SimPoll(void); // 0 = rodando, >0 = ticks do finish, -1 = timeout/sem finish
void Bf_SimEnd(void); // restore do estado + gTickLogic de volta + desarma
void Bf_PlayBaseStart(void);
void Bf_PlayBaseStop(void);
```

- [ ] **Step 2: Escrever `src/port/BfSim.cpp`**

```cpp
#include "port/BfSim.h"
#include "port/BfState.h"

extern "C" {
#include "common_structs.h"
#include "main.h"
}

static bool sPlaying = false;
static int sPlayIdx = 0;
static const BfInput* sCand = nullptr;
static int sCandLen = 0;
static int sCandIdx = 0;
static int sSimTicks = 0;
static int sSimMax = 0;
static int sSimResult = 0; // 0 rodando, >0 finish, -1 timeout
static int sSavedTickLogic = 2;
static BfRaceState* sSimSaved = nullptr;

void Bf_PlayBaseStart(void) { sPlaying = true; sPlayIdx = 0; }
void Bf_PlayBaseStop(void) { sPlaying = false; }

static void Inject(const BfInput& in) {
    gControllers[0].rawStickX = in.stickX;
    gControllers[0].rawStickY = in.stickY;
    gControllers[0].button = in.button;
    gControllers[0].buttonPressed = 0;
}

void Bf_SimBegin(const BfInput* cand, int n, int maxTicks) {
    if (sSimSaved) { BfState_Free(sSimSaved); }
    sSimSaved = BfState_Alloc();
    BfState_Save(sSimSaved);
    sSavedTickLogic = gTickLogic;
    gTickLogic = 32;
    sCand = cand; sCandLen = n; sCandIdx = 0;
    sSimTicks = 0; sSimMax = maxTicks; sSimResult = 0;
}

int Bf_SimPoll(void) { return sSimResult; }

void Bf_SimEnd(void) {
    if (sSimSaved) { BfState_Restore(sSimSaved); BfState_Free(sSimSaved); sSimSaved = nullptr; }
    gTickLogic = sSavedTickLogic;
    sCand = nullptr;
}

void Bf_OverrideTick(void) {
    if (sPlaying) {
        const BfInput* b = Bf_BaseData();
        int n = Bf_BaseLen();
        if (sPlayIdx < n) { Inject(b[sPlayIdx]); }
        sPlayIdx++;
        return;
    }
    if (!sCand || sSimResult != 0) { return; }
    if (sCandIdx < sCandLen) { Inject(sCand[sCandIdx]); sCandIdx++; }
    sSimTicks++;
    if (gLapCountByPlayerId[0] >= 3) { sSimResult = sSimTicks; return; }
    if (sSimTicks >= sSimMax) { sSimResult = -1; }
}
```

- [ ] **Step 3: Hook em `func_8028F474`**

No topo da função (onde era o hook Scattershot removido na Task 1):
```c
void func_8028F474(void) {
    s32 i;

    {
        extern void Bf_OverrideTick(void);
        Bf_OverrideTick();
    }
    switch (gRaceState) {
```

- [ ] **Step 4: Build + teste de replay da base**

Run: `cmake --build build -j$(nproc)` Expected: PASS.
Manual: gravar base curta (Task 3), voltar ao início da corrida, `bf_playbase` → kart repete os inputs; tempo final igual ao gravado (±0) prova determinismo + injeção correta.

- [ ] **Step 5: Commit**

```bash
git add src/port/BfSim.h src/port/BfSim.cpp src/racing/race_logic.c && git commit -m "feat: add BfSim tick injection and fast simulation"
```

---

### Task 5: `BfMutator` — hill-climbing, `result.txt`, UI Developer

**Files:**
- Create: `src/port/BfMutator.h`, `src/port/BfMutator.cpp`
- Modify: `src/port/ui/PortMenu.cpp` (seção Developer > Bruteforce), comandos de console definitivos (consolidar andaimes das Tasks 2–4 em `bf_record`, `bf_start`, `bf_stop`, `bf_loadresult`).
- Test: build + BF real numa volta gravada: log verde `BF new best: <ticks> iter <k>` + `result.txt` no app dir; `bf_loadresult` + `bf_playbase` repete o melhor.

**Interfaces:**
- Consumes: `Bf_BaseData/Len`, `Bf_SetBase` (Task 3); `Bf_SimBegin/Poll/End`, `Bf_OverrideTick` (Task 4); `BfState_*` (Task 2).
- Produces:
  - `void Bf_SearchStart(int windowFrames, int maxIters);`
  - `void Bf_SearchStop(void);`
  - `void Bf_SearchFrame(void);` (chamado 1x por frame quando buscando: dirige SimBegin/Poll/End + mutação + save)
  - `int Bf_SaveResult(const char* path); int Bf_LoadResult(const char* path);`
  - Formato `result.txt`: linha 1 `<ticks> <frames>`, demais linhas `<stickX> <stickY> <button>` (um input por linha).

- [ ] **Step 1: Escrever `src/port/BfMutator.h`**

```cpp
#pragma once

void Bf_SearchStart(int windowFrames, int maxIters);
void Bf_SearchStop(void);
void Bf_SearchFrame(void);
int Bf_SaveResult(const char* path);
int Bf_LoadResult(const char* path);
const char* Bf_ResultPath(void);
```

- [ ] **Step 2: Escrever `src/port/BfMutator.cpp`**

```cpp
#include "port/BfMutator.h"
#include "port/BfBase.h"
#include "port/BfSim.h"
#include "port/BfState.h"
#include <cstdio>
#include <vector>
#include <random>

static bool sSearching = false;
static int sWindow = 30;
static int sMaxIters = 200;
static int sIter = 0;
static int sBestTicks = -1;
static std::vector<BfInput> sBest;
static std::mt19937 sRng(12345);
static int sPhase = 0; // 0 idle, 1 sim running
static std::vector<BfInput> sCand;
static char sPath[512] = {0};

const char* Bf_ResultPath(void) {
    // Mesmo padrão do Dump Collision removido: app dir.
    // Executor: obter via Ship::Context::GetRawInstance()->GetAppDirectoryPath()
    // e concatenar "/result.txt" em Bf_SearchStart (cache em sPath).
    return sPath;
}

void Bf_SearchStart(int windowFrames, int maxIters) {
    int n = Bf_BaseLen();
    if (n <= 0) { return; } // sem base, sem busca (erro logado pelo chamador)
    sBest.assign(Bf_BaseData(), Bf_BaseData() + n);
    // Mede a base primeiro: roda e conta ticks até finish.
    sSearching = true; sIter = 0; sPhase = 0;
    sWindow = windowFrames; sMaxIters = maxIters;
    sBestTicks = -1;
    sCand = sBest;
    // Validação TMInterface: a base PRECISA terminar. Se o baseline estourar
    // o timeout, Bf_SearchFrame aborta com erro explícito (equivale ao
    // "replay não termina" do TMInterface).
    Bf_SimBegin(sCand.data(), (int)sCand.size(), (int)sCand.size() + 3600);
    sPhase = 1;
}

void Bf_SearchStop(void) {
    if (sPhase == 1) { Bf_SimEnd(); }
    sSearching = false; sPhase = 0;
}

void Bf_SearchFrame(void) {
    if (!sSearching) { return; }
    if (sPhase == 0) {
        if (sIter >= sMaxIters) { Bf_SearchStop(); return; }
        // Muta: copia a melhor e perturba a janela [f0, f0+sWindow).
        sCand = sBest;
        int n = (int)sCand.size();
        int f0 = sIter == 0 ? 0 : (std::uniform_int_distribution<int>(0, n - 1)(sRng) % n);
        if (f0 + sWindow > n) { f0 = n - sWindow; }
        if (f0 < 0) { f0 = 0; }
        for (int f = f0; f < f0 + sWindow && f < n; f++) {
            sCand[f].stickX += (int16_t)std::uniform_int_distribution<int>(-8, 8)(sRng);
            if (std::uniform_int_distribution<int>(0, 9)(sRng) == 0) {
                sCand[f].button ^= 0x8000; // A_BUTTON do Controller (bit15)
            }
        }
        Bf_SimBegin(sCand.data(), n, n + 3600);
        sPhase = 1;
        return;
    }
    int r = Bf_SimPoll();
    if (r == 0) { return; } // ainda simulando
    Bf_SimEnd();
    if (sBestTicks < 0 && r < 0) {
        // Baseline não terminou: base inválida, aborta com erro.
        sSearching = false; sPhase = 0;
        return; // chamador loga "BF base does not finish"
    }
    if (r > 0 && (sBestTicks < 0 || r < sBestTicks)) {
        sBestTicks = r;
        sBest = sCand;
        Bf_SetBase(sBest.data(), (int)sBest.size());
        Bf_SaveResult(Bf_ResultPath());
        // Log verde estilo TMInterface (SPDLOG_INFO aqui; cor vem do console).
    }
    sIter++;
    sPhase = 0;
}

int Bf_SaveResult(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) { return -1; }
    fprintf(f, "%d %d\n", sBestTicks, (int)sBest.size());
    for (size_t i = 0; i < sBest.size(); i++) {
        fprintf(f, "%d %d %u\n", sBest[i].stickX, sBest[i].stickY, (unsigned)sBest[i].button);
    }
    fclose(f);
    return 0;
}

int Bf_LoadResult(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) { return -1; }
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
        if (fscanf(f, "%d %d %u\n", &sx, &sy, &b) != 3) { fclose(f); return -1; }
        BfInput in;
        in.stickX = (int16_t)sx; in.stickY = (int16_t)sy; in.button = (uint16_t)b;
        v.push_back(in);
    }
    fclose(f);
    Bf_SetBase(v.data(), (int)v.size());
    sBestTicks = ticks;
    return 0;
}
```

Nota: `button ^= 0x8000` — no `struct Controller` o A é o bit 15 (`u16 button`, máscara A_BUTTON do jogo). Executor: confirmar `A_BUTTON` em `include/` (padrão N64: `0x8000`) e trocar pelo define real.

- [ ] **Step 3: UI Developer > Bruteforce + comandos definitivos**

Em `src/port/ui/PortMenu.cpp`, após a seção removida do Scattershot, adicionar (mesmo padrão de `AddWidget` das seções vizinhas):

```cpp
    path = { "Developer", "Bruteforce", SECTION_COLUMN_1 };
    AddSidebarEntry("Developer", "Bruteforce", 1);
    AddWidget(path, "Record Base", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) { Bf_RecordStart(); })
        .Options(ButtonOptions().Tooltip("Starts recording P1 inputs as the BF base (drive a finished race)"));
    AddWidget(path, "Stop Record", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) { Bf_RecordStop(); })
        .Options(ButtonOptions().Tooltip("Stops recording; base is ready"));
    AddWidget(path, "Start Search", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) { Bf_SearchStart(30, 200); })
        .Options(ButtonOptions().Tooltip("Hill-climb finish time from the recorded base, saves result.txt"));
    AddWidget(path, "Stop Search", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) { Bf_SearchStop(); })
        .Options(ButtonOptions().Tooltip("Stops search and restores pre-BF state"));
```

Comandos de console (consolidar andaimes das Tasks 2–4 aqui, remover duplicados): `bf_record <0|1>`, `bf_start <window> <iters>`, `bf_stop`, `bf_loadresult`. `Bf_SearchFrame()` deve ser chamado 1x por frame enquanto buscando — gancho: chamar do mesmo lugar onde o menu ticka por frame, ou do topo de `Bf_OverrideTick` (que já roda todo tick; guardar chamada 1x por frame via contador de frames). Recomendado: chamar `Bf_SearchFrame()` dentro de `Bf_OverrideTick` no máximo 1x por frame (comparar `gGlobalTimer` com último valor visto).

- [ ] **Step 4: Build + teste real**

Run: `cmake --build build -j$(nproc)` Expected: PASS.
Manual: gravar base (3 voltas time trial) → Start Search → observar log `BF new best` + `result.txt` no app dir → Stop → `bf_loadresult` + replay da base confirma mesmo tempo.

- [ ] **Step 5: Commit**

```bash
git add src/port/BfMutator.h src/port/BfMutator.cpp src/port/ui/PortMenu.cpp && git commit -m "feat: add BfMutator hill-climb, result.txt and Developer UI"
```

---

## Pós-plano

Após as 5 tasks: build Android (`./gradlew :app:assembleDebug` ou o comando que o repo usa), `adb install -r`, teste no device. Só então considerar o BF pronto.
