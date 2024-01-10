# ![Push Push Logo](push-push-logo.png) Push Push

Push Push is a very easy to build device (based on Arduino) that allows you to send keystrokes to a computer by emulating a USB keyboard.

Up to 20 buttons can be used connected to digital pins 2, 3, 4 and 5... 
Each button can send a configurable keystroke, including non-printable controls 

While you can find a lot of similar DIY project on the web, **Push Push is pretty unique because it can be configured with it's own [editor](https://garubi.github.io/push-push-editor/)** to send keystrokes of your choice, without the need to change the firmware code e without the need of Arduino compiler.
You simply connect it to the [editor](https://garubi.github.io/push-push-editor/) and configure your Push Push.

# How to build it

## The hardware 
The hardware part is really basic: they are just pushbuttons connected to an Arduino.

The script expects to be run on a **Arduino Leonardo** or **Arduino Pro Micro**, but it should run without problems on any boards that supports `HID` protocol, like *Leonardo*, *Micro*, *Due*, *Zero*, *UNO R4 Minima*, *UNO R4 WiFi*, *Giga R1*, *Nano ESP32*, *MKR Family* (i.e. all the boards supported by the [Keyboard library](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/))

All you have to do is connect one end of each push button to the GND pin and the other to one of the available digital pins.

Then update the script (see [section below](#the-software)) with the number of buttons connected and the digital pins they are connected to

You can find some example schematics in the [`/schematics`](/schematics/) folder

![A wiring example for 2 buttons](/schematics/wiring-example.png)

## The software
If you are using just 2 buttons, you can use the `push-push.ino` file included in the [/arduino/push-push](/arduino/push-push/) folder without any modification. Just install the required libraries (see below), compile and upload it to your Arduino.

If you have a different number of buttons, you have to change just two rows in the script

At the beginning the script, find the following section and change the buttons count  and Pin numbers with yours
```C++
// *************** START EDITING HERE *********************** //
// Depending on the number of buttons you'll use
// edit the following defines

  // Write the number of buttons used (For Arduino Leonardo I recommend no more than 20)
  #define NUM_BUTTONS 2

  // Write the Digital Pins where the buttons are connected to
  const int buttonPins[NUM_BUTTONS] = {2, 3};

// *************** STOP EDITING HERE  ********************** //
```
When finished, save the script, compile and upload

>[!WARNING]
>Be sure to have the following libraries installed in your IDE before compiling.
>
>- [FortySevenEffects MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library)
>- [lathoub Arduino-USBMIDI](https://github.com/lathoub/Arduino-USBMIDI)
>- [Arduino Keyboard library](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/)
>- [Arduino EEPROM Library](https://docs.arduino.cc/learn/built-in-libraries/eeprom)


## The configuration
The strength of Push Push is the ease of configuration: you don't need to modify the Arduino code or use complex software to configure the key sequences to send: just use the **Push Push Editor**.

[**>>>> GO TO PUSH PUSH EDITOR <<<<**](https://garubi.github.io/push-push-editor/)

The communication between Push Push and the editor is based on the MIDI protocol, exchanging System Exclusive Messages. 
If you are curious you can find the implemetation details in the [SYSEX-MIDI-IMPLEMENTATION.md](SYSEX-MIDI-IMPLEMENTATION) file

### How to install and use the Push Push Editor
Since the editor is contained in a web page, you don't have to install anything: just open the editor page and follow the simple instructions.
The editor works on desktop computers, tablets and mobile phones, on any operating system: all you need is an internet browser and a USB port to connect Push Push to.

Here is a preview:

![An animated preview of the Push Push editor](animated-editor-preview.gif)

[**>>>> GO TO PUSH PUSH EDITOR <<<<**](https://garubi.github.io/push-push-editor/)

### How to know the current Push Push configuration
You have two way to read the current configuration: 
1. The easy way: Connect to the [Push Push Editor](https://garubi.github.io/push-push-editor/) and you will see all the configurations
1. The "developer" way: Open the Arduino Serial Monitor and type `?` then push `ENTER`. Push Push will display informations on the Serial Monitor 
