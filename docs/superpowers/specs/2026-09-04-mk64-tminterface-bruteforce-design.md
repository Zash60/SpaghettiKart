# MK64 Bruteforce estilo TMInterface — Design

Data: 2026-09-04. Ref: https://donadigo.com/tminterface/what-is-bf

## Objetivo

Remover todo o Scattershot e adicionar um bruteforce integrado ao jogo,
modelado no "What is Bruteforce?" do TMInterface: replay base que termina
a corrida + simulação rápida + save states + mutação automática de inputs,
otimizando o tempo final. Resultado em `result.txt`, carregável no jogo.

## Decisões do usuário

- Remoção: TUDO do Scattershot (tool, CI, hooks no jogo, docs).
- BF: integrado no jogo (como o TMInterface, não tool externa).
- Base: sistema de replay do próprio jogo (`src/replays.c/h`).
- Alvo: só tempo final (sem checkpoints/triggers nesta fase).
- Abordagem A: `gTickLogic` p/ sim rápida + snapshot/restore + aba no menu.

## Seção 1 — Remoção do Scattershot

Apagar:

- `tools/mk64-scattershot/` (inteiro)
- `.github/workflows/scattershot.yml`
- `src/port/ScattershotReplay.h`, `src/port/ScattershotReplay.cpp`
- Hooks: `src/racing/race_logic.c` (autoload/override), `src/engine/objects/Lakitu.cpp`
  (autoload no Go), `src/port/ui/PortMenu.cpp` (seção Scattershot)
- Docs: `docs/superpowers/specs/2026-09-02-scattershot-mk64-design.md`,
  `docs/superpowers/plans/2026-09-02-scattershot-mk64.md`

Manter: `src/replays.c/h` (vira a base do BF), `gTickLogic` + `GameSpeed`
em `src/port/ui/Tools.cpp` (vira a simulação rápida).

Verificação: `grep -ri scatter` vazio + build limpo desktop e Android.
Um commit só de remoção, antes de qualquer código novo.

## Seção 2 — Arquitetura (`src/port/bf/`)

Quatro unidades, interfaces estreitas:

1. `BfBase` — carrega o replay do jogo como vetor de inputs por frame
   (steering/throttle/botões). Valida pré-requisito TMInterface: o replay
   PRECISA terminar a corrida; senão, erro explícito e nada roda.
2. `BfSimRunner` — durante o BF, eleva `gTickLogic` (N ticks por frame
   renderizado) e injeta o input do candidato em `gControllers[0]` a cada
   tick. Ao sair, restaura `gTickLogic` e o estado pré-BF.
3. `BfSaveStates` — anel de slots com snapshot/restore do estado de corrida:
   `gPlayers`, timers de corrida/volta, contadores de volta, índices de path
   (`gPathIndexByPlayerId`, `gNearestPathPointByPlayerId`), RNG se aplicável.
   Cada slot tem magic + frame para validar o restore. Mutação no frame F
   restaura o slot mais próximo (não re-simula do zero) — igual ao TMInterface.
4. `BfMutator` — hill-climbing por segmentos: varia inputs em janelas de
   frames, re-simula, mantém o que baixa o tempo final. Novo melhor vira a
   base, salva `result.txt` (inputs + tempo) e imprime linha verde no console
   (`New best: 24.32, iterations: 14`, padrão TMInterface).

UI: aba "Bruteforce" no menu Developer (Start/Stop, replay base, tamanho da
janela de mutação, `gTickLogic`, `load result`). Stop/Esc a qualquer momento.

Fora de escopo (fase 2): checkpoints, triggers de posição, eval custom via API.

## Seção 3 — Fluxo, erros, testes

Fluxo: Start → valida base (termina?) → snapshot inicial → loop
{restaura slot → muta janela → simula até finish/timeout → compara} →
melhor salva `result.txt`.

Erros: timeout por iteração (kart emperrado não trava o jogo); restore com
magic inválido aborta a iteração, não o BF; sair do BF sempre restaura o
estado pré-BF (jogo nunca fica corrompido).

Testes:

- Determinismo: mesmos inputs ⇒ mesmo tempo (prova snapshot fiel).
- Mutação nula não altera o tempo.
- Replay que não termina ⇒ erro claro, BF nem inicia.
- Pós-remoção: nenhum `scatter` no tree + builds desktop/Android passam.

## Ordem de implementação

1. Commit de remoção (+ verificação de build).
2. `BfSaveStates` + teste de determinismo.
3. `BfBase` (leitura do replay + validação de finish).
4. `BfSimRunner` (injeção + `gTickLogic`).
5. `BfMutator` + `result.txt` + UI Developer.
