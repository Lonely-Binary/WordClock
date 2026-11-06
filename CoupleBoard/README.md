# 💌 Couple Message Board

*ESP32-S3 Blueprints No.01 — Word Clock, Couple Message Board face*

Two panels, one in each place. Tap **ON THE WAY** on your phone and it lights up on your partner's panel a moment later. They answer **OK**, **NO** or **MAYBE** with a button on the panel, and the answer lights up on yours.

**→ [Install it from your browser](https://lonely-binary.github.io/WordClock/)** (Chrome or Edge, press *Install Couple Message Board*, on both panels)

## The faceplate

```
  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      R  O  N  Q  T  H  E  Z  W  A  Y    ON THE WAY
  r1      H  O  M  E  T  L  S  A  F  E  P    HOME SAFE
  r2      D  L  A  T  E  Q  S  O  R  R  Y    LATE SORRY
  r3      C  A  L  L  R  M  E  V  N  O  W    CALL ME (NOW)
  r4      C  O  M  E  R  G  E  T  V  M  E    COME GET ME
  r5      V  D  O  W  N  S  T  A  I  R  S    DOWNSTAIRS
  r6      A  L  M  O  S  T  Q  H  E  R  E    ALMOST HERE
  r7      D  O  N  E  V  W  O  R  K  N  P    DONE WORK
  r8      O  K  N  O  Q  M  A  Y  Z  B  E    OK | NO | MAY BE   (replies)
  r9      L  U  N  C  H  D  I  N  N  E  R    LUNCH? | DINNER?
```

Messages: ON THE WAY, HOME SAFE, LATE SORRY, CALL ME, CALL ME NOW, COME GET ME, DOWNSTAIRS, ALMOST HERE, DONE WORK, LUNCH, DINNER. Replies: OK, NO, MAYBE.

## Pairing the two panels

Both panels need the **same pair code**. That's how they find each other.

1. Install the firmware on both panels and power them from their bases.
2. Set up the first panel: join its WiFi network **`CoupleBoard-XXXX`** from your phone, pick your home WiFi, and leave **Pair code** empty. It makes one up for itself.
3. Open the first panel's control page, **`http://coupleboard-xxxx.local`**, and copy the pair code shown at the bottom.
4. Set up the second panel the same way, but paste that code into **Pair code**. If it's already set up, open its control page → **Settings** and paste it there.

The panels can be on different WiFi networks in different cities. They only need the internet.

## Day to day

- **Send** from the control page on your phone (add it to your home screen). What you sent shows **dim** on your own panel, and your partner's answer lights up next to it when it arrives.
- **Receive**: the panel sparkles, then shows the message **bright**. Answer with the buttons, or from the control page for MAYBE.
- A conversation clears itself after **6 hours**, or when either of you sends a new message.
- If the panel can't reach the message server, the first corner dot glows dim red.

## Buttons

| | While a message from your partner is waiting | Otherwise |
|---|---|---|
| **A** | Answer **OK** | Change colour |
| **B** | Answer **NO** | Clear the panel |
| **Both for 3 seconds** | Forget WiFi and pair code, back to setup | same |

## Privacy and limits

Messages travel through the public MQTT broker **broker.emqx.io**, encrypted (TLS), on topics under `lonelybinary/couple/<pair code>/`. The pair code is the only secret: anyone who knows it can send to your panels, so share it only with your partner. The default code is 10 random characters. Change it in Settings on both panels if it ever leaks.

A message sent while the other panel is switched off or offline is **not** delivered later. Only messages that arrive while it's online show up.

## Building from source

Open `CoupleBoard.ino` in the Arduino IDE. You need the `libraries/LBPanel` folder from this repo copied into your Arduino `libraries` folder, plus FastLED, WiFiManager, PubSubClient and ArduinoJson from the Library Manager. Board **ESP32S3 Dev Module**, **Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)**. See the [root README](../README.md#building-from-source).

Message format, if you want to send from something else: publish `{"from":"me","m":"on_the_way"}` to `lonelybinary/couple/<pair code>/msg`, or `{"from":"me","r":"ok"}` to `.../reply`.
