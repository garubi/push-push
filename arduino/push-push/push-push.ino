#include <USB-MIDI.h>
#include <Keyboard.h>
#include <EEPROM.h>

// Questo script di Arduino permette di inviare keystrokes a un computer emulando una tastiera USB
// Si possono usare fino a 20 pulsanti collegati ai pin digitali 2, 3, 4 e 5 ... 
// Ogni pulsante può inviare una sequenza di tasti configurabile, compresi i tasti di controllo non stampabili
// La configurazione dei tasti da inviare è salvata nella eprom e può essere modificata tramite System Exclusive via MIDI USB

// firmware version
const byte VERSION_MAJOR = 0;
const byte VERION_MINOR = 0;
const byte VERSION_PATCH = 1;

const uint8_t X_MANID1 = 0x37; // Manufacturer ID 1 (UBIStage)
const uint8_t X_MANID2 = 0x72; // Manufacturer ID 2 (UBIStage)
const uint8_t X_PRODID = 0x10; // Product ID (Push Push)

const uint8_t X_MODELID = 0x10; // Model ID (Specific Push Push implementations: num of buttons etc)

const byte X_REQ = 0x00; // Request
const byte X_REP = 0x01; // Replay

const byte X_GET = 0x00; 
const byte X_SET = 0x01;

const byte X_FAILED = 0x00;
const byte X_OK = 0x01;

// Definire il numero di pulsanti da usare (da 1 a 20)
#define NUM_BUTTONS 2

// Definire i pin digitali a cui sono collegati i pulsanti
const int buttonPins[NUM_BUTTONS] = {2, 3};

// Definire la dimensione massima della sequenza di tasti da inviare per ogni pulsante (in byte)
#define MAX_SEQUENCE_SIZE 5

// Definire l'indirizzo iniziale della eprom dove salvare la configurazione dei tasti
#define EEPROM_START_ADDRESS 0

// Creare un array per memorizzare lo stato dei pulsanti
int buttonState[NUM_BUTTONS];

// Creare un array per memorizzare la sequenza di tasti da inviare per ogni pulsante
// byte keySequence[NUM_BUTTONS][MAX_SEQUENCE_SIZE];
byte  keySequence[NUM_BUTTONS][MAX_SEQUENCE_SIZE] = {
        {'a', 'b', 'c', 'z', 'g'}, // Il primo pulsante manda CTRL+C
        {'d', 'e', 'f', 'u', 'v'}, // Il secondo pulsante manda CTRL+V
  };

// Creare una variabile per memorizzare il numero di byte letti dalla eprom
int numBytesRead;

/* *************************************************************************
 *  MIDI initialization: we use MIDI only to send e receive the configuration from the editor via SysEx
 */

USBMIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // pinMode(LED_BUILTIN, OUTPUT);
  
  // Inizializzare la comunicazione seriale a 9600 baud
  Serial.begin(115200);
  while(!Serial);
  
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
  //numBytesRead = EEPROM.read(EEPROM_START_ADDRESS); // Il primo byte indica il numero di byte da leggere
  // for (int i = 0; i < NUM_BUTTONS*MAX_SEQUENCE_SIZE; i++) {
  //    keySequence[i / MAX_SEQUENCE_SIZE][i % MAX_SEQUENCE_SIZE] = EEPROM.read(EEPROM_START_ADDRESS + i + 1); // I byte successivi contengono le sequenze di tasti
  // }

  // Stampare un messaggio di benvenuto sul monitor seriale
  Serial.println("Benvenuto nello script di Arduino per inviare keystrokes.");
  Serial.println("Per modificare la configurazione dei tasti, scrivi 'config' e segui le istruzioni.");
}

void loop() {
   MIDI.read();

  // Leggere lo stato dei pulsanti e inviare la sequenza di tasti corrispondente se premuti
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(buttonPins[i]);
    if (buttonState[i] == LOW) { // Se il pulsante è premuto (stato basso)
    Serial.println("premuto");
      sendKeySequence(i); // Invia la sequenza di tasti associata al pulsante
      delay(500); // Aggiungi un ritardo per evitare ripetizioni accidentali
    }
  }

  // Controllare se è arrivato un comando dal monitor seriale
  if (Serial.available() > 0) {
    String command = Serial.readString(); // Leggere il comando come una stringa
    command.trim(); // Rimuovere eventuali spazi o caratteri di fine linea
    if (command == "config") { // Se il comando è 'config'
      configKeySequence(); // Avvia la procedura di configurazione dei tasti
    }
    else if(command == "info"){
        printInfo();
    }
    else { // Se il comando non è riconosciuto
      Serial.println("Comando non valido. ");
      Serial.println(command);
      Serial.println("Scrivi 'config' per modificare la configurazione dei tasti.");
    }
  }
}

// Callback per gestire la ricezione dei messaggi SysEx per la configurazione
void receiveSysExConfig( byte* sysex_message, unsigned sysExSize ) {
  Serial.println("RX sysex");

  if ( sysex_message[1] == X_MANID1 && sysex_message[2] == X_MANID2 && sysex_message[3] == X_PRODID && sysex_message[4] == X_REQ){
      // The SysEx is for internal use of Push Push
      Serial.println("SYSEX for us ;-) ");
      Serial.println(sysex_message[5]) ;
        switch( sysex_message[5]){
          case X_GET: // reply sending the configuration to the editor
          {
            Serial.println("GET");
            uint8_t rp[6+NUM_BUTTONS*MAX_SEQUENCE_SIZE] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_GET, NUM_BUTTONS };

            byte count = 0;
            for (int btn = 0; btn < NUM_BUTTONS; btn++) {
                for (int i = 0; i < MAX_SEQUENCE_SIZE; i++) {
                  rp[6+count] = keySequence[btn][i]; // Leggere il byte corrispondente al tasto da inviare
                  count ++;
                }
            }
             MIDI.sendSysEx(sizeof(rp), rp, false);
          }
          break;
          case X_SET: // receive the configuration, stores it, and replay to the editor
          {
            Serial.println("SET");
            for (int i = 0; i < NUM_BUTTONS*MAX_SEQUENCE_SIZE; i++) {
              byte key = sysex_message[6 + i];
              keySequence[i / MAX_SEQUENCE_SIZE][i % MAX_SEQUENCE_SIZE] = key ;
              // EEPROM.write(EEPROM_START_ADDRESS + i + 1, key); // Salvare il byte nella eprom
            }              
            
              uint8_t rp[6] = { X_MANID1, X_MANID2, X_PRODID, X_REP, X_SET, X_OK };
              MIDI.sendSysEx(sizeof(rp), rp, false);
          }
          break;
          default:
            Serial.print("SysEx Action value not allowed. Received: ");
            Serial.println( sysex_message[5] );
          break;
        }
      }
      else{
          Serial.println("This SysEx message is not for us");
        }
}

// Funzione che invia la sequenza di tasti associata a un pulsante
void sendKeySequence(int buttonIndex) {
    Serial.println("sendKeySequence"); 
    Serial.println(buttonIndex); 
  for (int i = 0; i < MAX_SEQUENCE_SIZE; i++) {
    byte key = keySequence[buttonIndex][i]; // Leggere il byte corrispondente al tasto da inviare
    // Serial.println(key);
    if (key == 0) { // Se il byte è zero, termina la sequenza
      break;
    }
    else { 
      Keyboard.press(key); // Premere il tasto
      delay(100); // Aggiungere un ritardo per assicurare la corretta trasmissione

    }
  }
  Keyboard.releaseAll(); // Rilasciare il tasto
}

void printInfo() {
    Serial.println("Push push è collegato");
    Serial.println("Questa è la configurazione:");
    for (int btn = 0; btn < NUM_BUTTONS; btn++) {
        Serial.print("Tasto ");
        Serial.println(btn);
        for (int i = 0; i < MAX_SEQUENCE_SIZE; i++) {
        byte key = keySequence[btn][i]; // Leggere il byte corrispondente al tasto da inviare
        Serial.print(key);
        Serial.print(" ");
        }
        Serial.println();
    }
}

// Funzione che avvia la procedura di configurazione dei tasti via Seriale 
void configKeySequence() {
  Serial.println("Inizio della configurazione dei tasti.");
  Serial.println("Per ogni pulsante, inserisci la sequenza di tasti da inviare separati da spazi.");
  Serial.println("Puoi usare i seguenti codici speciali per i tasti di controllo non stampabili:");
  Serial.println("128 = CTRL sinistro, 129 = SHIFT sinistro, 130 = ALT sinistro, 131 = GUI sinistro (Win/Command)");
  Serial.println("132 = CTRL destro, 133 = SHIFT destro, 134 = ALT destro, 135 = GUI destro (Win/Command)");
  Serial.println("136 = INVIO, 137 = ESC, 138 = BACKSPACE, 139 = TAB, 140 = SPAZIO, 141 = CANCELLA");
  Serial.println("142 = FRECCIA SINISTRA, 143 = FRECCIA DESTRA, 144 = FRECCIA SU, 145 = FRECCIA GIU");
  Serial.println("Per terminare la sequenza, inserisci uno zero.");
  Serial.println("Per terminare la configurazione, inserisci una linea vuota.");

  // Creare una variabile per memorizzare il numero di byte scritti nella eprom
  int numBytesWritten = 0;

  // Scrivere il primo byte nella eprom con il valore zero (sarà aggiornato alla fine con il numero effettivo di byte scritti)
  EEPROM.write(EEPROM_START_ADDRESS, 0);

  // Creare un ciclo per ogni pulsante
  for (int i = 0; i < NUM_BUTTONS; i++) {
    Serial.print("Pulsante ");
    Serial.print(i + 1);
    Serial.print(": ");
    // Creare un ciclo per leggere i tasti da inviare per il pulsante corrente
    for (int btn = 0; btn < MAX_SEQUENCE_SIZE; btn++) {
      while (Serial.available() == 0) {} // Attendere che arrivi un dato dal monitor seriale
      String input = Serial.readStringUntil(' '); // Leggere il dato come una stringa fino allo spazio successivo
      input.trim(); // Rimuovere eventuali spazi o caratteri di fine linea
      Serial.println(input);
      if (input == "") { // Se la stringa è vuota, termina la configurazione
        Serial.println("Fine della configurazione dei tasti.");
        // Aggiornare il primo byte della eprom con il numero effettivo di byte scritti
        EEPROM.write(EEPROM_START_ADDRESS, numBytesWritten);
        return;
      }
      else { // Se la stringa non è vuota, convertirla in un byte e salvarla nell'array keySequence e nella eprom

      // ma qui non la converte in un byte... qui va solo se abbiamo scritto degli interi
        byte key = input.toInt(); // Convertire la stringa in un byte
        Serial.println(key);
        keySequence[i][btn] = key; // Salvare il byte nell'array keySequence
        EEPROM.write(EEPROM_START_ADDRESS + numBytesWritten + 1, key); // Salvare il byte nella eprom
        numBytesWritten++; // Incrementare il numero di byte scritti
        if (key == 0) { // Se il byte è zero, termina la sequenza per il pulsante corrente
          break;
        }
      }
    }
    Serial.println(); // Andare a capo
  }
}
