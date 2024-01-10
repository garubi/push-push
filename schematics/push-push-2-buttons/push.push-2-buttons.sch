EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Push Push"
Date "2024-01-10"
Rev "1.0"
Comp "UBI Stage"
Comment1 "2 buttons"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Switch:SW_Push SW1
U 1 1 5DD7D5B1
P 3950 3200
F 0 "SW1" H 3950 3485 50  0000 C CNN
F 1 "Pedal Next" H 3950 3394 50  0000 C CNN
F 2 "" H 3950 3400 50  0001 C CNN
F 3 "~" H 3950 3400 50  0001 C CNN
	1    3950 3200
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 5DD7E343
P 3950 3600
F 0 "SW2" H 3950 3885 50  0000 C CNN
F 1 "Pedal Prev" H 3950 3794 50  0000 C CNN
F 2 "" H 3950 3800 50  0001 C CNN
F 3 "~" H 3950 3800 50  0001 C CNN
	1    3950 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3600 4150 3600
Wire Wire Line
	5400 3200 4150 3200
$Comp
L MCU_Module:Arduino_Leonardo A1
U 1 1 5DD7BDC3
P 5900 3600
F 0 "A1" H 5900 4781 50  0000 C CNN
F 1 "Arduino_Leonardo" H 5900 4690 50  0000 C CNN
F 2 "Module:Arduino_UNO_R3" H 6050 2550 50  0001 L CNN
F 3 "https://www.arduino.cc/en/Main/ArduinoBoardLeonardo" H 5700 4650 50  0001 C CNN
	1    5900 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 4700 3450 3600
Wire Wire Line
	3450 3200 3750 3200
Wire Wire Line
	3750 3600 3450 3600
Connection ~ 3450 3600
Wire Wire Line
	3450 3600 3450 3200
Wire Wire Line
	4200 3300 5400 3300
Wire Wire Line
	4200 3600 4200 3300
Wire Wire Line
	3450 4700 5800 4700
$EndSCHEMATC
