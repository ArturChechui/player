# ESP32 Internet Radio – Requirements v0.1

## 0. Summary

An ESP32-S3 based internet radio device that:
- Plays online radio streams (AAC + MP3) via ESP-ADF.
- Shows a station list on a 0.96" OLED (I2C) and indicates the currently active station.
- Uses buttons for selection + play/stop and a rotary encoder for volume.
- Displays temperature & humidity in a corner of the screen (AHT20).
- Maintains a local station list and can download updates in the background (reboot to apply).

## 1. Goals & Non-Goals

### Goals
- Reliable playback of AAC/MP3 radio URLs.
- Predictable UI behavior: selection is separate from activation.
- Testable core logic (state machine, station selection, persistence rules).
- Non-blocking background station-list update.

### Non-Goals (v0.1)
- Metadata (ICY title/artist), album art, track history.
- Voice/clap control.
- Battery power / power management.
- Web UI / BLE UI for station management (beyond background download).
- Audius / Spotify integration (possible future versions).

## 2. Hardware Assumptions (V1)

- MCU: ESP32-S3 SuperMini
- Display: SSD1306 128x64 OLED via I2C
- Temp/Hum: AHT20 (I2C) (BMP280 pressure ignored in v0.1)
- Audio: 2× MAX98357 (I2S DAC/amp) + 2× 8Ω 3W speakers
- Inputs:
  - Buttons: Up, Down, Play, Stop
  - Rotary encoder EC11 for Volume (push optional, not used in v0.1)

## 3. Functional Requirements (FR)

### FR-01 Station list & selection
- The device shall display a list of stations (name + active marker).
- `Up` shall move selection up (wrap optional; default: wrap ON).
- `Down` shall move selection down.
- Selection shall NOT start playback.

Acceptance:
- Pressing Up/Down changes only selection within 50 ms perceived UI update time.

### FR-02 Play
- `Play` shall start playback of the currently selected station.
- If the selected station is already active, Play is a no-op (or restarts — define as no-op in v0.1).
- UI shall show status: `Stopped`, `Buffering`, `Playing`, `Error`.

Acceptance:
- Given a reachable URL and Wi-Fi connected, audio starts within a reasonable time (informational target: P50 < 3s).

### FR-03 Stop
- `Stop` shall stop playback.
- The UI shall update status to `Stopped`.

Acceptance:
- Stop transitions to `Stopped` state and audio output ceases.

### FR-04 Active station indicator
- The list shall mark the active station (e.g., `•` dot on the left).
- Active station and selection may differ (selection is navigated independently).

Acceptance:
- When a station is playing, it is marked active even if user scrolls selection elsewhere.

### FR-05 Volume control (rotary encoder)
- Rotating encoder changes volume.
- Volume range: 0..100 (step: 2 by default).
- Volume changes apply immediately.

Acceptance:
- Rotating encoder updates volume and shows it on UI within 100 ms.

### FR-06 Temp/Humidity display
- Read AHT20 periodically (e.g., every 5–10 seconds).
- Display current temperature & humidity in a small UI corner.

Acceptance:
- Temp/Hum values update periodically and do not disrupt playback.

### FR-07 Persistence (NVS)
Persist in NVS:
- `autoplay` (bool): whether device should auto-start playback on boot
- `last_station_id` (string): id of last active station
- `volume` (0..100)

Rules:
- When Play starts successfully: set `autoplay=true`, store `last_station_id`.
- When user presses Stop: set `autoplay=false`.
- Volume updates persist with debounce (e.g., save 1–2s after last change).

Acceptance:
- After reboot, device restores last volume.
- If `autoplay=true`, device attempts to start last station.

### FR-08 Wi-Fi configuration (v0.1)
- Wi-Fi credentials are provided via hardcoded configuration (v0.1 bring-up).
- Implementation may cache credentials in NVS, but user provisioning is out of scope for v0.1.

### FR-09 Boot behavior
On boot:
1) Load station list from LittleFS into RAM (active list for this session).
2) Read settings from NVS (autoplay, last_station_id, volume).
3) If `autoplay=true`, attempt to play `last_station_id`.
   - If missing: show message “Last station missing” and remain stopped.

Acceptance:
- Device boots into usable UI without requiring network.
- Autoplay does not block UI rendering (start playback asynchronously).

### FR-10 Station list storage (LittleFS)
- Store `stations.json` in LittleFS.
- The station list loaded into RAM remains fixed for the session (no hot-swap in v0.1).

Constraints:
- V0.1 cap: max 10 stations.

JSON schema v0.1:
```json
[
  {"id":"radio1_aac_h","name":"Radio 1 (AAC High)","url":"https://...aac"},
  {"id":"some_mp3","name":"Some MP3","url":"https://...mp3"}
]
````

### FR-11 Background station list update (GitHub)

* After Wi-Fi is connected and playback is stable, start a background updater task.
* Definition: playback is stable if player state is `Playing` continuously for 10 seconds.
* Updater checks a remote manifest first (cheap download):

  * `stations.manifest.json` contains `{ "version": <int>, "sha256": "<hex>", "size": <int> }`
* If manifest indicates change, download `stations.json` to a temp file, validate it, then atomically replace local file.
* Do not apply the new list immediately. UI shows:

  * “Station list updated. Reboot to apply.”

Retry/backoff:

* On failure, retry with exponential backoff: 60s → 2m → 4m → 8m → 16m → 30m (cap).
* After success, reset backoff.

Validation:

* JSON parse ok
* array size 1..10
* each entry has non-empty `id`, `name`, `url`
* url must start with `http://` or `https://`

Invalid remote behavior:

* Keep old local file.
* Show short UI message: “Remote list invalid” (rate-limited; not spamming).
* If 30 minutes pass with only invalid results: show “Remote list invalid (paused)” and stop updater until reboot.

No-update behavior:

* If no updates found for 30 minutes: show “List up-to-date” once (optional) and continue checking at low frequency OR pause (v0.1 decision: pause to save resources).

(Implementation note: optional optimization later via ETag/If-Modified-Since.)

### FR-12 Logging (local)

* Log major events: boot, wifi connect/disconnect, play/stop, station change, updater status.
* Log should be minimal and useful for debugging.

## 4. Non-Functional Requirements (NFR)

* NFR-01 Reliability: no crashes during 4-hour playback soak (v0.1 target).
* NFR-02 Responsiveness: button actions should feel instant (UI update < ~100 ms).
* NFR-03 Resource: station list stored in RAM; v0.1 max 10 items.
* NFR-04 Observability: log levels configurable at compile-time (INFO default).

## 5. Testing Strategy (v0.1)

Host unit tests (fast):

* Station selection rules (up/down, wrap)
* Player state machine transitions (Stopped/Buffering/Playing/Error)
* Persistence rules (autoplay + last_station_id + volume debounce)
* Updater decision logic (manifest comparison, backoff schedule, “pause after 30m” rules)
* JSON validation

Target/integration tests (later):

* AHT20 read sanity
* LittleFS read/write
* Basic playback smoke on ESP32-S3 (manual initially)

## 6. V2/V3 candidates (stub)

* V2: metadata, better UI layouts, battery, clap/voice control.
* V3: Audius station/track browsing (if desired), Wi-Fi provisioning via BLE or captive portal.
