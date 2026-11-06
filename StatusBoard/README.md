# 💼 Status Board

*ESP32-S3 Blueprints No.01 — Word Clock, Status Board face*

A door sign for your desk: **I AM IN A MEETING … ASK … BACK AT … OFFICE**. Set it from your phone, from a script or shortcut, or with a button on the panel. No account, no cloud: it all stays on your home network.

**→ [Install it from your browser](https://lonely-binary.github.io/WordClock/)** (Chrome or Edge, press *Install Status Board*)

## The faceplate

```
  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      I  A  M  V  B  A  C  K  A  T  R    I AM   BACK AT
  r1      B  U  S  Y  F  R  E  E  K  V  D    BUSY | FREE
  r2      I  N  A  M  E  E  T  I  N  G  Q    IN A MEETING
  r3      O  N  A  C  A  L  L  V  N  O  W    ON A CALL   NOW
  r4      F  O  C  U  S  K  T  I  M  E  V    FOCUS TIME
  r5      L  U  N  C  H  Q  B  R  E  A  K    LUNCH BREAK
  r6      S  H  H  K  P  L  E  A  S  E  V    SHH PLEASE
  r7      K  N  O  C  K  O  K  A  S  K  D    KNOCK | OK | ASK
  r8      S  O  O  N  L  A  T  E  R  V  Q    SOON | LATER
  r9      O  F  F  I  C  E  H  O  M  E  K    OFFICE | HOME
```

## What you can show

Four independent choices. Each can be on or off:

| Choice | Options | Lights |
|---|---|---|
| **I am** | `busy` `free` `meeting` `call` `call_now` `focus` `lunch` `shh` | BUSY, FREE, IN A MEETING, ON A CALL, ON A CALL NOW, FOCUS TIME, LUNCH BREAK, SHH PLEASE |
| **Come in?** | `knock` `ok` `ask` | KNOCK, OK, ASK |
| **Back** | `soon` `later` `back_at` | SOON, LATER, BACK AT |
| **Where** | `office` `home` | OFFICE, HOME |

**I AM** lights by itself whenever it reads as a sentence (I AM BUSY, I AM HOME). In **Auto** colour, busy states glow red, FREE green, FOCUS TIME and LUNCH BREAK amber. The panel remembers your status through a power cut.

## Setup

1. Install the firmware and power the unit from the base.
2. On your phone, join the open WiFi network **`StatusBoard-XXXX`**. A setup page opens (or go to `http://192.168.4.1`).
3. Pick your home WiFi. That's all it needs.

## Control page

Open **`http://statusboard-xxxx.local`** (the same `XXXX` as the setup network) from any phone or computer on the same WiFi. Tap a button to switch it on, and tap it again to switch it off. Add it to your phone's home screen for one-tap access.

## From a script or shortcut

Every choice is one HTTP request, so a calendar script, an iOS Shortcut, Home Assistant or a hotkey can drive it:

```bash
curl "http://statusboard-xxxx.local/api/set?status=meeting&door=ask"
curl "http://statusboard-xxxx.local/api/set?status=call_now"
curl "http://statusboard-xxxx.local/api/set?status=&door=&back=soon"   # blank = off
```

Only the choices you name change. The reply is the full state as JSON, and an unknown value returns `400` with the reason.

## Buttons

- **A**: change colour (Auto → White → Red → Blue → Green → Rainbow, remembered)
- **B**: step through Free → Busy → In a meeting → On a call → Focus time → Lunch break → off
- **Both for 3 seconds**: forget WiFi and status, back to setup

## Building from source

Open `StatusBoard.ino` in the Arduino IDE. You need the `libraries/LBPanel` folder from this repo copied into your Arduino `libraries` folder, plus FastLED, WiFiManager and ArduinoJson from the Library Manager. Board **ESP32S3 Dev Module**, **Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)**. See the [root README](../README.md#building-from-source).
