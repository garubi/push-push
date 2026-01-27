# Push Push - AI Coding Instructions

## Project Overview

Push Push is a dual-component DIY MIDI hardware project:
- **Arduino firmware** (`push-push/`): Converts button presses to USB keyboard keystrokes
- **Web-based editor** (`push-push-editor/`): Configures button mappings via MIDI SysEx messages

The two components communicate bidirectionally using MIDI System Exclusive (SysEx) messages to read/write configuration stored in Arduino's EEPROM.

## Architecture

### Communication Protocol (Critical)

The MIDI SysEx protocol is the integration backbone - see [SYSEX-MIDI-IMPLEMENTATION.md](../SYSEX-MIDI-IMPLEMENTATION.md):

- **Manufacturer IDs**: `0x37, 0x72` (UBIStage)
- **Product ID**: `0x10` (Push Push)
- **Message format**: `F0 [IDs] [REQ/REP] [GET/SET] [payload] F7`
- **7-bit encoding**: Each keystroke byte is split into two 7-bit values (`nyb1`, `nyb2`) because SysEx only supports 7-bit data
  - Encode: `nyb1 = (byte >> 7) & 0x7F; nyb2 = byte & 0x7F`
  - Decode: `byte = (nyb1 << 7) | nyb2; byte = byte << 2 >> 2` (sign-extend)

**When modifying**: Both Arduino (`.ino`) and JavaScript (`app.js`) must implement identical encoding/decoding logic.

### Arduino Firmware ([push-push.ino](../arduino/push-push/push-push.ino))

**User-configurable section** (lines 14-23):
```cpp
#define NUM_BUTTONS 2
const int buttonPins[NUM_BUTTONS] = {2, 3};
#define DEBOUNCE_TIME_MS 150
```
This is the ONLY section users should edit for hardware customization.

**Core components**:
- `keySequence[NUM_BUTTONS][KEYS_SEQUENCE_SIZE]`: 2D array storing button→keystroke mappings
- `receiveSysExConfig()`: MIDI callback handling GET/SET commands, performs EEPROM read/write
- `sendKeySequence()`: Presses all 5 keys in sequence with 100ms delays, then releases all
- Serial monitor `?` command: Displays ASCII art + current configuration

**Libraries required** (must be installed via Arduino IDE):
- FortySevenEffects MIDI Library
- lathoub Arduino-USBMIDI  
- Arduino Keyboard library
- Arduino EEPROM Library

### Web Editor ([push-push-editor/docs/](../push-push-editor/docs/))

**Stack**: Vanilla JS + Alpine.js (reactive state) + WebMIDI.js

**Key functions in [app.js](../push-push-editor/docs/js/app.js)**:
- `connect()`: Sends `X_REQ, X_GET` SysEx to request current config
- `storeInPushPush()`: Sends `X_REQ, X_SET` with new config
- `parseSysEx()`: Routes incoming SysEx to `assignParams()` or handles SET response
- `fileExport()`/`fileImport()`: Save/load config as `.pushpush` files (space-separated SysEx bytes)

**State management**: `Alpine.store('pp')` holds device info, config parameters, and UI flags (`pp_loading`, `pp_stored`, etc.)

## Development Workflows

### Testing Arduino Changes

1. Edit [push-push.ino](../arduino/push-push/push-push.ino) - typically only the user-configurable section
2. Compile and upload via Arduino IDE (ensure Leonardo/Pro Micro board selected)
3. Open Serial Monitor (115200 baud), type `?` + Enter to verify config loaded from EEPROM
4. Press physical buttons to test keystroke output
5. To test SysEx: Connect via [web editor](https://garubi.github.io/push-push-editor/), click "Connect", verify displayed config

### Testing Web Editor Changes

1. Edit files in `push-push-editor/docs/` (no build step - static site)
2. Serve locally: `python -m http.server 8000` from `docs/` directory
3. Open browser to `localhost:8000`
4. Connect real Arduino device or test with mock SysEx data via browser console:
   ```javascript
   fromPushPush.emit('sysex', {data: [0xF0, 0x37, 0x72, 0x10, 0x01, 0x00, ...]});
   ```

### Common Debugging

**"Editor can't connect"**: Check browser WebMIDI permissions, ensure Arduino USB-MIDI is recognized by OS

**"Keystrokes not working"**: Verify Arduino shows "premuto" in Serial Monitor on button press - if yes, issue is in `sendKeySequence()`; if no, check wiring/debounce

**"Config not persisting"**: EEPROM writes happen only via SET command - check SysEx reply is `X_OK` in Serial Monitor

## Code Conventions

- **Version syncing**: `VERSION_MAJOR/MINOR/PATCH` in `.ino` must match validation logic in `receiveSysExConfig()` SET handler (line 195)
- **Magic numbers**: SysEx message sizes are hardcoded (e.g., `11+NUM_BUTTONS*KEYS_SEQUENCE_SIZE*2`) - update if protocol changes
- **Italian comments**: Some Serial.println strings use Italian ("Tasto", "premuto") - preserve for consistency
- **No build tooling**: Web editor is intentionally vanilla - no npm/webpack. Alpine/WebMIDI are CDN-linked
- **Service Worker**: `push-push-editor/docs/service-worker.js` enables PWA offline support - version in `app.js` must increment for update notifications

## KiCad Schematics

Located in [schematics/](../schematics/) - open with [KiCad](https://www.kicad.org/) (free). Two examples provided:
- `push-push-2-buttons/`: Default 2-button config
- `push-push-5-buttons/`: 5-button variant

Schematics are trivial (buttons between GND and digital pins with internal pull-up) but serve as reference for pin assignments.
