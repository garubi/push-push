#include <USB-MIDI.h>
#include <Keyboard.h>
#include <EEPROM.h>

// Questo script di Arduino permette di inviare keystrokes a un computer emulando una tastiera USB
// Si possono usare fino a 20 pulsanti collegati ai pin digitali 2, 3, 4 e 5 ... 
// Ogni pulsante può inviare una sequenza di tasti configurabile, compresi i tasti di controllo non stampabili
// La configurazione dei tasti da inviare è salvata nella eprom e può essere modificata tramite System Exclusive via MIDI USB

// Definire il numero di pulsanti da usare (da 1 a 20)
#define NUM_BUTTONS 2

// Definire i pin digitali a cui sono collegati i pulsanti
const int buttonPins[NUM_BUTTONS] = {2, 3};

// Definire la dimensione massima della sequenza di tasti da inviare per ogni pulsante (in byte)
#define KEYS_SEQUENCE_SIZE 5

// firmware version
const byte VERSION_MAJOR = 0;
const byte VERSION_MINOR = 0;
const byte VERSION_PATCH = 2;

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

// Definire l'indirizzo iniziale della eprom dove salvare la configurazione dei tasti
#define EEPROM_START_ADDRESS 0

// Creare un array per memorizzare lo stato dei pulsanti
int buttonState[NUM_BUTTONS];

// Creare un array per memorizzare la sequenza di tasti da inviare per ogni pulsante
byte keySequence[NUM_BUTTONS][KEYS_SEQUENCE_SIZE];


/* *************************************************************************
 *  MIDI initialization: we use MIDI only to send e receive the configuration from the editor via SysEx
 */

USBMIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // pinMode(LED_BUILTIN, OUTPUT);
  
  // Inizializzare la comunicazione seriale a 9600 baud
  Serial.begin(115200);
  // while(!Serial);
  
  // Inizializzare la tastiera USB
  Keyboard.begin();

  // Inizializzare il MIDI per comunicare con l'editor via SysEx
  MIDI.begin(1);
  MIDI.setHandleSystemExclusive(receiveSysExConfig);

  // Impostare i pin dei pulsanti come input con resistenza di pull-up interna
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  
  // Leggere la configurazione dei tasti dalla eprom e metterla nell'array keySequence
  for (int i = 0; i < NUM_BUTTONS*KEYS_SEQUENCE_SIZE; i++) {
     keySequence[i / KEYS_SEQUENCE_SIZE][i % KEYS_SEQUENCE_SIZE] = EEPROM.read(EEPROM_START_ADDRESS + i); // I byte successivi contengono le sequenze di tasti
  }

  // Stampare un messaggio di benvenuto sul monitor seriale
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
  Serial.println();

  Serial.println(F("Benvenuto nello script di Arduino per inviare keystrokes."));
}

void loop() {
   MIDI.read();

  // Leggere lo stato dei pulsanti e inviare la sequenza di tasti corrispondente se premuti
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(buttonPins[i]);
    if (buttonState[i] == LOW) { // Se il pulsante è premuto (stato basso)
    Serial.println(F("premuto"));
      sendKeySequence(i); // Invia la sequenza di tasti associata al pulsante
      delay(500); // Aggiungi un ritardo per evitare ripetizioni accidentali
    }
  }

  // Controllare se è arrivato un comando dal monitor seriale
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

// Callback per gestire la ricezione dei messaggi SysEx per la configurazione
// https://forum.arduino.cc/t/midi-sysex-sending-2-bytes-from-signed-int/930637

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
                  byte nyb1 = ( keySequence[btn][i] >>7 ) & 0x7F ;
                  byte nyb2 = keySequence[btn][i]  & 0x7F;

                  rp[11+count] = nyb1; // Leggere il byte corrispondente al tasto da inviare
                  rp[11+count+1] = nyb2; // Leggere il byte corrispondente al tasto da inviare

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

                  byte res = (sysex_message[11 + index] << 7) | sysex_message[11 + index+1];  // combine the 7 bit chunks to 14 bits in the int
                  res = res << 2 >> 2 ;  // sign-extend as 16 bit
                  keySequence[btn][key] = res ;

                  EEPROM.write(EEPROM_START_ADDRESS + eepr_index, res); // Salvare il byte nella eprom
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

// Funzione che invia la sequenza di tasti associata a un pulsante
void sendKeySequence(int buttonIndex) {
    Serial.println(F("sendKeySequence")); 
    Serial.println(buttonIndex); 
  for (int i = 0; i < KEYS_SEQUENCE_SIZE; i++) {
    byte key = keySequence[buttonIndex][i]; // Leggere il byte corrispondente al tasto da inviare
    Keyboard.press(key); // Premere il tasto
    delay(100); // Aggiungere un ritardo per assicurare la corretta trasmissione
  }
  Keyboard.releaseAll(); // Rilasciare il tasto
}

void printInfo() {
    // Stampare un messaggio di benvenuto sul monitor seriale
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

  Serial.println(F("Benvenuto nello script di Arduino per inviare keystrokes."));

    Serial.println(F("Push Push è collegato"));
    Serial.println(F("Questa è la configurazione:"));
    for (int btn = 0; btn < NUM_BUTTONS; btn++) {
        Serial.print(F("Tasto "));
        Serial.println(btn);
        for (int i = 0; i < KEYS_SEQUENCE_SIZE; i++) {
        byte key = keySequence[btn][i]; // Leggere il byte corrispondente al tasto da inviare
        Serial.print(key);
        Serial.print(F(" "));
        }
        Serial.println();
    }
}
