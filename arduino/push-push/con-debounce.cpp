`
// Questo script di Arduino permette di inviare keystrokes al computer tramite dei pulsanti collegati ai pin digitali.
// Si può indicare il numero di pulsanti desiderati e configurare quali tasti mandare per ogni pulsante.
// Tra i tasti da mandare si possono usare anche i tasti di controllo non stampabili, come CTRL, ALT, SHIFT, ecc.

#include <Keyboard.h> // Libreria per usare la funzione Keyboard

#define NUM_BUTTONS 4 // Numero di pulsanti da usare
#define DEBOUNCE_TIME 50 // Tempo di debounce in millisecondi

// Array che contiene i pin digitali a cui sono collegati i pulsanti
int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5};

// Array che contiene lo stato corrente dei pulsanti
int buttonState[NUM_BUTTONS];

// Array che contiene lo stato precedente dei pulsanti
int lastButtonState[NUM_BUTTONS];

// Array che contiene il tempo dell'ultimo cambiamento di stato dei pulsanti
unsigned long lastDebounceTime[NUM_BUTTONS];

// Array bidimensionale che contiene i tasti da mandare per ogni pulsante
// Si possono usare le costanti definite nella libreria Keyboard, come KEY_LEFT_CTRL, KEY_LEFT_ALT, ecc.
// Per i tasti stampabili si possono usare le lettere tra apici singoli, come 'a', 'b', ecc.
// Si possono mandare più tasti contemporaneamente separandoli da una virgola
char* buttonKeys[NUM_BUTTONS][3] = {
  {KEY_LEFT_CTRL, 'c', '\0'}, // Il primo pulsante manda CTRL+C
  {KEY_LEFT_CTRL, 'v', '\0'}, // Il secondo pulsante manda CTRL+V
  {KEY_LEFT_ALT, KEY_F4, '\0'}, // Il terzo pulsante manda ALT+F4
  {' ', '\0', '\0'} // Il quarto pulsante manda la barra spaziatrice
};

void setup() {
  // Inizializza la comunicazione seriale a 9600 baud
  Serial.begin(9600);

  // Inizializza la funzione Keyboard
  Keyboard.begin();

  // Imposta i pin dei pulsanti come input con resistenza di pull-up interna
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    // Inizializza lo stato dei pulsanti a HIGH (non premuto)
    buttonState[i] = HIGH;
    lastButtonState[i] = HIGH;
  }
}

void loop() {
  // Legge lo stato di ogni pulsante e controlla se è cambiato rispetto al precedente
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(buttonPins[i]); // Legge il valore del pin

    // Se il valore è diverso dallo stato precedente, aggiorna il tempo di debounce
    if (reading != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }

    // Se il tempo di debounce è trascorso, controlla se lo stato è cambiato effettivamente
    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_TIME) {
      // Se lo stato è cambiato, aggiorna lo stato corrente e manda i tasti corrispondenti
      if (reading != buttonState[i]) {
        buttonState[i] = reading;

        // Se il pulsante è premuto (stato basso), manda i tasti configurati
        if (buttonState[i] == LOW) {
          for (int j = 0; j < 3; j++) {
            // Se il tasto è diverso da '\0', lo manda
            if (buttonKeys[i][j] != '\0') {
              Keyboard.press(buttonKeys[i][j]);
            }
          }
          // Rilascia tutti i tasti mandati
          Keyboard.releaseAll();
        }
      }
    }

    // Aggiorna lo stato precedente con quello corrente
    lastButtonState[i] = reading;
  }
}