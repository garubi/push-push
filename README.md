# Push Push
configurable arduino pedalboard

# Midi Syex Reference:
-  http://midi.teragonaudio.com/tech/midispec.htm (qui trovi anche il sysex request device identity )
- https://midimonitor.lim.di.unimi.it/monitor.html
- https://studiocode.dev/

#usbMIDI libraries:

- https://github.com/lathoub/Arduino-USBMIDI (questa sembra la più simile a quelle che ho usato con teensy)
- https://github.com/arduino-libraries/MIDIUSB (questa è quella "originale" di arduino)
- https://github.com/BlokasLabs/usbmidi (questa mi sembra che legga solo raw midi... boh... )

# sysex CONFIG implementation


            IDENTIFIER        MESSAGE_TYPE   ACTION   CONTENT_TYPE       
F0 X_MANID1 X_MANID2 X_PRODID [REQ, REPL] [ GET, SET ]   [...]           F7

const uint8_t X_MANID1 = 0x37; // Manufacturer ID 1  (UBIStage)
const uint8_t X_MANID2 = 0x72; // Manufacturer ID 2  (UBIStage)
const uint8_t X_PRODID = 0x10; // Product ID (Push Push)

const uint8_t X_MODELID = 0x10; // Model ID (Specific Push Push implementations: num of buttons etc)

const byte VERSION_MAJOR = 1;
const byte VERION_MINOR = 0;
const byte VERSION_PATCH = 0;

const byte REQ = 0x00; // Request
const byte REP = 0x01; // Replay

const byte GET = 0x00; 
const byte SET = 0x01;
const byte X_ERROR = 0x7F;

const byte FAILED = 0x7F;
const byte OK = 0x01;


//Where CONTENT_TYPE is: 

// Ask the current configuration to Push Push:
REQ GET 
// Push Push answer:
REPL GET VERSION_MAJOR VERION_MINOR VERSION_PATCH X_MODELID BUTTON_QTY KEYS_SEQUENCE_SIZE BTN_n_MODIFIER_CODE_1 BTN_n_MODIFIER_CODE_2 BTN_n_MODIFIER_CODE_3 BTN_n_MODIFIER_CODE_4 BTN_n_KEY_CODE [...]

// Store the editor's fields as new configuration in Push Push:
REQ SET VERSION_MAJOR VERION_MINOR VERSION_PATCH X_MODELID BUTTON_QTY KEYS_SEQUENCE_SIZE BTN_n_MODIFIER_CODE_1 BTN_n_MODIFIER_CODE_2 BTN_n_MODIFIER_CODE_3 BTN_n_MODIFIER_CODE_4 BTN_n_KEY_CODE [...]
// Push Push answer
REPL SET  [OK, FAILED ]


per migliorare:
- https://www.partsnotincluded.com/diy-stream-deck-mini-macro-keyboard/

Nome USB device:

 - https://forum.arduino.cc/t/multiple-leonardos-as-hid-joystick-how-to-change-the-names/402646

 - https://gist.github.com/Hyratel/80017369fedd1bbc9eef4c8e7a896225

 - https://github.com/NicoHood/HID/issues/125
 
