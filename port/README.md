# `port/` — build nativo para macOS

Código e build do port nativo. A arquitetura, as decisões e o plano estão em
[`docs/port-macos/`](../docs/port-macos/README.md); este arquivo só explica **como compilar e onde está cada coisa**.

## Pré-requisitos

- macOS 14+ (Apple Silicon), Xcode Command Line Tools (`xcode-select --install`)
- `brew install cmake ninja sdl3 python3`
- Rede na primeira configuração (o CMake baixa o GoogleTest com hash fixo)

## Compilar e testar

```sh
cmake -S port --preset debug         # ou: cmake -S port -B build-native -G Ninja -DOOT_VERSION=gc-eu-mq-dbg
cmake --build build-native
ctest --test-dir build-native        # testes unitários: não precisam de ROM nem de janela
```

Presets (`port/CMakePresets.json`, rodar `cmake --build/ctest --preset` de dentro de `port/`): `debug`, `asan`
(AddressSanitizer + UBSan), `ubsan`, `release`.

> **ASan:** em 2026-11-08, com Apple clang 17 no macOS 26 (Darwin 25.6), qualquer binário com
> `-fsanitize=address` — até um "hello world" — trava na inicialização. É do toolchain/SO, não do port. Até isso ser
> resolvido, use o preset `ubsan`.

### Com a ROM (necessário a partir do M2)

```sh
cp <sua-rom>.z64 baseroms/gc-eu-mq-dbg/baserom.z64
make setup VERSION=gc-eu-mq-dbg      # extrai os assets para extracted/gc-eu-mq-dbg/
cmake -S port -B build-native -G Ninja
```

Sem `extracted/<versão>/`, o CMake avisa e compila só as fontes do jogo que não dependem de assets.

### Compilar todo o código do jogo (trabalho do M1)

A biblioteca `oot_game` ainda não faz parte de `all`:

```sh
cmake --build build-native --target oot_game -- -k 0 2>&1 | tee build.log
```

## Estrutura

| Caminho | Camada ([04 §2](../docs/port-macos/04-arquitetura-do-port.md#2-visão-em-camadas)) | Conteúdo |
|---------|------|----------|
| `CMakeLists.txt`, `cmake/` | — | Build. `GameConfig.cmake` espelha a tabela de versões do Makefile; `GameSources.cmake` lista as fontes do jogo a partir do `spec/` |
| `include/port/` | 3 | API C do port: `port.h` (`PORT_ASSERT`, `PORT_UNIMPLEMENTED`), `endian.h`, `log.h`, `platform.h` |
| `include/libc_compat/` | 1↔2 | Substitui `include/libc/` no build nativo (`math.h`, `assert.h`, `alloca.h`) |
| `src/ultra/` | 2 | Shim do libultra: filas de mensagens, tempo/timers, threads, dispositivos, `gu`, libm do jogo, `_Printf` |
| `src/core/` | 3 | Log (e, nos próximos marcos, `main`, loop, config, crash handler) |
| `tests/unit/` | — | GoogleTest. Os testes em C++ acessam o código do jogo por funções C em `game_helpers.c` |
| `tools/` | — | Scripts (abaixo) |

## Ferramentas

| Script | Para quê |
|--------|----------|
| `tools/check_matching_neutral.py` | Garante que mudanças em `src/`/`include/` não alteram o build *matching*: pré-processa os 957 `.c` com as flags do build da ROM (IDO gc-eu-mq-dbg, IDO ntsc-1.2, GCC), antes e depois, e compara os tokens. Use em todo PR que toca `src/` ou `include/`. Não substitui `make VERSION=<v>` → `OK`; é o que dá para checar sem ROM/toolchain MIPS |
| `tools/gen_game_sources.py` | Gera a lista de fontes a partir do `spec/` pré-processado (chamado pelo CMake) e marca as que dependem de assets extraídos |
| `tools/fix_pointer_casts.py` | Aplica a correção padrão `(u32)ptr` → `(uintptr_t)ptr` nos locais exatos apontados pelos avisos `-Wpointer-to-int-cast` de um log de build |

## Como o código do jogo é compilado

Flags e o motivo de cada uma: [05 §6](../docs/port-macos/05-problemas-de-portabilidade.md#6-compilador-cpu-e-comportamento-indefinido-ub).
Resumo: `-DTARGET_NATIVE=1 -DNON_MATCHING -DAVOID_UB -funsigned-char -fno-strict-aliasing -fwrapv -ffp-contract=off`.
Os headers do jogo entram com `-iquote` (o jogo só usa `#include "..."`), porque `include/sched.h`, `include/printf.h` e
`include/vis.h` têm o mesmo nome de headers do SDK do macOS.

Avisos conhecidos e a linha de base: [`WARNINGS.md`](WARNINGS.md).
