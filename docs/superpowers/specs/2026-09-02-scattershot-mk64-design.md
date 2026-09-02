# Scattershot MK64 — Design Spec
**Date:** 2026-09-02
**Status:** Draft — approved in chat
**Scope:** External headless tool `tools/mk64-scattershot`, 1 track + 1 kart, optimizes lap time

## 1. Context & Goal
Port the SM64 TAS Scattershot bruteforcer (Krithalith / TylerKehne C++ port) to SpaghettiKart (MK64 PC port). The tool explores the MK64 state space by random-scattering inputs from seeded save-states, keeping the best lap times, and iterating. This spec covers the MVP: **one course (Mario Raceway) + one kart (Mario), external headless binary that outputs a replay viewable in-game**.

Non-goal (MVP): multi-course, multi-kart tuning, online distribution, video encode. Those are v2.

## 2. Architecture — Approach A (Headless Fork)

```
SpaghettiKart (src/ + libultraship)
  │
  ├─► libSpaghettify (SDL/OpenGL) — existing game
  └─► libSpaghettifyHeadless (HEADLESS=1, NO_RENDER, NO_AUDIO)
        ▲ linked by
tools/mk64-scattershot/
  ├─ MK64State       (save/load)
  ├─ ScattershotCore (GlobalState / Block / Segment / ThreadState)
  ├─ InputGen
  ├─ Evaluator       (lap time)
  └─ ReplayWriter    (best.mkr + best.json)
  Viewer patch in src/port (Debug > Scattershot > Load Replay)
```

Why A: reuses the real kart physics (same code as the game, deterministic), savestate is a memcpy of globals, maintenance is low when the decomp updates. B (copy physics) diverges, C (emulator) is slow and non-deterministic.

## 3. Components

### 3.1 MK64State — Savestate
- **Snapshot POD:** `struct MK64State { KartState karts[8]; CourseState course; RaceState race; uint32_t frame; uint32_t rngSeed; }`
- **KartState:** pos xyz (s32/f32 as in `src/racing/kart.h`), velocity xyz, angle yaw/pitch/roll, speed, lap, checkpoint, progress, `framesSinceStart`.
- **API:**
  ```cpp
  MK64State save_state();
  void load_state(const MK64State& s);
  uint64_t hashPos(const MK64State& s); // truncated pos+speed for hash table
  bool truncEq(const MK64State& a, const MK64State& b);
  ```
- Implementation: `save_state` copies `gKartStates`, `gRaceInfo`, `gCourseInfo`, `gRngSeed`. `load_state` restores them and calls `course_reinit_if_needed()`. Size < 64KB per state, fits in pool.

### 3.2 ScattershotCore — Search Engine
Ported from `TylerKehne/scattershot` with MK64 adaptation:

- **Segment:** `{ Segment* parent; int depth; int numFrames; MK64Input inputs[]; MK64State endState; float score; }`
- **Block:** `{ Vec3d pos == hashPos(endState); Segment* tailSeg; }` — `blockLength()` sums `numFrames` to root.
- **GlobalState:** `hashTab[maxHashes] -> block index`, `blocks[]`, `segments[]`, `std::mutex`, `bestBlock`, `bestFrames`.
- **ThreadState:** each worker loops:
  1. pick seed block (weighted by `1/score`, bias toward recent/best),
  2. `load_state(seed.endState)`,
  3. `scatter(scatterDepth=30 frames)`: for each frame `inp = InputGen::random() ; kart_tick(inp)`,
  4. after scatter, compute `pos = hashPos(currentState)`, `findBlock()`, if new block has better score (`Evaluator::score()` < existing), allocate `Segment` + `Block` and insert into hash.
- **Hash:** `xoro_r` seeded with truncated `x/100 + y/100 + z/100 + speed*10` (like SM64's `x,y,z,s`). 100 probes, `maxHashes = 1<<20`.

### 3.3 InputGen — MK64 Input Space
```cpp
struct MK64Input { s8 stick_x; s8 stick_y; bool A; bool B; bool Z; bool R; bool L; };
MK64Input random(Xoroshiro& rng);
```
- `stick_x` uniform -80..80 (favor 0..60 for turning), `stick_y` uniform -20..20,
- `A=true` 95% (acceleration), `R` drift 10% hold, `B` brake 2%.
- Configurable via `scattershot.toml` (`stick_range`, `drift_rate`).

### 3.4 Evaluator — Lap Time
- **Complete lap:** `score = framesToFinish` (lower is better). Finish when `karts[0].lap >= 1 && checkpoint == 0`.
- **Partial (did not finish within scatterDepth + maxFrames):** `score = 1e9 - progress`, where `progress = checkpoint*1e6 + distAlongSpline + speed*10`. This lets the search rank states that got further along the track even if they didn't finish.
- Primary metric for GlobalState ranking is `score`; `bestBlock` is the completed lap with minimal `frames`.

### 3.5 ReplayWriter & Viewer
- **Writer:** after search ends (time budget or target time), walk `bestBlock.tailSeg -> root`, concatenate all segment inputs into `std::vector<MK64Input> replay`. Write:
  - `out/best.mkr` — binary ghost format compatible with `src/data/ghost` (reuse `GhostData` serialization: header `{course=MARIO_RACEWAY, frames=N, inputs[] compressed as delta}`).
  - `out/best.json` — `{lapTime:"01:32.451", frames:5427, seed:42, scatterDepth:30, totalBlocks:14321, date:"2026-09-02"}`
- **Viewer:** patch `src/port/Game.cpp` / `src/port/ui`:
  - Menu `Debug > Scattershot > Load Replay` → file picker for `*.mkr` → installs a virtual controller that feeds `replay[i]` per frame instead of `SDL_GameController`.
  - HUD overlay: lap timer, inputs (stick cross), frame counter, `best vs current` delta.
  - Also CLI: `tools/mk64-scattershot --replay out/best.mkr --render` (build with SDL) to dump frames to `out/best.mp4` via `ffmpeg` pipe.

## 4. Build & Config

### 4.1 CMake
- New option `option(SCATTERSHOT_HEADLESS "Build headless lib for scattershot" OFF)`
- `add_library(SpaghettifyHeadless STATIC ${ALL_FILES})` with `target_compile_definitions(SpaghettifyHeadless PRIVATE HEADLESS=1)` — excludes `src/port/ui`, `src/audio` mixer SSE path still uses `sse2neon.h` but no SDL.
- `add_subdirectory(tools/mk64-scattershot)` builds `mk64-scattershot` executable, links `SpaghettifyHeadless`, `yaml-cpp`, `thread-pool` already in repo.
- Config file `tools/mk64-scattershot/scattershot.toml.example`:
  ```toml
  course = "mario_raceway"
  kart = "mario"
  threads = 8
  scatter_depth = 30
  max_frames_per_scatter = 600
  max_blocks = 1000000
  time_budget_sec = 300
  target_lap_ms = 90000
  ```

### 4.2 Determinism
- Fixed RNG seed per thread (`seed = base + threadId*0x9e3779b1`), `gRngSeed` saved in MK64State.
- Disable non-deterministic subsystems in HEADLESS: particles, sound, vsync. Ensure `course_collision` is pure.

## 5. Error Handling
- Hash table full (`findNewHashInx` fails after 100 probes) → drop weakest block (largest score) or grow table (double, rehash). Log `WARN hash table saturated`.
- `save_state/load_state` mismatch → `assert(memcmp)` in debug, returns partial state and aborts thread.
- Invalid `mkr` replay → viewer shows `Error: invalid replay header` and falls back to normal controls.

## 6. Testing Strategy
- **Unit:** `save/load` round-trip (tick 100 frames, save, tick 50, load, assert pos equal after re-tick 50).
- **Integration:** `course=mario_raceway, scatter_depth=1, threads=1, time_budget=5s` → assert `bestBlock != nullptr` and `replay.size() > 0`; reload `best.mkr` in headless ` --replay` and assert `lap time == best.json`.
- **Determinism:** run same seed twice, assert `bestBlocks` hash equal.
- **Viewer:** manual: load `test.mkr` (10s straight), verify kart follows line and timer matches.

## 7. Out of Scope (v2)
- Multi-track / multi-kart search, distributed workers over network, GPU rendering, `output-metadata.json` for CI, auto-upload to leaderboard.

## 8. Success Criteria (MVP)
- `bash tools/mk64-scattershot --course mario_raceway --time 60` finishes with `out/best.mkr` that, when loaded via `Debug > Load Replay`, shows a complete lap < 1:35 on Mario Raceway with Mario.
- Headless build compiles on Linux/Termux without SDL, runs at >1M frames/sec on 8 threads (measured via `eval/sec` counter).
- No regression: normal `Spaghettify` game builds and runs unchanged when `SCATTERSHOT_HEADLESS=OFF`.

