# Lesson 4 — Buttons

**Goal:** read the onboard COLOR button and use it to cycle the panel's colour, instead of a fixed timer.

## What you'll learn

- The two buttons' wiring: `GPIO 42` (COLOR) and `GPIO 41` (EFFECT), both **active-high** — they read `HIGH` when pressed, `LOW` when released
- Why you need **debouncing**: a mechanical button "bounces" for a few milliseconds when pressed, which a naive read would see as several rapid presses
- **Edge detection**: reacting once on the moment a button goes from released → pressed, not on every loop iteration while it's held down

## The code

See [`04_Buttons.ino`](04_Buttons.ino).

```cpp
bool pressedEdge(uint8_t pin, bool &prev, uint32_t &tLast) {
  bool now   = (digitalRead(pin) == HIGH);
  bool fired = false;
  if (now != prev && (millis() - tLast) >= DEBOUNCE_MS) {
    tLast = millis();
    if (now) fired = true;   // rising edge = press
    prev = now;
  }
  return fired;
}
```

## Line by line

- `digitalRead(pin) == HIGH` — these buttons are wired active-high with an internal pull-down, so `HIGH` means pressed, `LOW` means released.
- `now != prev` — only look at this button if its reading actually *changed* since last time.
- `(millis() - tLast) >= DEBOUNCE_MS` — but only trust that change if enough time (40ms) has passed since the last accepted change. This filters out the rapid HIGH/LOW/HIGH chatter a physical switch produces in the first few milliseconds of a press.
- `if (now) fired = true` — of the two ways a debounced change can happen (release→press or press→release), we only care about the press (rising edge), so `fired` is only set when `now` is `HIGH`.
- In `loop()`, `pressedEdge(...)` is called every ~10ms. It returns `true` for exactly one of those calls per physical press, no matter how long the button is held.

## What you should see

The panel starts white. Each time you press the COLOR button (GPIO 42), the whole panel cycles to the next colour: white → red → blue → green → back to white. The Serial Monitor (115200 baud) logs each mode change.

## Troubleshooting

- **Multiple colour jumps per press**: increase `DEBOUNCE_MS`.
- **Button seems unresponsive**: confirm you're pressing the COLOR button, not EFFECT — this lesson only wires up COLOR (EFFECT is read but unused until the full firmware).

## Next

[Lesson 5 — WiFi Connect](../05_WiFiConnect/README.md): get the board online, which is the last piece needed before the panel can tell real time.

---

*Part of the* **Lonely Binary** *Word Clock tutorial series — thank you for supporting us and building with our hardware.*
