// Questo script di Arduino permette di inviare keystrokes a un computer emulando una tastiera USB
// Si possono usare fino a 20 pulsanti collegati ai pin digitali 2, 3, 4 e 5 ... 
// Ogni pulsante può inviare una sequenza di tasti configurabile, compresi i tasti di controllo non stampabili
// La configurazione dei tasti da inviare è salvata nella eprom e può essere modificata tramite il monitor seriale

#include <Keyboard.h>
#include <EEPROM.h>

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
byte keySequence[NUM_BUTTONS][MAX_SEQUENCE_SIZE];

// Creare una variabile per memorizzare il numero di byte letti dalla eprom
int numBytesRead;

void setup() {
  // Inizializzare la comunicazione seriale a 9600 baud
  Serial.begin(9600);

  // Inizializzare la tastiera USB
  Keyboard.begin();

  // Impostare i pin dei pulsanti come input con resistenza di pull-up interna
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Leggere la configurazione dei tasti dalla eprom e salvarla nell'array keySequence
  numBytesRead = EEPROM.read(EEPROM_START_ADDRESS); // Il primo byte indica il numero di byte da leggere
  for (int i = 0; i < numBytesRead; i++) {
    keySequence[i / MAX_SEQUENCE_SIZE][i % MAX_SEQUENCE_SIZE] = EEPROM.read(EEPROM_START_ADDRESS + i + 1); // I byte successivi contengono le sequenze di tasti
  }

  // Stampare un messaggio di benvenuto sul monitor seriale
  Serial.println("Benvenuto nello script di Arduino per inviare keystrokes.");
  Serial.println("Per modificare la configurazione dei tasti, scrivi 'config' e segui le istruzioni.");
}

void loop() {
  // Leggere lo stato dei pulsanti e inviare la sequenza di tasti corrispondente se premuti
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(buttonPins[i]);
    if (buttonState[i] == LOW) { // Se il pulsante è premuto (stato basso)
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
    else { // Se il comando non è riconosciuto
      Serial.println("Comando non valido. Scrivi 'config' per modificare la configurazione dei tasti.");
    }
  }
}

// Funzione che invia la sequenza di tasti associata a un pulsante
void sendKeySequence(int buttonIndex) {
  for (int i = 0; i < MAX_SEQUENCE_SIZE; i++) {
    byte key = keySequence[buttonIndex][i]; // Leggere il byte corrispondente al tasto da inviare
    if (key == 0) { // Se il byte è zero, termina la sequenza
      break;
    }
    // else if (key < KEY_LEFT_CTRL) { // Se il byte è minore di KEY_LEFT_CTRL, invia il tasto come un carattere
    //   Keyboard.write(key);
    // }
    else { 
      Keyboard.press(key); // Premere il tasto
      delay(100); // Aggiungere un ritardo per assicurare la corretta trasmissione

    }
  }
  Keyboard.releaseAll(); // Rilasciare il tasto
}

// Funzione che avvia la procedura di configurazione dei tasti
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
    for (int j = 0; j < MAX_SEQUENCE_SIZE; j++) {
      while (Serial.available() == 0) {} // Attendere che arrivi un dato dal monitor seriale
      String input = Serial.readStringUntil(' '); // Leggere il dato come una stringa fino allo spazio successivo
      input.trim(); // Rimuovere eventuali spazi o caratteri di fine linea
      if (input == "") { // Se la stringa è vuota, termina la configurazione
        Serial.println("Fine della configurazione dei tasti.");
        // Aggiornare il primo byte della eprom con il numero effettivo di byte scritti
        EEPROM.write(EEPROM_START_ADDRESS, numBytesWritten);
        return;
      }
      else { // Se la stringa non è vuota, convertirla in un byte e salvarla nell'array keySequence e nella eprom
        byte key = input.toInt(); // Convertire la stringa in un byte
        keySequence[i][j] = key; // Salvare il byte nell'array keySequence
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