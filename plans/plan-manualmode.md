# Manual test mode (encoder-driven, operator-controlled)

## Context

`STATE_MANUAL_TEST` in `src/main.cpp` is currently a stub (falls straight back to `STATE_SELECT_MODE`) — display screens `SCREEN_5`..`SCREEN_9` exist but no protocol logic was ever written for it. The goal is to implement the real manual measurement procedure: the operator dials frequency and amplitude themselves using two rotary encoders, decides when to present each tone, and records the patient's response — with none of semi-auto mode's randomized inter-stimulus wait or per-step "confirm" screens. This mirrors how a clinician operates a real manual audiometer (two knobs + a tone-presentation switch), as opposed to `STATE_SEMI_TEST`'s automated Hughson-Westlake bracketing.

Decisions confirmed with the user:
- **Tone trigger**: hold the existing `OK_BUTTON_PIN` down to play a tone continuously; release to stop. No new "play" button, no push-button encoders required.
- **Encoder pins**: placeholders only for now (real wiring not finalized) — defined clearly in `pins.h` as TODO, same spirit as the existing commented-out `ENC_*` block, but this time as real `#define`s the code compiles against.
- **Frequency control**: the Hz encoder steps through the *same 8-entry `freqTable`* used by semi-auto (not arbitrary continuous Hz).
- **Amplitude control**: the dB encoder changes intensity in **5 dB** steps, clamped 0–70 dB HL (same clamp `volume_setDb` already enforces). Resets to a **fixed default (20 dB)** every time the frequency changes.
- **Data logging**: every tone-hold/release cycle is one logged data point (ear, freq, dB, patient responded true/false) — appended to a growing log, sent over Serial at the end like `protocol_sendDataToPC()` does for semi-auto. Not the fixed 3-trial shape `toSendResponses` uses, since the operator may replay a frequency at different dB any number of times.
- **Advancing frequency**: a short press of `DOWN_BUTTON_PIN` moves to the next entry in `freqTable` (operator can hold/replay the current frequency at as many dB levels as they want first).
- **Ear flow**: starting ear is chosen exactly as today, in `STATE_SELECT_MODE`, before entering manual mode. Once all 8 frequencies for that ear have been stepped through (via `DOWN`), show an on-screen notice that the ear is switching, wait for a plain `OK` tap to acknowledge, then run the second ear the same way. After both ears are done, go to `STATE_FINISHED` and send the manual log to the PC.

## Design

### New module: `src/manual.h` / `src/manual.cpp`
Mirrors the role `protocol.cpp` plays for semi-auto. Owns the manual-test loop, entered once (blocking, busy-wait style — same idiom `STATE_SEMI_TEST`/`protocol_hughsonWestlake` already use) from `main.cpp`'s `STATE_MANUAL_TEST` case:

```cpp
void manual_runTest(volatile AudiometerData *audioData, IntensityChangedCallback onIntensityChanged);
```

Per ear (starting ear first, then the other):
- `audioData->freqIndex = 0`; reset intensity to 20 dB via `volume_setDb`.
- Loop until 8 frequencies stepped through:
  - **Dialing phase** (OK not held): poll the two encoders each iteration.
    - Hz encoder delta moves `freqIndex` by ±1, clamped `[0, FREQ_TABLE_SIZE-1]`; on change, recompute `phaseInc` (same formula `protocol_setFrequencyIndex` uses), reset intensity to 20 dB, call `onIntensityChanged`/redraw.
    - dB encoder delta moves intensity by ±5 via `volume_setDb` (clamped internally), call `onIntensityChanged`.
    - Poll `DOWN_BUTTON_PIN` flag (`buttonDownPressed`, existing ISR flag/clear pattern) → clear it, advance `freqIndex` by 1 and break out to the next frequency (or, if this was the 8th, fall through to the ear-complete branch below).
    - Poll `OK_BUTTON_PIN` **by level**, not the existing edge-triggered `buttonOKPressed` flag (holding needs live state, not a one-shot press event) → when pressed, enter the play phase.
  - **Play phase** (OK held): clear `buttonPatientPressed`, call the new `audio_playWhileHeld()` (below) which blocks until `digitalRead(OK_BUTTON_PIN)` goes back HIGH, polling `buttonPatientPressed` each buffer iteration. On release: write silence (already part of the helper), then append one entry to the manual results log (`ear = audioData->currentEar`, `freqHz = freqTable[freqIndex]`, `db = audioData->intensity`, `response = buttonPatientPressed`), clear the flag, redraw.
- On ear-complete: show the ear-switch notice screen, busy-wait for a plain `OK` tap (`buttonOKPressed` edge flag, same clear-before/clear-after idiom as `STATE_SEMI_TEST`), flip `audioData->currentEar`, continue to the second ear — or return if this was already the second ear.

### Audio: extract + add a hold-driven tone function (`src/audio.h`/`.cpp`)
`audio_generateTone()`'s per-sample fill-and-I2S-write loop (sine LUT lookup, phase advance, L/R steering by `currentEar`) is the reusable core. Extract it into a small static helper (e.g. `static void audio_writeToneBuffer(volatile AudiometerData *audioData)`), keep `audio_generateTone()` calling it under its existing random-duration `while`, and add:

```cpp
void audio_playWhileHeld(volatile AudiometerData *audioData); // NEW
```
which calls the same helper under `while (digitalRead(OK_BUTTON_PIN) == LOW)` instead of a duration timer, then writes silence once at the end exactly like `audio_generateTone()` already does.

### New encoder driver: `src/encoder.h` / `src/encoder.cpp`
No existing encoder code anywhere in the repo — new from scratch, following the codebase's existing ISR + volatile-flag/counter idiom (`buttons.cpp`):
- `void encoder_init();` — configure the 4 pins `INPUT_PULLUP`, attach `CHANGE` interrupts on each A pin (or both A/B) doing a standard 2-bit quadrature state-transition decode, incrementing/decrementing a `volatile int16_t` step counter per encoder.
- `int encoder_readHzDelta();` / `int encoder_readDbDelta();` — return the accumulated signed step count since last call and reset it to 0 (read-and-clear, same pattern as button flags).

### Pins (`src/pins.h`)
Replace the commented-out placeholder block with real (but explicitly TODO-flagged) defines on pins not otherwise claimed in the file (e.g. `A2`/`A3` for the Hz encoder, `A4`/`A5` for the dB encoder), with a comment that these must be confirmed against actual wiring before flashing to real hardware.

### Data model
- `src/types.h`: add a small fixed-capacity log, declared `extern` alongside `toSendResponses`:
  ```cpp
  struct ManualResult {
    bool ear;        // internal convention: 0=Lewe, 1=Prawe (same as AudiometerData.currentEar)
    uint16_t freqHz;
    uint8_t db;
    bool response;
  };
  #define MAX_MANUAL_RESULTS 200
  extern ManualResult manualResults[MAX_MANUAL_RESULTS];
  extern int manualResultCount;
  ```
  Defined in `main.cpp` next to `toSendResponses`. 200 entries comfortably covers 2 ears × 8 freqs × ~12 replays each; `manual_runTest` guards against overrunning the array (stops logging further points past capacity rather than overflowing).
- `AudiometerData.freqIndex` and `.intensity` (already existing fields) are reused directly — no new per-test-state struct needed. The unused `MenuNavigation` struct is left alone (its generic shape doesn't fit this concrete design; not worth forcing).

### Sending results (`src/protocol.h`/`.cpp`)
Add `protocol_sendManualDataToPC(ManualResult results[], int count)` alongside the existing `protocol_sendDataToPC()`, printing each logged point over Serial in an analogous format (ear/freq/dB/response per line).

### `src/main.cpp`
- `STATE_MANUAL_TEST` case: call `manual_runTest(&audioData, onIntensityChanged)` once, then transition to `STATE_FINISHED` (matching how `STATE_SEMI_TEST` falls through today).
- `STATE_FINISHED`: branch on `audioData.isSemiAutomatic` to call `protocol_sendDataToPC(toSendResponses)` or `protocol_sendManualDataToPC(manualResults, manualResultCount)`.
- `setup()`: add `encoder_init()` alongside the existing `buttons_init()`/`audio_init()`/`volume_init()` calls.

### Display (`src/display.h`/`.cpp`)
- Repurpose `SCREEN_5_MANUAL_FREQ` as the single live "dialing" screen for manual mode, extended to show both current frequency (existing `display_setFrequency`) **and** current intensity (existing `display_setIntensity`) at once, plus a short instruction line ("Trzymaj OK aby odtworzyc / DOWN = kolejna czestotliwosc"). `SCREEN_6_MANUAL_DB` becomes unused (freq+dB now shown together) — leave its rendering code in place but stop invoking it, since it's cheap to keep and not worth a separate cleanup pass right now.
- Repurpose `SCREEN_9_MANUAL_STEP_DONE` as the ear-switch notice screen (it already has roughly the right shape — a message + "ZATWIERDZ").
- `SCREEN_7_MANUAL_REACTION`/`SCREEN_8_MANUAL_NO_REACTION` are not needed by this design (response is captured automatically from `buttonPatientPressed` during the hold, not asked about afterward) — leave unused, same reasoning as `SCREEN_6`.
- `STATE_FINISHED`'s existing `SCREEN_4_SUMMARY` is reused for both modes' end-of-test screen.

## Files touched
- New: `src/manual.h`, `src/manual.cpp`, `src/encoder.h`, `src/encoder.cpp`
- Edited: `src/main.cpp`, `src/pins.h`, `src/types.h`, `src/audio.h`, `src/audio.cpp`, `src/protocol.h`, `src/protocol.cpp`, `src/display.cpp` (SCREEN_5/9 render content)

## Verification
- `pio run` builds cleanly for `arduino_nano_esp32`.
- Without real encoder hardware, sanity-check `encoder_init()`/ISR logic compiles and the manual-mode loop behaves correctly for the OK-hold/DOWN-advance/ear-switch control flow by temporarily driving the encoder pins from a wired test button or logic analyzer / bench signal generator, since this can't be verified through source review alone.
- End-to-end on hardware: select manual mode + starting ear in `STATE_SELECT_MODE`, confirm dialing a frequency snaps through the 8 `freqTable` values, dialing amplitude moves in 5 dB steps and resets to 20 dB on frequency change, holding OK plays a continuous tone in the correct ear that stops immediately on release, `buttonPatientPressed` during a hold gets logged as a response, `DOWN` advances frequency, the ear-switch notice appears after the 8th frequency and correctly flips ears, and `STATE_FINISHED` prints the full manual log over Serial (`pio device monitor`) with correct ear/freq/dB/response values.
