# Lesson 4 — Buttons (MicroPython)

**Goal:** read the onboard COLOR button and use it to cycle the panel's colour, instead of a fixed timer.

## What you'll learn

- The two buttons' wiring: `GPIO 42` (COLOR) and `GPIO 41` (EFFECT), both **active-high** — they read `1` when pressed, `0` when released
- Why you need **debouncing**: a mechanical button "bounces" for a few milliseconds when pressed, which a naive read would see as several rapid presses
- **Edge detection**: reacting once on the moment a button goes from released → pressed, not on every loop iteration while it's held down
- Wrapping that state in a small `Debouncer` class instead of loose global variables — useful once you want to debounce more than one button

## The code

See [`main.py`](main.py).

```python
class Debouncer:
    def __init__(self, pin, debounce_ms=DEBOUNCE_MS):
        self.pin = pin
        self.debounce_ms = debounce_ms
        self.prev = False
        self.last_t = 0

    def pressed(self):
        now = self.pin.value() == 1
        fired = False
        if now != self.prev and time.ticks_diff(time.ticks_ms(), self.last_t) >= self.debounce_ms:
            self.last_t = time.ticks_ms()
            if now:
                fired = True
            self.prev = now
        return fired
```

## Line by line

- `self.pin.value() == 1` — these buttons are wired active-high with an internal pull-down, so `1` means pressed, `0` means released.
- `now != self.prev` — only consider this button if its reading actually *changed* since last time.
- `time.ticks_diff(time.ticks_ms(), self.last_t) >= self.debounce_ms` — but only trust that change if enough time (40ms) has passed since the last accepted change. `time.ticks_ms()`/`time.ticks_diff()` are MicroPython's wraparound-safe millisecond timer functions (safer than plain subtraction for long-running boards). This filters out the rapid on/off/on chatter a physical switch produces in the first few milliseconds of a press.
- `if now: fired = True` — of the two ways a debounced change can happen (release→press or press→release), we only care about the press (rising edge).
- Each `Debouncer` instance keeps its own `prev`/`last_t` state, so `color_button.pressed()` can be called every loop iteration and only returns `True` for exactly one of those calls per physical press, no matter how long the button is held.

## What you should see

The panel starts white. Each time you press the COLOR button (GPIO 42), the whole panel cycles to the next colour: white → red → blue → green → back to white. The REPL/Serial output logs each mode change.

## Troubleshooting

- **Multiple colour jumps per press**: increase `DEBOUNCE_MS`.
- **Button seems unresponsive**: confirm you're pressing the COLOR button, not EFFECT — this lesson only wires up COLOR (EFFECT is read but unused until the full firmware).

## Next

[Lesson 5 — WiFi Connect](../05_WiFiConnect/README.md): get the board online, which is the last piece needed before the panel can tell real time.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
