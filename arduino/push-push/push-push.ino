// Changed Keystrokes to control the Reaper Record and Play functions:
// CTRL +r:  Record
// SPACE : play



#include <Keyboard.h>

// https://www.instructables.com/id/USB-Pedal-HID-Keyboard-With-Arduino-Leonardo/
// https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
// https://www.arduino.cc/reference/en/language/functions/usb/keyboard/keyboardmodifiers/
// KEY_UP_ARROW
// KEY_DOWN_ARROW
// KEY_LEFT_ARROW
// KEY_RIGHT_ARROW
// KEY_ESC
// KEY_PAGE_DOWN

#include <Bounce2.h>

//Pulsanti
const byte PEDALNEXT_PIN = 2;
const byte PEDALPREV_PIN = 3;

//Led
const byte PEDALNEXT_LED = 9;
const byte PEDALPREV_LED = 8;

//Tasti da emulare
//const byte PEDALNEXT_KEY = r;
const byte PEDALPREV_KEY = KEY_ESC;


const byte PED_NEXT = PEDALNEXT_PIN;
const byte PED_PREV = PEDALPREV_PIN;

// Instantiate a Bounce object
Bounce ped_next = Bounce(); 
// Instantiate another Bounce object
Bounce ped_prev = Bounce(); 

static void SendKey( byte pedal ){
  switch( pedal ){
    case PED_NEXT:
     Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.print("r");
    break; 
    case PED_PREV:
      //Keyboard.press(PEDALPREV_KEY);
       Keyboard.print(" ");

    break; 
  }
  delay(100);
  Keyboard.releaseAll();
}

void setup(void)
{
    Keyboard.begin();

    // Set pin to input
    pinMode(PEDALNEXT_PIN, INPUT_PULLUP);
    // Enable pullup resistor
    //digitalWrite(PEDALNEXT_PIN, HIGH);
    // After setting up the button, setup the Bounce instance :
    ped_next.attach(PEDALNEXT_PIN);
    // ped_next.interval(5); // interval in ms
  
    // Set pin to input
    pinMode(PEDALPREV_PIN, INPUT_PULLUP);
    // Enable pullup resistor
    //digitalWrite(PEDALPREV_PIN, HIGH);
        ped_prev.attach(PEDALPREV_PIN);
    // ped_prev.interval(5); // interval in ms

    pinMode(PEDALNEXT_LED, OUTPUT);
    pinMode(PEDALPREV_LED, OUTPUT);
}

void loop(void)
{
    static uint8_t pedalNEXTStateLast = 0;
    static uint8_t pedalPREVStateLast = 0;
    uint8_t pedalState;
    ped_next.update();
    ped_prev.update();
  
    pedalState = ped_next.read();
    if (pedalState != pedalNEXTStateLast) {
        pedalNEXTStateLast = pedalState;

        if (pedalState == LOW ) {
            SendKey( PED_NEXT );
            digitalWrite(PEDALNEXT_LED, HIGH );
        }
        else{
            digitalWrite(PEDALNEXT_LED, LOW );
        }
    }

    pedalState = ped_prev.read();
    if (pedalState != pedalPREVStateLast) {
        pedalPREVStateLast = pedalState;

        if (pedalState == LOW ) {
            SendKey( PED_PREV );
            digitalWrite(PEDALPREV_LED, HIGH );
        }
        else{
            digitalWrite(PEDALPREV_LED, LOW );
        }
    }

    delay(50);
}
