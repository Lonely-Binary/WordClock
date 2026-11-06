# 🖥 DevOps Word Clock

*ESP32-S3 Blueprints No.01 — Word Clock, DevOps Word Clock face*

Your build pipeline and servers, on your desk: **CI FAIL**, **DEPLOY LIVE**, **DB DOWN**, **CPU HIGH**. It doesn't watch anything by itself. Your own tooling (GitHub Actions, a cron job, Uptime Kuma, a backup script) tells it what happened, with one HTTP request or one MQTT message.

**→ [Install it from your browser](https://lonely-binary.github.io/WordClock/)** (Chrome or Edge, press *Install DevOps Word Clock*)

## The faceplate

```
  col ->  0  1  2  3  4  5  6  7  8  9 10
  r0      C  I  Q  P  A  S  S  F  A  I  L    CI  PASS | FAIL
  r1      D  E  P  L  O  Y  Q  L  I  V  E    DEPLOY LIVE
  r2      Q  R  O  L  L  B  A  C  K  Z  V    ROLLBACK
  r3      O  N  L  I  N  E  Q  D  O  W  N    ONLINE | DOWN *
  r4      Q  C  P  U  Q  M  E  M  Q  Z  V    CPU  MEM
  r5      Q  L  O  A  D  Q  D  I  S  K  V    LOAD  DISK
  r6      Q  Z  H  I  G  H  Q  O  K  Z  V    HIGH *  OK *
  r7      B  A  C  K  U  P  Q  F  A  I  L    BACKUP FAIL
  r8      Q  D  B  Q  A  P  I  Q  W  E  B    DB  API  WEB
  r9      T  R  A  F  F  I  C  Q  E  R  R    TRAFFIC  ERR
```

\* **DOWN**, **HIGH** and **OK** are shared. They pair with a label on another row to make **DB DOWN**, **CPU HIGH**, **DISK OK**. When several are true at once, the panel shows one pair at a time and rotates every 4 seconds, so you never see an ambiguous "CPU MEM HIGH".

## What you can tell it

| Kind | Values | Shows | How long |
|---|---|---|---|
| **Event** | `ci_pass` | CI PASS | 60 seconds |
| | `ci_fail` | CI FAIL | until the next CI event, or button B |
| | `deploy_live` | DEPLOY LIVE | 60 seconds |
| | `rollback` | ROLLBACK | until the next deploy event, or button B |
| | `backup_fail` / `backup_ok` | BACKUP FAIL / clears it | until cleared |
| | `clear` | clears all of the above | |
| **Server** | `online` / `down` | ONLINE / DOWN (whole server down hides everything else below) | until changed |
| **Service** | `db`, `api`, `web` = `down` / `up` | DB DOWN, API DOWN, WEB DOWN | until `up` |
| **Metric** | `cpu`, `mem`, `load`, `disk`, `traffic`, `err` = `high` / `ok` | CPU HIGH … ; `ok` shows CPU OK for 60 s | until `ok` |

Failures stay lit until something replaces them, so a failed build at 2am is still there in the morning. In **Auto** colour, failures and DOWN are red, HIGH amber, and PASS, LIVE, ONLINE and OK green.

## Setup

1. Install the firmware and power the unit from the base.
2. On your phone, join the open WiFi network **`DevOpsClock-XXXX`**. A setup page opens (or go to `http://192.168.4.1`).
3. Pick your home WiFi. Leave **Channel key** empty and the panel makes up a random one.

Then open **`http://devopsclock-xxxx.local`** (the same `XXXX` as the setup network). It has test buttons for every state and ready-to-copy commands with your channel key filled in.

## Sending it updates

**From your own network**, one HTTP request:

```bash
curl -X POST "http://devopsclock-xxxx.local/api/event?name=ci_fail"
curl -X POST "http://devopsclock-xxxx.local/api/status?server=online"
curl -X POST "http://devopsclock-xxxx.local/api/status?db=down"
curl -X POST "http://devopsclock-xxxx.local/api/metric?metric=disk&state=high"
curl -X POST "http://devopsclock-xxxx.local/api/reset"      # clear everything
```

**From anywhere** (GitHub Actions, a server in the cloud), publish to the MQTT broker **broker.emqx.io** with your channel key:

```bash
mosquitto_pub -h broker.emqx.io -p 8883 --capath /etc/ssl/certs \
  -t 'lonelybinary/devops/<channel key>/event/ci_pass' -m 1

mosquitto_pub -h broker.emqx.io -p 8883 --capath /etc/ssl/certs \
  -t 'lonelybinary/devops/<channel key>/metric' -m '{"metric":"cpu","state":"high"}'
```

Topics: `.../event/<event>`, `.../status/<online|down|db_down|db_up|api_down|…>` and `.../metric` with a JSON payload.

### GitHub Actions example

Add this as the last step of a job. It reports pass or fail for every run:

```yaml
      - name: Tell the DevOps Word Clock
        if: always()
        run: |
          sudo apt-get install -y mosquitto-clients
          mosquitto_pub -h broker.emqx.io -p 8883 --capath /etc/ssl/certs \
            -t "lonelybinary/devops/${{ secrets.WORDCLOCK_KEY }}/event/ci_${{ job.status == 'success' && 'pass' || 'fail' }}" -m 1
```

Store the channel key as the repository secret `WORDCLOCK_KEY`.

## Buttons

- **A**: change colour (Auto → White → Red → Blue → Green → Rainbow, remembered)
- **B**: acknowledge. Clears CI FAIL, ROLLBACK, BACKUP FAIL and any "OK" lines
- **Both for 3 seconds**: forget WiFi and settings, back to setup

If the panel can't reach the MQTT broker, the first corner dot glows dim red. HTTP updates on your own network still work.

## Security and limits

- The channel key is the only secret for MQTT. Anyone who knows it can light your panel, so keep it in your CI secrets and change it in Settings if it leaks.
- The HTTP API has no password; anyone on your WiFi can use it.
- The panel forgets the current state when it loses power. Your tooling sends the next update and it's back.

## Building from source

Open `DevOpsClock.ino` in the Arduino IDE. You need the `libraries/LBPanel` folder from this repo copied into your Arduino `libraries` folder, plus FastLED, WiFiManager, PubSubClient and ArduinoJson from the Library Manager. Board **ESP32S3 Dev Module**, **Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)**. See the [root README](../README.md#building-from-source).
