#!/usr/bin/env python3
"""
Render the six faceplates as SVG pictures for the READMEs and the
installer page, each lit with one example message in the colours the
firmware uses in Auto mode.

    python3 tools/render-faceplates.py        # writes docs/images/*.svg

The letter grids and word positions here are the same ones in each
sketch's header. If a faceplate changes, change it in both places.
"""
from pathlib import Path

OUT = Path(__file__).resolve().parent.parent / "docs" / "images"

WHITE = "#ffffff"
RED = "#ff2b1e"
GREEN = "#2edc55"
AMBER = "#ff9a14"
ORANGE = "#ff8c14"
BLUE = "#4b8cff"
COOLBLUE = "#58b4ff"
PINK = "#ff5a8c"
YELLOW = "#ffc800"

# id: (title, caption, grid rows, [(row, col, len, colour)], corner dots lit)
PANELS = {
    "wordclock": (
        "Word Clock", "IT IS TWENTY PAST FOUR · PM · 4:22",
        ["ITLISASAMPM", "ACQUARTERDC", "TWENTYFIVEX", "HALFSTENFTO", "PASTERUNINE",
         "ONESIXTHREE", "FOURFIVETWO", "EIGHTELEVEN", "SEVENTWELVE", "TENSEOCLOCK"],
        [(0, 0, 2, WHITE), (0, 3, 2, WHITE), (0, 9, 2, WHITE),
         (2, 0, 6, WHITE), (4, 0, 4, WHITE), (6, 0, 4, WHITE)],
        2,
    ),
    "weather": (
        "Weather Station", "IT IS CLOUDY · BREEZY · NOW · FEELS LIKE COOL",
        ["ITQISVZKDMR", "SUNNYCLOUDY", "RAINYSNOWYB", "WINDYFOGGYK", "STORMYCLEAR",
         "HOTCOLDMILD", "WARMCOOLDRY", "BREEZYCALMT", "TONIGHTNOWD", "FEELSVLIKER"],
        [(0, 0, 2, WHITE), (0, 3, 2, WHITE), (1, 5, 6, WHITE), (6, 4, 4, COOLBLUE),
         (7, 0, 6, WHITE), (8, 7, 3, WHITE), (9, 0, 5, WHITE), (9, 6, 4, WHITE)],
        0,
    ),
    "status": (
        "Status Board", "I AM IN A MEETING · ASK · OFFICE",
        ["IAMVBACKATR", "BUSYFREEKVD", "INAMEETINGQ", "ONACALLVNOW", "FOCUSKTIMEV",
         "LUNCHQBREAK", "SHHKPLEASEV", "KNOCKOKASKD", "SOONLATERVQ", "OFFICEHOMEK"],
        [(0, 0, 3, WHITE), (2, 0, 10, RED), (7, 7, 3, AMBER), (9, 0, 6, WHITE)],
        0,
    ),
    "couple": (
        "Couple Message Board", "ON THE WAY → OK",
        ["RONQTHEZWAY", "HOMETLSAFEP", "DLATEQSORRY", "CALLRMEVNOW", "COMERGETVME",
         "VDOWNSTAIRS", "ALMOSTQHERE", "DONEVWORKNP", "OKNOQMAYZBE", "LUNCHDINNER"],
        [(0, 1, 2, PINK), (0, 4, 3, PINK), (0, 8, 3, PINK), (8, 0, 2, GREEN)],
        0,
    ),
    "crypto": (
        "Crypto Word Clock", "MOON · ATH · FOMO · TO THE MOON · LOL",
        ["QZMOONDUMPK", "RKVATHQZDIP", "QFOMOZKFUDV", "QREKTZWHALE", "QZAPEKSENDV",
         "TOQTHEZMOON", "BUYQTHEZDIP", "QZRUGPULLKV", "QLOLRIPSAME", "QHODLZPANIC"],
        [(0, 2, 4, GREEN), (1, 3, 3, GREEN), (2, 1, 4, ORANGE),
         (5, 0, 2, BLUE), (5, 3, 3, BLUE), (5, 7, 4, BLUE), (8, 1, 3, YELLOW)],
        0,
    ),
    "devops": (
        "DevOps Word Clock", "CI FAIL · ONLINE · CPU HIGH",
        ["CIQPASSFAIL", "DEPLOYQLIVE", "QROLLBACKZV", "ONLINEQDOWN", "QCPUQMEMQZV",
         "QLOADQDISKV", "QZHIGHQOKZV", "BACKUPQFAIL", "QDBQAPIQWEB", "TRAFFICQERR"],
        [(0, 0, 2, WHITE), (0, 7, 4, RED), (3, 0, 6, GREEN), (4, 1, 3, AMBER), (6, 2, 4, AMBER)],
        0,
    ),
}

CELL = 56          # letter pitch
PAD = 44           # panel border
FONT = "'Helvetica Neue', Helvetica, Arial, 'Liberation Sans', sans-serif"


def panel(grid, lit, dots, ox=0, oy=0, uid="p"):
    """SVG fragment for one faceplate, top-left at (ox, oy)."""
    size = PAD * 2 + CELL * 11
    height = PAD * 2 + CELL * 10
    colour = {}
    for r, c, n, col in lit:
        for i in range(n):
            colour[(r, c + i)] = col
    out = [f'<g transform="translate({ox},{oy})">',
           f'<rect width="{size}" height="{height}" rx="18" fill="#0a0a0c"/>',
           f'<rect x="1" y="1" width="{size - 2}" height="{height - 2}" rx="17" fill="none" stroke="#24262c" stroke-width="2"/>']
    unlit, glow, sharp = [], [], []
    for r, row in enumerate(grid):
        for c, ch in enumerate(row):
            x = PAD + CELL * c + CELL / 2
            y = PAD + CELL * r + CELL / 2 + 11
            if (r, c) in colour:
                t = f'<text x="{x:.0f}" y="{y:.0f}" fill="{colour[(r, c)]}">{ch}</text>'
                glow.append(t)
                sharp.append(t)
            else:
                unlit.append(f'<text x="{x:.0f}" y="{y:.0f}">{ch}</text>')
    text = f'font-family="{FONT}" font-weight="700" font-size="30" text-anchor="middle"'
    out.append(f'<g {text} fill="#2b2d34">{"".join(unlit)}</g>')
    out.append(f'<g {text} filter="url(#glow-{uid})" opacity="0.9">{"".join(glow)}</g>')
    out.append(f'<g {text}>{"".join(sharp)}</g>')
    # The four corner minute dots.
    corners = [(PAD / 2, PAD / 2), (size - PAD / 2, PAD / 2),
               (size - PAD / 2, height - PAD / 2), (PAD / 2, height - PAD / 2)]
    for i, (cx, cy) in enumerate(corners):
        if i < dots:   # lit: a soft halo, then the dot itself
            out.append(f'<circle cx="{cx:.0f}" cy="{cy:.0f}" r="12" fill="{WHITE}" opacity="0.18"/>')
            out.append(f'<circle cx="{cx:.0f}" cy="{cy:.0f}" r="8" fill="{WHITE}" opacity="0.3"/>')
        out.append(f'<circle cx="{cx:.0f}" cy="{cy:.0f}" r="5" fill="{WHITE if i < dots else "#2b2d34"}"/>')
    out.append("</g>")
    return "\n".join(out), size, height


def glow_def(uid):
    return (f'<filter id="glow-{uid}" x="-50%" y="-50%" width="200%" height="200%">'
            f'<feGaussianBlur stdDeviation="7" result="b"/>'
            f'<feMerge><feMergeNode in="b"/><feMergeNode in="b"/></feMerge></filter>')


def write_single(pid):
    title, caption, grid, lit, dots = PANELS[pid]
    body, w, h = panel(grid, lit, dots, uid=pid)
    svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" '
           f'role="img" aria-label="{title} faceplate lit: {caption}">'
           f'<title>{title}: {caption}</title><defs>{glow_def(pid)}</defs>{body}</svg>\n')
    (OUT / f"{pid}.svg").write_text(svg)


def write_overview():
    cols, gap, label = 3, 40, 70
    parts, defs = [], []
    w = h = 0
    for i, (pid, (title, caption, grid, lit, dots)) in enumerate(PANELS.items()):
        body, pw, ph = panel(grid, lit, dots, uid=pid)
        x = (i % cols) * (pw + gap)
        y = (i // cols) * (ph + label + gap)
        defs.append(glow_def(pid))
        parts.append(f'<g transform="translate({x},{y})">{body}'
                     f'<text x="{pw / 2:.0f}" y="{ph + 34}" text-anchor="middle" font-family="{FONT}" '
                     f'font-weight="700" font-size="26" fill="#e9eaee">{title}</text>'
                     f'<text x="{pw / 2:.0f}" y="{ph + 62}" text-anchor="middle" font-family="{FONT}" '
                     f'font-size="17" fill="#9aa0ac">{caption}</text></g>')
        w = max(w, x + pw)
        h = max(h, y + ph + label)
    svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="-30 -30 {w + 60} {h + 60}" '
           f'width="{(w + 60) // 2}" height="{(h + 60) // 2}" role="img" '
           f'aria-label="The six Word Clock faceplates, each lit with an example">'
           f'<title>Six faces, one platform</title><defs>{"".join(defs)}</defs>'
           f'<rect x="-30" y="-30" width="{w + 60}" height="{h + 60}" rx="24" fill="#141519"/>'
           f'{"".join(parts)}</svg>\n')
    (OUT / "overview.svg").write_text(svg)


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    for pid in PANELS:
        write_single(pid)
    write_overview()
    print("wrote", ", ".join(sorted(p.name for p in OUT.glob("*.svg"))))
