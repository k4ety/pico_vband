EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Pi Pico Vband Dongle"
Date "2024-02-22"
Rev "1.0"
Comp "Graham Whaley"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_RaspberryPi_and_Boards:Pico U1
U 1 1 65D72976
P 5700 3850
F 0 "U1" H 5700 5065 50  0000 C CNN
F 1 "Pico" H 5700 4974 50  0000 C CNN
F 2 "RPi_Pico:RPi_Pico_SMD_TH" V 5700 3850 50  0001 C CNN
F 3 "" H 5700 3850 50  0001 C CNN
	1    5700 3850
	1    0    0    -1  
$EndComp
$Comp
L Connector:AudioJack3 J1
U 1 1 65D77292
P 2100 3200
F 0 "J1" H 2082 3525 50  0000 C CNN
F 1 "Morse_Paddle" H 2082 3434 50  0000 C CNN
F 2 "" H 2100 3200 50  0001 C CNN
F 3 "~" H 2100 3200 50  0001 C CNN
	1    2100 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 3300 5000 3300
Wire Wire Line
	2300 3200 5000 3200
Wire Wire Line
	2300 3100 5000 3100
$Comp
L Switch:SW_Push SW1
U 1 1 65D7A934
P 2000 3950
F 0 "SW1" V 1954 4098 50  0000 L CNN
F 1 "Mode_Button" V 2045 4098 50  0000 L CNN
F 2 "" H 2000 4150 50  0001 C CNN
F 3 "~" H 2000 4150 50  0001 C CNN
	1    2000 3950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2000 3750 4550 3750
Wire Wire Line
	4550 3750 4550 3400
Wire Wire Line
	4550 3400 5000 3400
Wire Wire Line
	2000 4150 4700 4150
Wire Wire Line
	4700 4150 4700 3600
Wire Wire Line
	4700 3600 5000 3600
$EndSCHEMATC
