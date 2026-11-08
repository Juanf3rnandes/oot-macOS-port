# Linha de base de avisos do código do jogo

Critério de saída do M1 ([09](../docs/port-macos/09-plano-de-implementacao.md#m1--compila-e-linka)): a contagem de avisos
de portabilidade é registrada aqui e **não pode crescer**. Cada aviso de ponteiro↔inteiro é um bug potencial de 64 bits
([05 §2](../docs/port-macos/05-problemas-de-portabilidade.md#2-ponteiros-de-64-bits)).

Como medir (build limpo da biblioteca do jogo):

```sh
ninja -C build-native -t clean oot_game
cmake --build build-native --target oot_game -- -k 0 2>&1 | tee build.log
grep -E "warning:" build.log | sed -E 's/.*\[(-W[^]]+)\]$/\1/' | sort | uniq -c | sort -rn
```

## 2026-11-08 — gc-eu-mq-dbg, sem assets extraídos (269 de 668 fontes)

Todas as 269 fontes compilam. As 399 restantes dependem de `extracted/<versão>/` (ROM).

| Aviso | Qtd | Onde | Situação |
|-------|----:|------|----------|
| `-Wpointer-to-int-cast` | 40 | `src/audio/internal/load.c` (39), `sched.c` (1) | Áudio: endereços de ROM de áudio em `u32` — depende do desenho da ROM virtual de áudio (M5). `sched.c` é substituído no M2 |
| `-Wvoid-pointer-to-int-cast` | 17 | `src/audio/` (16), `sched.c` (1) | Idem |
| `-Wint-to-pointer-cast` | 15 | `src/audio/` (14), `fault_gc.c` (1) | Idem; `Fault_ResumeThread` reescreve instruções MIPS — o Fault é substituído no M2 |
| `-Wint-to-void-pointer-cast` | 15 | `src/audio/` (11), `fault_gc.c` (3), `game.c` (1) | Idem; `game.c:151` faz *hexdump* de um endereço KSEG0 (só via registrador de debug) |
| `-Warray-bounds` | 23 | `src/code/db_camera.c` | Câmera de debug; revisar com ASan quando o jogo rodar |
| `-Wconstant-conversion` | 16 | `z_camera_data.inc.c`, `z_player_lib.c`, `z_en_encount1.c` | Intencional: constantes > 0x7FFF em `s16` dão a volta igual ao N64 |
| `-Wreturn-type` | 12 | `z_camera.c` (7), `audio/internal/load.c` (5) | `z_camera.c`: os chamadores descartam o valor, então não é UB em C. `load.c`: revisar no M5 |
| `-Wtrigraphs`, `-Wpointer-bool-conversion`, `-Wparentheses-equality` | 3 | `z_sram.c`, `db_camera.c`, `z_lights.c` | Inofensivos |

Já corrigidos nesta etapa (padrão `(u32)ptr` → `(uintptr_t)ptr`, neutro para o *matching*):
`__osMalloc_gc.c` (alocador, 42), `z_DLF.c` (overlays, 45), `runtime.c` (14), `fault_gc.c`, `game.c`,
`z_ss_sram.c`, `z_camera.c`, `debug.c`.

Ainda **não** medidos: `-Wshorten-64-to-32` e `-Wformat` (muito ruidosos; ligar quando os de ponteiro zerarem).
