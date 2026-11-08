#!/usr/bin/env python3
"""
Applies the project's standard 64-bit fix to pointer→integer casts reported by clang
(docs/port-macos/05-problemas-de-portabilidade.md §2.1, 11-convencoes.md §3):

    (u32)ptr  →  (uintptr_t)ptr
    (s32)ptr  →  (intptr_t)ptr

In the ROM build uintptr_t/intptr_t are the same types as u32/s32, so the fix is neutral for matching; verify with
port/tools/check_matching_neutral.py anyway. Only casts written exactly as `(u32)` / `(s32)` (also `(unsigned int)`,
`(int)`) at the column clang reports are touched; anything else is listed for manual review.

Usage:
    cmake --build build-native --target oot_game -- -k 0 2>&1 | tee build.log
    port/tools/fix_pointer_casts.py build.log [--only src/code/z_DLF.c ...] [--dry-run]
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]

WARNING_RE = re.compile(
    r"^(?P<file>[^:\s]+):(?P<line>\d+):(?P<col>\d+): warning: cast to smaller integer type .*"
    r"\[-W(?:void-)?pointer-to-int-cast\]$"
)

REPLACEMENTS = {
    "(u32)": "(uintptr_t)",
    "(s32)": "(intptr_t)",
    "(unsigned int)": "(uintptr_t)",
    "(int)": "(intptr_t)",
}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log", help="build log with clang warnings")
    ap.add_argument("--only", action="append", help="restrict to these repo-relative files")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    sites: dict[Path, set[tuple[int, int]]] = defaultdict(set)
    for raw in Path(args.log).read_text(errors="replace").splitlines():
        m = WARNING_RE.match(raw.strip())
        if not m:
            continue
        path = Path(m["file"]).resolve()
        try:
            rel = path.relative_to(REPO)
        except ValueError:
            continue
        if args.only and str(rel) not in args.only:
            continue
        if not str(rel).startswith("src/"):
            continue  # headers need a human decision (they are shared by many files)
        sites[path].add((int(m["line"]), int(m["col"])))

    manual = []
    fixed = 0
    for path, locs in sorted(sites.items()):
        lines = path.read_text().split("\n")
        # Right to left so earlier columns stay valid.
        for line_no, col in sorted(locs, key=lambda lc: (lc[0], -lc[1])):
            text = lines[line_no - 1]
            idx = col - 1
            for old, new in REPLACEMENTS.items():
                if text.startswith(old, idx):
                    lines[line_no - 1] = text[:idx] + new + text[idx + len(old):]
                    fixed += 1
                    break
            else:
                manual.append(f"{path.relative_to(REPO)}:{line_no}:{col}: {text.strip()}")
        if not args.dry_run:
            path.write_text("\n".join(lines))

    print(f"fix_pointer_casts: {fixed} cast(s) {'would be ' if args.dry_run else ''}fixed in {len(sites)} file(s)")
    if manual:
        print("needs manual review:")
        for m in manual:
            print("  " + m)
    return 0


if __name__ == "__main__":
    sys.exit(main())
