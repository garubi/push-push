#include <USB-MIDI.h>
#include <Keyboard.h>
#include <EEPROM.h>

// This Arduino script allows you to send keystrokes to a computer by emulating a USB keyboard. 
// Up to 20 buttons can be used connected to digital pins 2, 3, 4 and 5... 
// Each button can send a configurable keystroke, including non-printable controls 
// The configuration of the keys to be sent is saved in the eprom and can be modified via System Exclusive via MIDI USB

// *************** START EDITING HERE *********************** //
// Depending on the number of buttons you'll use
// edit the following defines

  // Write the number of buttons used (For Arduino Leonardo I recommend no more than 20)
  #define NUM_BUTTONS 2

  // Write the Digital Pins where the buttons are connected
  const int buttonPins[NUM_BUTTONS] = {2, 3};

// *************** STOP EDITING HERE  ********************** //

// Define the maximum size of the keystroke to send for each button (in bytes)
#define KEYS_SEQUENCE_SIZE 5

// firmware version
const byte VERSION_MAJOR = 1;
const byte VERSION_MINOR = 0;
const byte VERSION_PATCH = 1;

// ********************************************************* //
// SysEx implementation for remote configuration             //

const uint8_t X_MANID1 = 0x37; // Manufacturer ID 1 (UBIStage)
const uint8_t X_MANID2 = 0x72; // Manufacturer ID 2 (UBIStage)
const uint8_t X_PRODID = 0x10; // Product ID (Push Push)

const uint8_t X_MODELID = 0x10; // Model ID (Specific Push Push implementations: num of buttons etc)

const byte X_REQ = 0x00; // Request
const byte X_REP = 0x01; // Replay

const byte X_GET = 0x00; 
const byte X_SET = 0x01;
const byte X_ERROR = 0x7F;

const byte X_FAILED = 0x7F;
const byte X_OK = 0x01;
// ********************************************************* //

// Define the initial address of the EPROM where to save the key configuration
#define EEPROM_START_ADDRESS 0

// Create an array to store the state of the buttons
int buttonState[NUM_BUTTONS];

// Create an array to store the keystroke to send for each button
byte keySequence[NUM_BUTTONS][KEYS_SEQUENCE_SIZE];


// MIDI initialization: we use MIDI only to send e receive the configuration from the editor via SysEx
USBMIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize the serial communication at 9600 baud
  Serial.begin(115200);
  // while(!Serial); // commenting out becouse it blocks the MIDI over USB communication (Why???????)
  
  // Initialize USB keyboard
  Keyboard.begin();

  // Initialize MIDI to communicate with the editor via SysEx
  MIDI.begin(1);
  MIDI.setHandleSystemExclusive(receiveSysExConfig);

  // Set the button pins as inputs with internal pull-up resistor
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  
  // Read the key configuration from the eprom and put it in the keySequence array
  for (int i = 0; i < NUM_BUTTONS*KEYS_SEQUENCE_SIZE; i++) {
     keySequence[i / KEYS_SEQUENCE_SIZE][i % KEYS_SEQUENCE_SIZE] = EEPROM.read(EEPROM_START_ADDRESS + i); // I byte successivi contengono le sequenze di tasti
  }

  printInfo();
}

void loop() {
   MIDI.read();

// **************** SEND KEYSTROKE WHEN BUTTON PRESSED ************************ //

  // Read the state of the buttons and send the corresponding keystroke if pressed
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(buttonPins[i]);
    if (buttonState[i] == LOW) { // If the button is pressed (low state)
    Serial.println(F("premuto"));
      sendKeySequence(i); // Sends the keystroke associated with the button
      delay(500); // Add a delay to avoid accidental repeats
    }
  }

// **************** PRINT DEVICE'S INFO ON SERIAL MONITOR ************************ //
// ****************** WHEN YOU TYPE ? + ENTER ON CONSOLE  ************************ //

  // Check if a command has arrived from the serial monitor
  if (Serial.available() > 0) {
    String command = Serial.readString(); // Leggere il comando come una stringa
    command.trim(); // Rimuovere eventuali spazi o caratteri di fine linea
    if(command == "?"){
        printInfo();
    }
    else { // Se il comando non è riconosciuto
      Serial.println("Comando non valido. ");
      Serial.println(command);
      Serial.println("Scrivi '?' per avere info sulla configurazione.");
    }
  }
}


// Callback to manage the receipt of SysEx messages for configuration

void receiveSysExConfig( byte* sysex_message, unsigned sysExSize ) {

  if ( sysex_message[1] == X_MANID1 && sysex_message[2] == X_MANID2 && sysex_message[3] == X_PRODID && sysex_message[4] == X_REQ){
      // The SysEx is for internal use of Push Push
      Serial.println(F("SYSEX for us ;-) "));

        switch( sysex_message[5]){
          case X_GET: // reply sending the configuration to the editor
          {
            Serial.println(F("GET"));
            uint8_t rp[11+NUM_BUTTONS*KEYS_SEQUENCE_SIZE*2] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_GET, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, X_MODELID, NUM_BUTTONS, KEYS_SEQUENCE_SIZE };

            byte count = 0;
            for (int btn = 0; btn < NUM_BUTTONS; btn++) {
                for (int i = 0; i < KEYS_SEQUENCE_SIZE; i++) {

                  // Sysex has only 7 bit so for each byte representing a keystroke we have to split it in two
                  // see: https://forum.arduino.cc/t/midi-sysex-sending-2-bytes-from-signed-int/930637
                  byte nyb1 = ( keySequence[btn][i] >>7 ) & 0x7F ;
                  byte nyb2 = keySequence[btn][i]  & 0x7F;

                  rp[11+count] = nyb1; 
                  rp[11+count+1] = nyb2; 

                  count = count +2;
                }
            }

             MIDI.sendSysEx(sizeof(rp), rp, false);
          }
          break;
          case X_SET: // receive the configuration, stores it, and replay to the editor
          {
            Serial.println(F("SET"));
            if (  sysex_message[6] == VERSION_MAJOR && 
                  sysex_message[7] == VERSION_MINOR && 
                  sysex_message[8] == VERSION_PATCH && 
                  sysex_message[9] == X_MODELID && 
                  sysex_message[10] == NUM_BUTTONS &&
                  sysex_message[11] == KEYS_SEQUENCE_SIZE &&
                  sysExSize == 1 + 11 + NUM_BUTTONS*KEYS_SEQUENCE_SIZE*2 + 1 )
            {
              byte index = 1;
              byte eepr_index = 0;
              for( byte btn = 0; btn < NUM_BUTTONS; btn++){
                for(byte key = 0; key < KEYS_SEQUENCE_SIZE; key++ ){

                  // Sysex has only 7 bit so we need to combine the 7 bit chunks for represent a keystroke
                  // see: https://forum.arduino.cc/t/midi-sysex-sending-2-bytes-from-signed-int/930637
                  byte res = (sysex_message[11 + index] << 7) | sysex_message[11 + index+1];  // combine the 7 bit chunks to 14 bits in the int
                  res = res << 2 >> 2 ;  // sign-extend as 16 bit
                  keySequence[btn][key] = res ;

                  EEPROM.write(EEPROM_START_ADDRESS + eepr_index, res); // Save the byte in eprom
                  index = index + 2;
                  eepr_index ++;
                }
              }
            
              Serial.println(F("Stored"));
              uint8_t rp[6] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_SET, X_OK };
              MIDI.sendSysEx(sizeof(rp), rp, false);
            }
            else{
              Serial.println(F("Error"));
                uint8_t rp[6] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_SET, X_FAILED };
                MIDI.sendSysEx(sizeof(rp), rp, false);              
            }
          }
          break;
          default:
            Serial.print(F("SysEx Action value not allowed. Received: "));
            Serial.println( sysex_message[5] );
            uint8_t rp[6] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_ERROR, X_FAILED };
            MIDI.sendSysEx(sizeof(rp), rp, false);  
          break;
        }
      }
      else{
          Serial.println(F("This SysEx message is not for us"));
        }
}

// Function that sends the keystroke associated with a button
void sendKeySequence(int buttonIndex) {
    Serial.println(F("sendKeySequence")); 
    Serial.println(buttonIndex); 
  for (int i = 0; i < KEYS_SEQUENCE_SIZE; i++) {
    byte key = keySequence[buttonIndex][i]; // Read the byte corresponding to the key to send
    Keyboard.press(key); // Press the button
    delay(100); // Add a delay to ensure correct transmission
  }
  Keyboard.releaseAll(); // Release all pressed keys
}

// When you type ? and press ENTER on the serial monitor, we display some info:
void printInfo() {
    // Print a welcome message on the serial monitor
  Serial.println(F(" .----------------.  .----------------.  .----------------.  .----------------. "));
  Serial.println(F("| .--------------. || .--------------. || .--------------. || .--------------. |"));
  Serial.println(F("| |   ______     | || | _____  _____ | || |    _______   | || |  ____  ____  | |"));
  Serial.println(F("| |  |_   __ \\   | || ||_   _||_   _|| || |   /  ___  |  | || | |_   ||   _| | |"));
  Serial.println(F("| |    | |__) |  | || |  | |    | |  | || |  |  (__ \\_|  | || |   | |__| |   | |"));
  Serial.println(F("| |    |  ___/   | || |  | '    ' |  | || |   '.___`-.   | || |   |  __  |   | |"));
  Serial.println(F("| |   _| |_      | || |   \\ `--' /   | || |  |`\\____) |  | || |  _| |  | |_  | |"));
  Serial.println(F("| |  |_____|     | || |    `.__.'    | || |  |_______.'  | || | |____||____| | |"));
  Serial.println(F("| |              | || |              | || |              | || |              | |"));
  Serial.println(F("| '--------------' || '--------------' || '--------------' || '--------------' |"));
  Serial.println(F(" '----------------'  '----------------'  '----------------'  '----------------' "));
  Serial.println(F(" .----------------.  .----------------.  .----------------.  .----------------. "));
  Serial.println(F("| .--------------. || .--------------. || .--------------. || .--------------. |"));
  Serial.println(F("| |   ______     | || | _____  _____ | || |    _______   | || |  ____  ____  | |"));
  Serial.println(F("| |  |_   __ \\   | || ||_   _||_   _|| || |   /  ___  |  | || | |_   ||   _| | |"));
  Serial.println(F("| |    | |__) |  | || |  | |    | |  | || |  |  (__ \\_|  | || |   | |__| |   | |"));
  Serial.println(F("| |    |  ___/   | || |  | '    ' |  | || |   '.___`-.   | || |   |  __  |   | |"));
  Serial.println(F("| |   _| |_      | || |   \\ `--' /   | || |  |`\\____) |  | || |  _| |  | |_  | |"));
  Serial.println(F("| |  |_____|     | || |    `.__.'    | || |  |_______.'  | || | |____||____| | |"));
  Serial.println(F("| |              | || |              | || |              | || |              | |"));
  Serial.println(F("| '--------------' || '--------------' || '--------------' || '--------------' |"));
  Serial.println(F(" '----------------'  '----------------'  '----------------'  '----------------' "));
  Serial.print( F("Version: "));
  Serial.print(VERSION_MAJOR);
  Serial.print(F("."));
  Serial.print(VERSION_MINOR);
  Serial.print(F("."));
  Serial.print(VERSION_PATCH);
  Serial.print(F(" Model: "));
  Serial.print(X_MODELID);

  Serial.println();

  Serial.println(F("Welcome to Push Push, an Arduino script for sending keystrokes."));
  Serial.println();
  Serial.println(F("Go to https://garubi.github.io/push-push-editor/ to configure"));

    Serial.println(F("Push Push connected"));
    Serial.println(F("Current configuration:"));
    for (int btn = 0; btn < NUM_BUTTONS; btn++) {
        Serial.print(F("Tasto "));
        Serial.println(btn);
        for (int i = 0; i < KEYS_SEQUENCE_SIZE; i++) {
        byte key = keySequence[btn][i];
        Serial.print(key);
        Serial.print(F(" "));
        }
        Serial.println();
    }
}
