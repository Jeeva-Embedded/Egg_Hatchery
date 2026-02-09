ESP32 Multi-Sensor Industrial Data Logger
with environmental sensing, electrical monitoring, RPM measurement, RTC timestamps, and SD logging.

Here’s a clean, professional README you can paste directly into `README.md`.

ESP32 Egg Hatchery Data Logger

Overview

The system records and logs:

* Temperature & humidity (BME680)
* Temperature & humidity (DHT22 backup sensor)
* Voltage & current (PZEM004T)
* RPM (interrupt-based sensor)
* Mechanical position (Front/Back/Turning)
* Date & time (DS3231 RTC)

All readings are shown on an LCD and saved to an SD card.

Features

* Dual temperature & humidity sensing
* Voltage & current monitoring
* Real-time RPM measurement
* RTC timestamped logging
* CSV data logging to SD card
* 16×2 LCD live display
* Button-based position detection
* Interrupt-driven RPM counter
* Automatic file creation
* Structured data logging format

Hardare Requirements

* ESP32 development board
* BME680 environmental sensor
* DHT22 temperature/humidity sensor
* PZEM004T v3.0 power monitor
* DS3231 RTC module
* 16×2 I2C LCD display
* SD card module
* RPM sensor (hall/optical)
* 3 push buttons
* LEDs
* Power supply


Pin Configuration

| Component    | GPIO                       |
| ------------ | -------------------------- |
| BME680 CS    | 5                          |
| DHT22        | 4                          |
| Front Button | 35                         |
| Back Button  | 34                         |
| Log Button   | 32                         |
| RPM Sensor   | 33                         |
| LED 1        | 15                         |
| LED 2        | 2                          |
| PZEM RX      | 16                         |
| PZEM TX      | 17                         |
| SD Card CS   | ⚠ change to avoid conflict |

> Important: SD card CS pin must not share the BME680 CS pin.


Software Requirements

* Arduino IDE or PlatformIO
* ESP32 board package
* Required libraries:


Adafruit BME680
Adafruit Sensor
DHT sensor library
PZEM004Tv30
LiquidCrystal I2C
RTClib
SD
FS
SPI



Proect Structure


ESP32_DataLogger/
├── src/
│   └── main.ino
├── lib/
├── include/
├── README.md

Installation & Upload

1. Install Arduino IDE or PlatformIO
2. Connect ESP32 via USB
3. Install required libraries
4. Upload sketch
5. Insert FAT32 SD card
6. Power the system


Data Logging Format

Data is stored in:


/data.csv

Header:
ReadingID,Date,Time,BME_Temp,BME_Hum,DHT_Temp,DHT_Hum,Position,RPM,Voltage,Current

Example entry:
12,9-2-2026,14:32,78.5F,45%,79.1F,43%,F,1200,230V,1.2A

LCD Display Cycle
The LCD rotates through screens:

Screen 1
Date & Time

Screen 2
BME Temp | Hum
DHT Temp | Hum

Screen 3
Voltage | Current
Position | RPM

RPM Measurement

RPM is calculated using hardware interrupts:

RPM = pulses_per_second × 60

Timer updates every 1 second.

This provides accurate real-time rotational speed.
System Workflow
1. Read RTC timestamp
2. Collect environmental data
3. Read electrical parameters
4. Count RPM via interrupt
5. Detect mechanical position
6. Update LCD
7. Log to SD card
8. Repeat
