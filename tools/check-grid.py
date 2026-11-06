#!/usr/bin/env python3
"""
Check that every word in a panel sketch points at the right letters.

    python3 tools/check-grid.py WeatherStation/WeatherStation.ino
    python3 tools/check-grid.py            # every sketch in the repo

Reads the 11x10 letter grid from the sketch's header comment (the lines
that look like "r0  I T Q I S ..."), then every word definition:
    const LBWord SUNNY = {1, 0, 5};        (LBPanel sketches)
    #define W_IT 0, 0, 2                   (WordClock.ino)
and prints the letters each one lights. A word whose letters don't spell
its name is flagged. Names like IN_A_MEETING, BACK_AT or CI_FAIL are fine:
the underscores are dropped, and a match on the last part (FAIL) counts.
Exit code 1 if anything is flagged.

    python3 tools/check-grid.py --find RICH CryptoClock/CryptoClock.ino

answers "could this panel light RICH?": it searches every row of the
grid for those letters, side by side, and prints where they are (or that
they aren't there).
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GRID_LINE = re.compile(r"^\s*\*?\s*r(\d)\s+((?:[A-Z]\s+){10}[A-Z])\b")
LBWORD = re.compile(r"\b([A-Z][A-Z0-9_]*)\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}")
DEFINE = re.compile(r"^#define\s+([WMH]_[A-Z0-9_]+)\s+(\d+)\s*,\s*(\d+)\s*,\s*(\d+)", re.M)
PREFIXES = ("W_", "M_", "H_", "RP_")


def read_grid(src):
    rows = {}
    for line in src.splitlines():
        m = GRID_LINE.match(line)
        if m and int(m.group(1)) not in rows:
            rows[int(m.group(1))] = m.group(2).replace(" ", "")
    if sorted(rows) != list(range(10)) or any(len(r) != 11 for r in rows.values()):
        return None
    return [rows[i] for i in range(10)]


def name_ok(name, letters):
    for p in PREFIXES:
        if name.startswith(p):
            name = name[len(p):]
    parts = [re.sub(r"\d+$", "", p) for p in name.strip("_").split("_") if p]
    return letters in ("".join(parts), parts[-1] if parts else "")


def show(path):
    try:
        return path.relative_to(ROOT)
    except ValueError:
        return path


def check(path):
    src = path.read_text()
    grid = read_grid(src)
    if not grid:
        print(f"{show(path)}: no 11x10 grid found in the header comment")
        return 1
    words = LBWORD.findall(src) + DEFINE.findall(src)
    bad = 0
    print(f"{show(path)}")
    for name, r, c, n in words:
        r, c, n = int(r), int(c), int(n)
        if r > 9 or c + n > 11:
            print(f"  !! {name:14} {{{r}, {c}, {n}}}  runs off the grid")
            bad += 1
            continue
        letters = grid[r][c:c + n]
        ok = name_ok(name, letters)
        bad += not ok
        print(f"  {'  ' if ok else '!!'} {name:14} {{{r}, {c}, {n}}}  {letters}")
    print(f"  {len(words)} words, {bad} flagged\n")
    return bad


def find(word, path):
    grid = read_grid(path.read_text())
    if not grid:
        print(f"{show(path)}: no 11x10 grid found in the header comment")
        return 1
    word = word.upper().replace(" ", "")
    hits = [(r, row.find(word)) for r, row in enumerate(grid) if word in row]
    if hits:
        for r, c in hits:
            print(f"{show(path)}: {word} is at {{{r}, {c}, {len(word)}}}  (row {r}: {grid[r]})")
        return 0
    print(f"{show(path)}: {word} is not on this faceplate (its letters never sit side by side in one row)")
    return 1


if __name__ == "__main__":
    args = sys.argv[1:]
    if args[:1] == ["--find"]:
        if len(args) < 3:
            sys.exit("usage: check-grid.py --find WORD SKETCH.ino")
        sys.exit(find(args[1], Path(args[2]).resolve()))
    targets = [Path(a).resolve() for a in args] or sorted(ROOT.glob("*/*.ino"))
    sys.exit(1 if sum(check(p) for p in targets) else 0)
