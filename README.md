# Push Push
configurable arduino pedalboard

# sysex CONFIG implementation


            IDENTIFIER        MESSAGE_TYPE   ACTION   CONTENT_TYPE       
F0 X_MANID1 X_MANID2 X_PRODID [REQ, REPL] [ GET, SET ]   [...]           F7

const uint8_t X_MANID1 = 0x37; // Manufacturer ID 1
const uint8_t X_MANID2 = 0x72; // Manufacturer ID 2
const uint8_t X_PRODID = 0x10; // Product ID

const byte VERSION_MAJOR = 1;
const byte VERION_MINOR = 0;
const byte VERSION_PATCH = 0;

const byte REQ = 0x00; // Request
const byte REP = 0x01; // Replay

const byte GET = 0x00; 
const byte SET = 0x01;

const byte FAILED = 0x00;
const byte OK = 0x01;


//Where CONTENT_TYPE is: 

// Ask the current configuration to Push Push:
REQ GET 
// Push Push answer:
REPL GET VERSION_MAJOR VERION_MINOR VERSION_PATCH BUTTON_QTY BTN_n_MODIFIER_CODE_1 BTN_n_MODIFIER_CODE_2 BTN_n_MODIFIER_CODE_3 BTN_n_MODIFIER_CODE_4 BTN_n_KEY_CODE [...]

// Store the editor's fields as new configuration in Push Push:
REQ SET VERSION_MAJOR VERION_MINOR VERSION_PATCH BUTTON_QTY BTN_n_MODIFIER_CODE_1 BTN_n_MODIFIER_CODE_2 BTN_n_MODIFIER_CODE_3 BTN_n_MODIFIER_CODE_4 BTN_n_KEY_CODE [...]
// Push Push answer
REPL SET  [OK, FAILED ]


per migliorare:
- https://www.partsnotincluded.com/diy-stream-deck-mini-macro-keyboard/


