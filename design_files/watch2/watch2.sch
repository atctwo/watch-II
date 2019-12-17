EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "watch2 mainboard"
Date "2019-12-17"
Rev "1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RF_Module:ESP32-WROOM-32 U1
U 1 1 5DF8B1B1
P 5750 2900
F 0 "U1" H 5900 4400 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 6200 4300 50  0000 C CNN
F 2 "Alice's KiCad Footprints:ESP32-WROOM-32" H 5750 1400 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 5450 2950 50  0001 C CNN
	1    5750 2900
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0101
U 1 1 5DF8C016
P 5750 1150
F 0 "#PWR0101" H 5750 1000 50  0001 C CNN
F 1 "+3V3" H 5765 1323 50  0000 C CNN
F 2 "" H 5750 1150 50  0001 C CNN
F 3 "" H 5750 1150 50  0001 C CNN
	1    5750 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 1500 5750 1150
$Comp
L Regulator_Linear:AP1117-33 U2
U 1 1 5DF8DC50
P 1450 750
F 0 "U2" H 1450 992 50  0000 C CNN
F 1 "AP1117-33" H 1450 901 50  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-223-3_TabPin2" H 1450 950 50  0001 C CNN
F 3 "http://www.diodes.com/datasheets/AP1117.pdf" H 1550 500 50  0001 C CNN
	1    1450 750 
	1    0    0    -1  
$EndComp
$Comp
L power:+BATT #PWR0102
U 1 1 5DF8EFAD
P 650 750
F 0 "#PWR0102" H 650 600 50  0001 C CNN
F 1 "+BATT" H 665 923 50  0000 C CNN
F 2 "" H 650 750 50  0001 C CNN
F 3 "" H 650 750 50  0001 C CNN
	1    650  750 
	1    0    0    -1  
$EndComp
Wire Wire Line
	650  750  1100 750 
$Comp
L power:GND #PWR0103
U 1 1 5DF95B86
P 1450 1300
F 0 "#PWR0103" H 1450 1050 50  0001 C CNN
F 1 "GND" H 1455 1127 50  0000 C CNN
F 2 "" H 1450 1300 50  0001 C CNN
F 3 "" H 1450 1300 50  0001 C CNN
	1    1450 1300
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0104
U 1 1 5DF95E48
P 1750 750
F 0 "#PWR0104" H 1750 600 50  0001 C CNN
F 1 "+3.3V" V 1765 878 50  0000 L CNN
F 2 "" H 1750 750 50  0001 C CNN
F 3 "" H 1750 750 50  0001 C CNN
	1    1750 750 
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C2
U 1 1 5DF98C2B
P 1750 1100
F 0 "C2" H 1842 1146 50  0000 L CNN
F 1 "C_Small" H 1842 1055 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 1750 1100 50  0001 C CNN
F 3 "~" H 1750 1100 50  0001 C CNN
	1    1750 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 5DF9948E
P 1100 1100
F 0 "C1" H 900 1150 50  0000 L CNN
F 1 "C_Small" H 700 1050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 1100 1100 50  0001 C CNN
F 3 "~" H 1100 1100 50  0001 C CNN
	1    1100 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 1000 1100 750 
Connection ~ 1100 750 
Wire Wire Line
	1100 750  1150 750 
Wire Wire Line
	1750 1000 1750 750 
Connection ~ 1750 750 
Wire Wire Line
	1450 1300 1450 1200
Wire Wire Line
	1750 1200 1450 1200
Connection ~ 1450 1200
Wire Wire Line
	1450 1200 1450 1050
Wire Wire Line
	1100 1200 1450 1200
$Comp
L power:GND #PWR0105
U 1 1 5DF9AFDA
P 5750 5400
F 0 "#PWR0105" H 5750 5150 50  0001 C CNN
F 1 "GND" H 5755 5227 50  0000 C CNN
F 2 "" H 5750 5400 50  0001 C CNN
F 3 "" H 5750 5400 50  0001 C CNN
	1    5750 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 5400 5750 4300
Text Notes 500  1350 0    50   ~ 0
todo: add lipo charger
$Comp
L Connector:Conn_01x11_Female J1
U 1 1 5DF9F8F2
P 10100 2200
F 0 "J1" H 10128 2226 50  0000 L CNN
F 1 "Conn_01x11_Female" H 10128 2135 50  0000 L CNN
F 2 "" H 10100 2200 50  0001 C CNN
F 3 "~" H 10100 2200 50  0001 C CNN
	1    10100 2200
	1    0    0    -1  
$EndComp
$EndSCHEMATC
