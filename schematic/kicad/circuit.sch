EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 9055 9056
encoding utf-8
Sheet 1 1
Title "Kitchen Clock"
Date "2021-12-25"
Rev "A"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 "Author: Mark Sungurov <mark.sungurov@gmail.com>"
$EndDescr
Text Notes 1650 4600 0    90   ~ 18
Matrix keyboard 2x2
Wire Wire Line
	2300 5650 2300 6250
Connection ~ 2300 5650
Wire Wire Line
	2300 5000 2300 5650
Wire Wire Line
	3000 5650 3000 6250
Connection ~ 3000 5650
Wire Wire Line
	3000 5000 3000 5650
Wire Wire Line
	2600 6250 2600 6350
Wire Wire Line
	1900 6350 2600 6350
Connection ~ 1900 6350
Wire Wire Line
	1900 6250 1900 6350
Wire Wire Line
	1450 6350 1900 6350
Wire Wire Line
	2600 5650 2600 5750
Wire Wire Line
	1900 5750 2600 5750
Connection ~ 1900 5750
Wire Wire Line
	1900 5650 1900 5750
Wire Wire Line
	1450 5750 1900 5750
Text GLabel 3000 5000 1    50   Input ~ 0
PA4
Text GLabel 2300 5000 1    50   Input ~ 0
PA3
Text GLabel 1450 6350 0    50   Input ~ 0
PA2
Text GLabel 1450 5750 0    50   Input ~ 0
PA1
$Comp
L Switch:SW_Push BTN4
U 1 1 61DB6BF7
P 2800 6250
F 0 "BTN4" H 2800 6535 50  0000 C CNN
F 1 "4" H 2800 6444 50  0000 C CNN
F 2 "" H 2800 6450 50  0001 C CNN
F 3 "~" H 2800 6450 50  0001 C CNN
	1    2800 6250
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push BTN2
U 1 1 61DB6774
P 2100 6250
F 0 "BTN2" H 2100 6535 50  0000 C CNN
F 1 "2" H 2100 6444 50  0000 C CNN
F 2 "" H 2100 6450 50  0001 C CNN
F 3 "~" H 2100 6450 50  0001 C CNN
	1    2100 6250
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push BTN3
U 1 1 61DB6144
P 2800 5650
F 0 "BTN3" H 2800 5935 50  0000 C CNN
F 1 "3" H 2800 5844 50  0000 C CNN
F 2 "" H 2800 5850 50  0001 C CNN
F 3 "~" H 2800 5850 50  0001 C CNN
	1    2800 5650
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push BTN1
U 1 1 61DB5A08
P 2100 5650
F 0 "BTN1" H 2100 5950 50  0000 C CNN
F 1 "1" H 2100 5850 50  0000 C CNN
F 2 "" H 2100 5850 50  0001 C CNN
F 3 "~" H 2100 5850 50  0001 C CNN
	1    2100 5650
	1    0    0    -1  
$EndComp
Text Label 7600 3350 0    40   ~ 0
PC5
Text Label 7600 5450 0    40   ~ 0
PC12
Text Label 7600 2450 0    40   ~ 0
PA0
Text Label 7600 2350 0    40   ~ 0
PC3
Text Label 7600 2550 0    40   ~ 0
PA1
Text Label 7600 6850 0    40   ~ 0
PC6
Text Label 7600 6750 0    40   ~ 0
PC7
Text Label 7600 6650 0    40   ~ 0
PC8
Text Label 7600 6550 0    40   ~ 0
PC9
Text Label 7600 6450 0    40   ~ 0
PA8
Text Label 7600 6250 0    40   ~ 0
PA10
Text Label 7600 6150 0    40   ~ 0
PA11
Text Label 7600 6050 0    40   ~ 0
PA12
Text Label 7600 5950 0    40   ~ 0
PA13
Text Label 7600 5750 0    40   ~ 0
PA15
Text Label 7600 5650 0    40   ~ 0
PC10
Text Label 7600 5550 0    40   ~ 0
PC11
Text Label 7600 5250 0    40   ~ 0
PB3
Text Label 7600 5150 0    40   ~ 0
PB4
Text Label 7600 5050 0    40   ~ 0
PB5
Text Label 7600 4950 0    40   ~ 0
PB6
Text Label 7600 4850 0    40   ~ 0
PB7
Text Label 7600 4750 0    40   ~ 0
BOOT
Text Label 7600 4650 0    40   ~ 0
PB8
Text Label 7600 4550 0    40   ~ 0
PB9
Text Label 7600 3650 0    40   ~ 0
PB2
Text Label 7600 3550 0    40   ~ 0
PB1
Text Label 7600 3450 0    40   ~ 0
PB0
Text Label 7600 3250 0    40   ~ 0
PC4
Text Label 7600 3150 0    40   ~ 0
PA7
Text Label 7600 3050 0    40   ~ 0
PA6
Text Label 7600 2950 0    40   ~ 0
PA5
Text Label 7600 2650 0    40   ~ 0
PA2
Text Label 7600 2250 0    40   ~ 0
PC2
Text Label 7600 2150 0    40   ~ 0
PC1
Text Label 7600 2050 0    40   ~ 0
PC0
Text Label 7600 1850 0    40   ~ 0
PD1
Text Label 7600 1750 0    40   ~ 0
PD0
Text Label 7600 1650 0    40   ~ 0
PC15
Text Label 7600 1550 0    40   ~ 0
PC14
Text Label 7600 1450 0    40   ~ 0
PC13
Text Label 7600 1350 0    40   ~ 0
VBAT
$Comp
L power:+5V #PWR06
U 1 1 50827384
P 7500 4450
F 0 "#PWR06" H 7500 4300 50  0001 C CNN
F 1 "+5V" H 7500 4590 50  0000 C CNN
F 2 "" H 7500 4450 50  0000 C CNN
F 3 "" H 7500 4450 50  0000 C CNN
	1    7500 4450
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5082736D
P 7500 6950
F 0 "#PWR04" H 7500 6700 50  0001 C CNN
F 1 "GND" H 7500 6800 50  0000 C CNN
F 2 "" H 7500 6950 50  0000 C CNN
F 3 "" H 7500 6950 50  0000 C CNN
	1    7500 6950
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR01
U 1 1 50827354
P 7600 1050
F 0 "#PWR01" H 7600 800 50  0001 C CNN
F 1 "GND" H 7600 900 50  0000 C CNN
F 2 "" H 7600 1050 50  0000 C CNN
F 3 "" H 7600 1050 50  0000 C CNN
	1    7600 1050
	0    1    1    0   
$EndComp
NoConn ~ 7800 4350
Wire Wire Line
	7800 6950 7500 6950
Wire Wire Line
	7800 4450 7500 4450
$Comp
L Device:Buzzer PiezoelectricBuzzer1
U 1 1 61CB600C
P 4650 5700
F 0 "PiezoelectricBuzzer1" H 4800 5750 50  0000 L CNN
F 1 "BPT3510" H 5250 5650 50  0000 L CNN
F 2 "" V 4625 5800 50  0001 C CNN
F 3 "~" V 4625 5800 50  0001 C CNN
	1    4650 5700
	-1   0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R2
U 1 1 61CC0E3A
P 4950 5700
F 0 "R2" H 5018 5746 50  0000 L CNN
F 1 "1K" H 5018 5655 50  0000 L CNN
F 2 "" H 4950 5700 50  0001 C CNN
F 3 "~" H 4950 5700 50  0001 C CNN
	1    4950 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 5600 4750 5450
Wire Wire Line
	4950 5600 4950 5450
Connection ~ 4950 5800
Wire Wire Line
	4950 5800 4750 5800
Wire Wire Line
	4950 5950 4950 5800
$Comp
L Transistor_BJT:BC547 Q1
U 1 1 61CB7F83
P 5050 6150
F 0 "Q1" H 5381 6200 50  0000 R CNN
F 1 "BC547" H 5280 6100 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 5250 6075 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC547.pdf" H 5050 6150 50  0001 L CNN
	1    5050 6150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7100 6150 7100 5450
$Comp
L power:Earth #PWR0103
U 1 1 61D0E678
P 4950 6350
F 0 "#PWR0103" H 4950 6100 50  0001 C CNN
F 1 "Earth" H 4950 6200 50  0001 C CNN
F 2 "" H 4950 6350 50  0001 C CNN
F 3 "~" H 4950 6350 50  0001 C CNN
	1    4950 6350
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0104
U 1 1 61D39970
P 4550 5300
F 0 "#PWR0104" H 4550 5150 50  0001 C CNN
F 1 "+5V" H 4565 5473 50  0000 C CNN
F 2 "" H 4550 5300 50  0001 C CNN
F 3 "" H 4550 5300 50  0001 C CNN
	1    4550 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 5450 4550 5300
Wire Wire Line
	4550 5450 4750 5450
Connection ~ 4750 5450
Wire Wire Line
	4750 5450 4950 5450
$Comp
L Device:R_Small_US R1
U 1 1 61CC081C
P 5350 6150
F 0 "R1" V 5145 6150 50  0000 L CNN
F 1 "1K" V 5236 6150 50  0000 L CNN
F 2 "" H 5350 6150 50  0001 C CNN
F 3 "~" H 5350 6150 50  0001 C CNN
	1    5350 6150
	0    1    1    0   
$EndComp
Text Notes 4450 4600 0    89   ~ 18
Buzzer
$Comp
L Connector_Generic:Conn_01x28 P2
U 1 1 50827286
P 8000 5650
F 0 "P2" H 8150 4200 60  0000 C CNN
F 1 "Header 28" V 8100 5650 60  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x28_P2.54mm_Vertical" V 8200 5650 30  0000 C CNN
F 3 "" H 8000 5650 60  0001 C CNN
	1    8000 5650
	1    0    0    1   
$EndComp
Text Label 7600 5350 0    40   ~ 0
PD2
$Comp
L power:GND #PWR02
U 1 1 50827361
P 7500 3750
F 0 "#PWR02" H 7500 3500 50  0001 C CNN
F 1 "GND" H 7500 3600 50  0000 C CNN
F 2 "" H 7500 3750 50  0000 C CNN
F 3 "" H 7500 3750 50  0000 C CNN
	1    7500 3750
	0    1    1    0   
$EndComp
Wire Wire Line
	3850 2400 4950 2400
Connection ~ 3850 2300
Wire Wire Line
	3850 2300 4950 2300
Connection ~ 4450 2200
Wire Wire Line
	4450 2200 4950 2200
Text Notes 1250 1150 0    89   ~ 18
RTC module
$Comp
L power:+5V #PWR0110
U 1 1 61E0C724
P 3100 1400
F 0 "#PWR0110" H 3100 1250 50  0001 C CNN
F 1 "+5V" H 3115 1573 50  0000 C CNN
F 2 "" H 3100 1400 50  0001 C CNN
F 3 "" H 3100 1400 50  0001 C CNN
	1    3100 1400
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0113
U 1 1 61D0CBE1
P 3850 1400
F 0 "#PWR0113" H 3850 1250 50  0001 C CNN
F 1 "+5V" H 3865 1573 50  0000 C CNN
F 2 "" H 3850 1400 50  0001 C CNN
F 3 "" H 3850 1400 50  0001 C CNN
	1    3850 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1400 1550 1850 1550
Wire Wire Line
	3100 1400 3100 1550
Wire Wire Line
	3850 1550 3850 1400
Connection ~ 3850 1550
Wire Wire Line
	4450 1550 3850 1550
Wire Wire Line
	4450 1700 4450 1550
Wire Wire Line
	4450 1900 4450 2200
$Comp
L Device:R_Small_US R7
U 1 1 61D20975
P 4450 1800
F 0 "R7" H 4200 1850 50  0000 L CNN
F 1 "4.7K" H 4200 1750 50  0000 L CNN
F 2 "" H 4450 1800 50  0001 C CNN
F 3 "~" H 4450 1800 50  0001 C CNN
	1    4450 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 1900 3850 2300
Wire Wire Line
	3850 1550 3850 1700
$Comp
L Device:R_Small_US R6
U 1 1 61D084D8
P 3850 1800
F 0 "R6" H 3550 1850 50  0000 L CNN
F 1 "4.7K" H 3550 1750 50  0000 L CNN
F 2 "" H 3850 1800 50  0001 C CNN
F 3 "~" H 3850 1800 50  0001 C CNN
	1    3850 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 2200 1850 2500
Connection ~ 1850 2500
Wire Wire Line
	1600 3200 3850 3200
Wire Wire Line
	1600 2500 1600 3200
Wire Wire Line
	1850 2500 1600 2500
Wire Wire Line
	3500 2300 3850 2300
Wire Wire Line
	3500 2200 4450 2200
$Comp
L power:+BATT #PWR0112
U 1 1 61E0DA04
P 2900 1950
F 0 "#PWR0112" H 2900 1800 50  0001 C CNN
F 1 "+BATT" H 2750 2050 50  0000 C CNN
F 2 "" H 2900 1950 50  0001 C CNN
F 3 "" H 2900 1950 50  0001 C CNN
	1    2900 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 1950 2900 2000
$Comp
L power:Earth #PWR0111
U 1 1 61E28DEE
P 1400 1850
F 0 "#PWR0111" H 1400 1600 50  0001 C CNN
F 1 "Earth" H 1400 1700 50  0001 C CNN
F 2 "" H 1400 1850 50  0001 C CNN
F 3 "~" H 1400 1850 50  0001 C CNN
	1    1400 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 61E283FD
P 1400 1700
F 0 "C1" H 1518 1746 50  0000 L CNN
F 1 "1uF" H 1518 1655 50  0000 L CNN
F 2 "" H 1438 1550 50  0001 C CNN
F 3 "~" H 1400 1700 50  0001 C CNN
	1    1400 1700
	1    0    0    -1  
$EndComp
Connection ~ 1850 1550
Wire Wire Line
	1850 2000 1850 1550
$Comp
L Device:R_Small_US R5
U 1 1 61E1FB78
P 1850 2100
F 0 "R5" H 1918 2146 50  0000 L CNN
F 1 "4.7K" H 1918 2055 50  0000 L CNN
F 2 "" H 1850 2100 50  0001 C CNN
F 3 "~" H 1850 2100 50  0001 C CNN
	1    1850 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 1550 3100 2000
Connection ~ 3100 1550
Wire Wire Line
	3100 1550 1850 1550
Wire Wire Line
	2500 2500 1850 2500
Wire Wire Line
	2900 2000 3000 2000
$Comp
L power:Earth #PWR0109
U 1 1 61E06219
P 3000 2800
F 0 "#PWR0109" H 3000 2550 50  0001 C CNN
F 1 "Earth" H 3000 2650 50  0001 C CNN
F 2 "" H 3000 2800 50  0001 C CNN
F 3 "~" H 3000 2800 50  0001 C CNN
	1    3000 2800
	1    0    0    -1  
$EndComp
NoConn ~ 2500 2200
NoConn ~ 3500 2600
Wire Notes Line
	1250 1450 1250 3300
Wire Notes Line
	1250 3300 4550 3300
Wire Notes Line
	4550 3300 4550 1450
Wire Notes Line
	4550 1450 1250 1450
Wire Wire Line
	3850 3200 3850 2400
Text Label 4700 2400 0    40   ~ 0
PB12
$Comp
L Connector_Generic:Conn_01x06 P3
U 1 1 50827295
P 5150 2400
F 0 "P3" H 5150 2000 60  0000 C CNN
F 1 "Header 6" V 5250 2400 60  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" V 5350 2400 30  0000 C CNN
F 3 "" H 5150 2400 60  0001 C CNN
	1    5150 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2600 4650 2600
Wire Wire Line
	4950 2700 4650 2700
Text Label 4700 2700 0    40   ~ 0
PB15
Text Label 4700 2600 0    40   ~ 0
PB14
Text Label 4700 2300 0    40   ~ 0
PB11
$Comp
L Timer_RTC:DS3231M RTC1
U 1 1 61DC83A7
P 3000 2400
F 0 "RTC1" H 2800 1950 50  0000 C CNN
F 1 "DS3231M" H 2719 1854 50  0000 C CNN
F 2 "Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm" H 3000 1800 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS3231.pdf" H 3270 2450 50  0001 C CNN
	1    3000 2400
	-1   0    0    -1  
$EndComp
Text Notes 6200 1150 0    89   ~ 18
LCD screen
Text Notes 5650 4600 0    89   ~ 18
Temperature sensor
$Comp
L power:+5V #PWR0102
U 1 1 61ED953E
P 6400 5050
F 0 "#PWR0102" H 6400 4900 50  0001 C CNN
F 1 "+5V" H 6415 5223 50  0000 C CNN
F 2 "" H 6400 5050 50  0001 C CNN
F 3 "" H 6400 5050 50  0001 C CNN
	1    6400 5050
	1    0    0    -1  
$EndComp
$Comp
L power:Earth #PWR0101
U 1 1 61ED8E6D
P 6400 5650
F 0 "#PWR0101" H 6400 5400 50  0001 C CNN
F 1 "Earth" H 6400 5500 50  0001 C CNN
F 2 "" H 6400 5650 50  0001 C CNN
F 3 "~" H 6400 5650 50  0001 C CNN
	1    6400 5650
	1    0    0    -1  
$EndComp
Wire Notes Line
	6700 5050 6700 5650
Wire Notes Line
	6700 5650 6100 5650
Wire Notes Line
	6100 5650 6100 5050
Wire Notes Line
	6100 5050 6700 5050
$Comp
L Sensor_Temperature:DS18B20 TS1
U 1 1 61E39F9F
P 6400 5350
F 0 "TS1" H 6050 5550 50  0000 R CNN
F 1 "DS18B20" H 6050 5450 50  0000 R CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 5400 5100 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf" H 6250 5600 50  0001 C CNN
	1    6400 5350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 50827367
P 7500 4250
F 0 "#PWR03" H 7500 4000 50  0001 C CNN
F 1 "GND" H 7500 4100 50  0000 C CNN
F 2 "" H 7500 4250 50  0000 C CNN
F 3 "" H 7500 4250 50  0000 C CNN
	1    7500 4250
	0    1    1    0   
$EndComp
Wire Wire Line
	7100 5450 7800 5450
$Comp
L Device:R_Small_US R3
U 1 1 61ED66F2
P 6850 5350
F 0 "R3" V 6645 5350 50  0000 L CNN
F 1 "4.7K" V 6736 5350 50  0000 L CNN
F 2 "" H 6850 5350 50  0001 C CNN
F 3 "~" H 6850 5350 50  0001 C CNN
	1    6850 5350
	0    1    1    0   
$EndComp
Wire Wire Line
	6950 5350 7800 5350
Wire Wire Line
	6700 5350 6750 5350
Wire Wire Line
	7500 4250 7800 4250
NoConn ~ 7550 6850
NoConn ~ 7550 6750
NoConn ~ 7550 6650
NoConn ~ 7550 6550
NoConn ~ 7550 6450
NoConn ~ 7550 6250
NoConn ~ 7550 6150
NoConn ~ 7550 6050
NoConn ~ 7550 5950
NoConn ~ 7550 5850
NoConn ~ 7550 5750
NoConn ~ 7550 5650
NoConn ~ 7550 5550
Wire Wire Line
	7200 1950 7200 2150
Wire Wire Line
	7300 1850 7300 2250
Wire Wire Line
	7400 1750 7400 2350
Text Label 7600 2750 0    40   ~ 0
PA3
NoConn ~ 12150 7500
Wire Wire Line
	7800 3750 7500 3750
NoConn ~ 7800 1150
Wire Wire Line
	7400 2350 7800 2350
Wire Wire Line
	7300 2250 7800 2250
Wire Wire Line
	7200 2150 7800 2150
Wire Wire Line
	7600 1050 7800 1050
Text GLabel 7550 2550 0    40   Output ~ 0
PA1
Text GLabel 7550 2650 0    40   Output ~ 0
PA2
Text GLabel 7550 2750 0    40   Output ~ 0
PA3
Text GLabel 7550 2850 0    40   Output ~ 0
PA3
$Comp
L power:+5V #PWR0108
U 1 1 61DCC122
P 6000 2950
F 0 "#PWR0108" H 6000 2800 50  0001 C CNN
F 1 "+5V" V 6015 3078 50  0000 L CNN
F 2 "" H 6000 2950 50  0001 C CNN
F 3 "" H 6000 2950 50  0001 C CNN
	1    6000 2950
	0    -1   -1   0   
$EndComp
NoConn ~ 7000 2250
NoConn ~ 7000 2350
NoConn ~ 7000 2450
$Comp
L power:Earth #PWR0105
U 1 1 6200B18E
P 7000 2850
F 0 "#PWR0105" H 7000 2600 50  0001 C CNN
F 1 "Earth" H 7000 2700 50  0001 C CNN
F 2 "" H 7000 2850 50  0001 C CNN
F 3 "~" H 7000 2850 50  0001 C CNN
	1    7000 2850
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small_US R4
U 1 1 61DCFD4B
P 6100 2950
F 0 "R4" V 5900 2900 50  0000 L CNN
F 1 "3.3K" V 6000 2900 50  0000 L CNN
F 2 "" H 6100 2950 50  0001 C CNN
F 3 "~" H 6100 2950 50  0001 C CNN
	1    6100 2950
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR0106
U 1 1 61FE3D28
P 6600 3150
F 0 "#PWR0106" H 6600 3000 50  0001 C CNN
F 1 "+5V" H 6615 3323 50  0000 C CNN
F 2 "" H 6600 3150 50  0001 C CNN
F 3 "" H 6600 3150 50  0001 C CNN
	1    6600 3150
	1    0    0    1   
$EndComp
NoConn ~ 6200 2550
NoConn ~ 6200 2650
Wire Wire Line
	7000 1750 7400 1750
Wire Wire Line
	7000 1850 7300 1850
Wire Wire Line
	7000 1950 7200 1950
Wire Wire Line
	7000 2050 7800 2050
NoConn ~ 7000 2150
$Comp
L power:Earth #PWR0107
U 1 1 61FB27CA
P 6600 1550
F 0 "#PWR0107" H 6600 1300 50  0001 C CNN
F 1 "Earth" H 6600 1400 50  0001 C CNN
F 2 "" H 6600 1550 50  0001 C CNN
F 3 "~" H 6600 1550 50  0001 C CNN
	1    6600 1550
	1    0    0    1   
$EndComp
$Comp
L Display_Character:WC1602A LCD1
U 1 1 61F0E2B2
P 6600 2350
F 0 "LCD1" H 7250 2050 50  0000 C CNN
F 1 "WH1602" H 7200 2150 50  0000 C CNN
F 2 "Display:WC1602A" H 6600 1450 50  0001 C CIN
F 3 "http://www.wincomlcd.com/pdf/WC1602A-SFYLYHTC06.pdf" H 7300 2350 50  0001 C CNN
	1    6600 2350
	-1   0    0    1   
$EndComp
Wire Wire Line
	7000 2750 7250 2750
Wire Wire Line
	7250 2750 7250 3250
Wire Wire Line
	7250 3250 7800 3250
Wire Wire Line
	7000 2950 7150 2950
Wire Wire Line
	7150 2950 7150 3350
Wire Wire Line
	7150 3350 7800 3350
Text Label 7600 2850 0    40   ~ 0
PA4
Text GLabel 7550 6350 0    40   Output ~ 0
USART1_Tx
Text Label 7600 6350 0    40   ~ 0
PA9
Wire Wire Line
	5450 6150 7100 6150
NoConn ~ 4650 2500
Text Label 4700 2500 0    40   ~ 0
PB13
Wire Wire Line
	4950 2500 4650 2500
NoConn ~ 4650 2600
NoConn ~ 4650 2700
Text Label 4700 2200 0    40   ~ 0
PB10
Text Label 7600 1950 0    40   ~ 0
RST
$Comp
L power:+3.3V #PWR05
U 1 1 50827375
P 7550 1250
F 0 "#PWR05" H 7550 1100 50  0001 C CNN
F 1 "+3.3V" H 7550 1390 50  0000 C CNN
F 2 "" H 7550 1250 50  0000 C CNN
F 3 "" H 7550 1250 50  0000 C CNN
	1    7550 1250
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x28 P1
U 1 1 50827277
P 8000 2350
F 0 "P1" H 8150 3650 60  0000 C CNN
F 1 "Header 28" V 8100 2350 60  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x28_P2.54mm_Vertical" V 8200 2350 30  0000 C CNN
F 3 "" H 8000 2350 60  0001 C CNN
	1    8000 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 3450 7550 3450
Wire Wire Line
	7800 3550 7550 3550
Wire Wire Line
	7800 3650 7550 3650
NoConn ~ 7550 3450
NoConn ~ 7550 3550
NoConn ~ 7550 3650
Wire Wire Line
	7800 3150 7550 3150
NoConn ~ 7550 3150
Wire Wire Line
	7800 3050 7550 3050
Wire Wire Line
	7800 2950 7550 2950
NoConn ~ 7550 3050
NoConn ~ 7550 2950
Wire Wire Line
	7800 2850 7550 2850
Wire Wire Line
	7800 2750 7550 2750
Wire Wire Line
	7800 2650 7550 2650
Wire Wire Line
	7800 2550 7550 2550
Wire Wire Line
	7800 2450 7550 2450
NoConn ~ 7550 2450
Wire Wire Line
	7800 1950 7600 1950
NoConn ~ 7550 1850
NoConn ~ 7550 1750
NoConn ~ 7550 1650
NoConn ~ 7550 1550
NoConn ~ 7550 1450
NoConn ~ 7550 1350
Wire Wire Line
	7550 1350 7800 1350
Wire Wire Line
	7550 1450 7800 1450
Wire Wire Line
	7550 1650 7800 1650
Wire Wire Line
	7550 1750 7800 1750
Wire Wire Line
	7550 1850 7800 1850
Wire Wire Line
	7550 1550 7800 1550
Wire Wire Line
	7800 1250 7550 1250
Text Label 7600 5850 0    40   ~ 0
PA14
Wire Wire Line
	7800 4750 7600 4750
NoConn ~ 7550 4550
NoConn ~ 7550 4650
Wire Wire Line
	7550 4550 7800 4550
Wire Wire Line
	7550 4650 7800 4650
NoConn ~ 7550 4850
NoConn ~ 7550 4950
NoConn ~ 7550 5050
NoConn ~ 7550 5150
NoConn ~ 7550 5250
Wire Wire Line
	7800 5550 7550 5550
Wire Wire Line
	7800 5650 7550 5650
Wire Wire Line
	7800 5750 7550 5750
Wire Wire Line
	7800 5850 7550 5850
Wire Wire Line
	7800 5950 7550 5950
Wire Wire Line
	7800 6050 7550 6050
Wire Wire Line
	7800 6150 7550 6150
Wire Wire Line
	7800 6250 7550 6250
Wire Wire Line
	7800 6450 7550 6450
Wire Wire Line
	7800 6550 7550 6550
Wire Wire Line
	7800 6650 7550 6650
Wire Wire Line
	7800 6750 7550 6750
Wire Wire Line
	7800 6850 7550 6850
Wire Wire Line
	7550 6350 7800 6350
Wire Wire Line
	7550 5250 7800 5250
Wire Wire Line
	7800 5150 7550 5150
Wire Wire Line
	7800 5050 7550 5050
Wire Wire Line
	7800 4950 7550 4950
Wire Wire Line
	7800 4850 7550 4850
$EndSCHEMATC
