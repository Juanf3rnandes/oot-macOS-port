#!/usr/bin/env python3
"""
Checks that changes to src/ and include/ are neutral for the matching (ROM) build.

The ROM build never defines TARGET_NATIVE, so if every game .c file preprocesses to the exact same token stream
before and after a change, the original compilers (IDO/EGCS/GCC) receive identical input and produce an identical
ROM. This is a cheap proxy for `make VERSION=<v>` -> OK when no ROM or MIPS toolchain is available.
It does NOT replace the real matching build; run that too when you can (docs/port-macos/11-convencoes.md §1).

Usage:
    port/tools/check_matching_neutral.py [--base REF] [--jobs N] [--config NAME ...]

Missing headers (assets extracted from the ROM) are replaced by empty stubs in both trees, so the comparison
works without `make setup`.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]

IDO_BUILTINS = [
    "-D__sgi", "-D_LANGUAGE_C", "-Dmips", "-D_MIPSEB", "-D_MIPS_FPSET=16",
    "-D_MIPS_ISA_MIPS2=2", "-D_MIPS_ISA=_MIPS_ISA_MIPS2", "-D_ABIO32=1", "-D_MIPS_SIM=_ABIO32",
    "-D_MIPS_SZINT=32", "-D_MIPS_SZLONG=32", "-D_MIPS_SZPTR=32",
]

# name -> (version, defines)
CONFIGS = {
    "gc-eu-mq-dbg/ido": [
        "-DPLATFORM_N64=0", "-DPLATFORM_GC=1", "-DPLATFORM_IQUE=0",
        "-DOOT_VERSION=GC_EU_MQ_DBG", "-DOOT_REVISION=15", "-DOOT_REGION=REGION_EU",
        "-DLIBULTRA_VERSION=LIBULTRA_VERSION_L", "-DLIBULTRA_PATCH=0", "-DDEBUG_FEATURES=1",
        "-DF3DEX_GBI_2", "-DF3DEX_GBI_PL", "-DGBI_DOWHILE", "-DGBI_DEBUG",
    ] + IDO_BUILTINS,
    "ntsc-1.2/ido": [
        "-DPLATFORM_N64=1", "-DPLATFORM_GC=0", "-DPLATFORM_IQUE=0",
        "-DOOT_VERSION=NTSC_1_2", "-DOOT_REVISION=2", "-DOOT_REGION=REGION_US",
        "-DLIBULTRA_VERSION=LIBULTRA_VERSION_I", "-DLIBULTRA_PATCH=1", "-DDEBUG_FEATURES=0", "-DNDEBUG",
        "-DF3DEX_GBI_2",
    ] + IDO_BUILTINS,
    "gc-eu-mq-dbg/gcc": [
        "-DCOMPILER_GCC", "-DNON_MATCHING", "-DAVOID_UB",
        "-DPLATFORM_N64=0", "-DPLATFORM_GC=1", "-DPLATFORM_IQUE=0",
        "-DOOT_VERSION=GC_EU_MQ_DBG", "-DOOT_REVISION=15", "-DOOT_REGION=REGION_EU",
        "-DLIBULTRA_VERSION=LIBULTRA_VERSION_L", "-DLIBULTRA_PATCH=0", "-DDEBUG_FEATURES=1",
        "-DF3DEX_GBI_2", "-DF3DEX_GBI_PL", "-DGBI_DOWHILE", "-DGBI_DEBUG",
        "-D_LANGUAGE_C", "-D_MIPS_SZLONG=32", "-D__GNUC__=7",
    ],
}

INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def make_stubs(trees: list[Path], stub_dir: Path) -> None:
    """Creates empty headers for every quoted include that does not exist in any include path of any tree."""
    for tree in trees:
        for path in list((tree / "src").rglob("*.c")) + list((tree / "src").rglob("*.h")) + \
                list((tree / "include").rglob("*.h")):
            for inc in INCLUDE_RE.findall(path.read_text(errors="replace")):
                candidates = [path.parent / inc, tree / "include" / inc, tree / "include/libc" / inc,
                              tree / inc, tree / "src" / inc]
                if any(c.exists() for c in candidates):
                    continue
                stub = stub_dir / inc
                stub.parent.mkdir(parents=True, exist_ok=True)
                stub.touch()


def preprocess(tree: Path, src: Path, defines: list[str], stub_dir: Path) -> str:
    cmd = ["clang", "-E", "-P", "-xc", "-nostdinc", "-std=gnu89", "-fno-dollars-in-identifiers",
           "-Wno-everything", *defines,
           "-Iinclude", "-Iinclude/libc", "-Isrc", "-I.",
           f"-idirafter{stub_dir}", str(src)]  # relative paths keep __FILE__ identical across trees
    res = subprocess.run(cmd, cwd=tree, capture_output=True, text=True)
    if res.returncode != 0:
        return "<<PREPROCESS ERROR>>\n" + res.stderr
    return "\n".join(normalize(line) for line in res.stdout.splitlines() if line.strip())


# String/char literals are kept verbatim (they end up in the ROM); elsewhere whitespace next to punctuation is dropped
# so that only token changes count.
LITERAL_RE = re.compile(r'("(?:[^"\\\n]|\\.)*"|\'(?:[^\'\\\n]|\\.)*\')')
PUNCT_SPACE_RE = re.compile(r"\s*([^\w\s])\s*")


# In the ROM build these typedefs name the very same types (checked by check_typedef_aliases), so swapping one for the
# other cannot change the generated code. This is the project's standard 64-bit fix: `(u32)ptr` → `(uintptr_t)ptr`
# (docs/port-macos/11-convencoes.md §3).
TYPE_ALIASES = {"uintptr_t": "u32", "intptr_t": "s32"}
TYPE_ALIAS_RE = re.compile(r"\b(" + "|".join(TYPE_ALIASES) + r")\b")


def normalize(line: str) -> str:
    parts = LITERAL_RE.split(line.strip())
    for i in range(0, len(parts), 2):
        code = TYPE_ALIAS_RE.sub(lambda m: TYPE_ALIASES[m.group(1)], parts[i])
        parts[i] = PUNCT_SPACE_RE.sub(r"\1", re.sub(r"\s+", " ", code))
    return "".join(parts)


def check_typedef_aliases(tree: Path) -> None:
    """Asserts that TYPE_ALIASES really are identical types in the ROM build of `tree`."""
    stdint = (tree / "include/libc/stdint.h").read_text()
    types = (tree / "include/ultra64/ultratypes.h").read_text()
    expected = {
        "uintptr_t": (r"typedef\s+unsigned long\s+uintptr_t;", stdint),
        "intptr_t": (r"typedef\s+signed long\s+intptr_t;", stdint),
        "u32": (r"typedef\s+unsigned long\s+u32;", types),
        "s32": (r"typedef\s+signed long\s+s32;", types),
    }
    for name, (pattern, text) in expected.items():
        if not re.search(pattern, text):
            sys.exit(f"check_matching_neutral: typedef of {name} changed; TYPE_ALIASES is no longer valid")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--base", default="origin/main", help="git ref to compare the working tree against")
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    ap.add_argument("--config", action="append", choices=sorted(CONFIGS), help="defaults to all")
    ap.add_argument("--show", type=int, default=3, help="number of differing files to print a diff for")
    args = ap.parse_args()

    configs = args.config or sorted(CONFIGS)
    tmp = Path(tempfile.mkdtemp(prefix="oot-neutral-"))
    base_tree = tmp / "base"
    try:
        subprocess.run(["git", "worktree", "add", "--detach", "-q", str(base_tree), args.base], cwd=REPO, check=True)
        check_typedef_aliases(REPO)
        check_typedef_aliases(base_tree)
        stub_dir = tmp / "stubs"
        make_stubs([REPO, base_tree], stub_dir)

        sources = sorted(p.relative_to(REPO) for p in (REPO / "src").rglob("*.c"))
        sources = [s for s in sources if (base_tree / s).exists()]

        failures = 0
        for name in configs:
            defines = CONFIGS[name]

            def job(src: Path) -> tuple[Path, str, str]:
                return src, preprocess(base_tree, src, defines, stub_dir), preprocess(REPO, src, defines, stub_dir)

            changed = []
            errors = 0
            with concurrent.futures.ThreadPoolExecutor(args.jobs) as ex:
                for src, before, after in ex.map(job, sources):
                    if before.startswith("<<PREPROCESS ERROR>>") and after.startswith("<<PREPROCESS ERROR>>"):
                        errors += 1  # file does not preprocess standalone in either tree; not our concern
                        continue
                    if before != after:
                        changed.append((src, before, after))

            status = "OK" if not changed else f"{len(changed)} file(s) differ"
            print(f"[{name}] {len(sources)} files, {errors} skipped (preprocess error in both trees): {status}")
            for src, before, after in changed[: args.show]:
                print(f"--- {src}")
                diff = difflib.unified_diff(before.splitlines(), after.splitlines(), "base", "work", n=1, lineterm="")
                for i, line in enumerate(diff):
                    if i > 40:
                        print("    ...")
                        break
                    print("    " + line)
            for src, _, _ in changed[args.show:]:
                print(f"--- {src}")
            failures += len(changed)
        return 1 if failures else 0
    finally:
        subprocess.run(["git", "worktree", "remove", "--force", str(base_tree)], cwd=REPO)
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
