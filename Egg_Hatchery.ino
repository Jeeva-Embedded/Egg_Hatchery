#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "DHT.h"
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "RTClib.h"
RTC_DS3231 rtc;
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
#define BME_SCK 18//temp-1
#define BME_MISO 19//temp-1
#define BME_MOSI 23//temp-1
#define BME_CS 5//temp-1

#define DHTPIN 4//temp-2
#define DHTTYPE DHT22

#define frontButton 35      // the number of the Front button
#define backButton 34       // the number of the Back Button
#define RPMpin 33              //RPM Sensor
#define logButton 32        // the number of the Back Button
#define led1 15             // the number of the LED pin-1
#define led2 2              // the number of the LED pin-2


#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif

#define SEALEVELPRESSURE_HPA (1013.25)
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BME680 bme; // I2C

#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#endif

//void RPM_check();
//void getTimeStamp();
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void SD_File();

LiquidCrystal_I2C lcd(0x27, 16, 2);

float f1=0;
float h1=0;
float h,f,voltage,current;
char pos;
int buttonState = 0;  // variable for reading the pushbutton status
bool FB;
bool BB;
bool LB;
bool connect;

//RPM starts
int wings= 1; // no of wings of rotating object, for disc object use 1 with white tape on one side
float value=0;
hw_timer_t * timer = NULL;
volatile byte state = LOW;
int rev=0, RPM=0;
int oldtime=0;
int newtime;
int date,month,year,hours,minutes;

struct Button {
	const uint8_t PIN;
	uint32_t numberKeyPresses;
	bool pressed;
};

Button button1 = {33, 0, false};

void IRAM_ATTR isr() { 
	button1.numberKeyPresses++;
	button1.pressed = true;
}

void IRAM_ATTR onTimer(){
  RPM = (button1.numberKeyPresses/1)*60;
  button1.numberKeyPresses=0;
}

// Define CS pin for the SD card module
#define SD_CS 5

// Save reading number on RTC memory
RTC_DATA_ATTR int readingID = 0;

String dataMessage;
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP);

// Variables to save date and time
//String formattedDate;
//String dayStamp;
//String timeStamp;
//SD Ends

void setup() {
  lcd.init();         // initialize the lcd
  lcd.backlight();    // open the backlight

  Serial.begin(115200);//temp-1
  rtc.begin();
  /*
    // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int count=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(count);
    count = count+1;
    if(count == 15)
    {
    break;
    }
  }
  */

//rtc.adjust(DateTime(__DATE__, __TIME__));
  lcd.clear();
  lcd.setCursor(0, 0);  // display position

/*
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.print("Wifi Connected");     // display the temperature1 in Celsius
      timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(19800);
  }
  else
  {
    lcd.print("** Wifi NOT **");     // display the temperature1 in Celsius
  }
  */

    lcd.setCursor(0, 1);  // display position
    SD.begin(SD_CS);
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        lcd.print("Card Mount Fail");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        lcd.print("No SD card");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
        lcd.print("SD Type: MMC");
        void SD_File();
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
        lcd.print("SD Type: SDSC");
        void SD_File();
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
        lcd.print("SD Type: SDHC");
        void SD_File();
    } else {
        Serial.println("UNKNOWN");
        lcd.print("SD Type: UNKNOWN");
    }

    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);  // display position

/*
  getTimeStamp();
  lcd.print(dayStamp);
  lcd.setCursor(0, 1);  // display position
  lcd.print(timeStamp);
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);  // display position

*/
   if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  dht.begin();

  // initialize the LED pin1 as an output:
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(frontButton, INPUT);
  pinMode(backButton, INPUT);
  pinMode(logButton, INPUT);
  pinMode(RPMpin, INPUT);

  pinMode(button1.PIN, INPUT_PULLUP);
	attachInterrupt(button1.PIN, isr, FALLING);
  pinMode(33,INPUT);
  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 80, true);

  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);

  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  timerAlarmWrite(timer, 1000000, true);

  /* Start an alarm */
  timerAlarmEnable(timer);

}

void loop() {
  //RPM_check();
DateTime now = rtc.now();

date = now.day();
month = now.month();
year = now.year();
hours = now.hour();
minutes = now.minute();

Serial.print("Time:");
Serial.print(hours);
Serial.print(":");
Serial.println(minutes);

Serial.print("Date:");
Serial.print(date);
Serial.print("-");
Serial.print(month);
Serial.print("-");
Serial.println(year);


if (button1.pressed) {
		button1.pressed = false;
	}
 Serial.println(RPM);
 delay(1000);





  //getTimeStamp();
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    delay(1000);
    return;
  }

  f1=(bme.temperature * 9/5) + 32;        // celsius to farenhiet

  Serial.print("BME_Temp = ");
  Serial.print(f1);
  Serial.println("°F");

  Serial.print("BME_Hum = ");
  h1 = bme.humidity;
  Serial.print(h1);
  Serial.println(" %");
  Serial.println();

  h = dht.readHumidity();
  f = dht.readTemperature(true);
  // true returns the temperature in Fahrenheit

  if (isnan(h) || isnan(f)) {
    Serial.println(F("Failed reception"));
    return;
    // Returns an error if the ESP32 does not receive any measurements
  }
  Serial.print("DHT_Temp =");
  Serial.print(f);
  Serial.println("°F");
  Serial.print("DHT_Hum =");
  Serial.print(h);
  Serial.println("%");
  
  Serial.print("Custom Address:");
  Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    voltage = pzem.voltage();
    current = pzem.current();

 if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    }
else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");     
         Serial.print(voltage);  
            Serial.println("V");
        Serial.print("Current: ");      
        Serial.print(current);   
           Serial.println("A");
}
    Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);  // display position
  lcd.print("Date: ");
  lcd.print(date);
  lcd.print("-");
  lcd.print(month);
  lcd.print("-");
  lcd.print(year);

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(hours);
  lcd.print(":");
  lcd.print(minutes);
  delay(5000);


  
  lcd.clear();
  lcd.setCursor(0, 0);  // display position
 
  lcd.print(f1);     // display the temperature1 in Celsius
 // lcd.print((char)223); // display ° character
  lcd.print("F");
  lcd.print(" | ");
  lcd.print(h1);     //humidity1
  lcd.print("%");
  lcd.setCursor(0, 1);  // display position
  lcd.print(f);     // display the temperature in Celsius
  lcd.print("F");
  lcd.print(" | ");
  lcd.print(h);     
  lcd.print("%");

  delay(10000);

  lcd.clear();
  lcd.setCursor(0, 0);  // display position
  lcd.print(voltage);     // display the temperature in Celsius
  lcd.print("V");
  lcd.print(" | ");
  lcd.print(current);     
  lcd.print("A");
  lcd.setCursor(0, 1);  // display position

  FB = digitalRead(frontButton);
  BB = digitalRead(backButton);
  LB = digitalRead(logButton);

  if(FB == 1 && BB == 0)
  {
    Serial.print("Position: BACK");
    pos = 'B';
    lcd.print("Back | ");
  }
  else if (FB == 0 && BB == 1)
  {
    Serial.print("Position: FRONT");
    pos = 'F';
    lcd.print("Front | ");
  }
  else
  {
    Serial.print("Position: Turning");
    pos = 'T';
    lcd.print("Turn | ");
  }
    logSDCard();
    lcd.setCursor(8, 1);  // display position
    lcd.print(RPM);
    lcd.setCursor(13, 1);  // display position
    lcd.print("Log");

    delay(5000);
    

  }

void SD_File()
{
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.csv");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    lcd.print("Creating file...");
    writeFile(SD, "/data.csv", "Reading ID, Date, Time, BME_Temp, BME_Hum, DHT_Temp, DHT_Hum, Position, RPM, Voltage, Current \r\n");
  }
  else {
    Serial.println("File already exists");
    lcd.print("File Found");  
  }
  file.close();
  }

  void logSDCard() {
  readingID+1;
  dataMessage = String(readingID) + "," + String(date) + "-" + String(month) + "-" + String(year) + "," + String(hours) + ":" + String(minutes) + "," +
                String(f1) + "F" + "," + String(h1) + "%" + "," + String(f) + "F" + "," +String(h) + "%" + "," +
                String(pos) + "," + String(RPM) + "," + String(voltage) + "V" + "," + String(current) + "A" + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());

}
/*
  void RPM_check()
  {
    delay(1000);
    detachInterrupt(0); //detaches the interrupt
    newtime=millis()-oldtime; //finds the time 
    int RPMnew = rev/wings; //here we used fan which has 3 wings
    rpm=(RPMnew/newtime)*60000; //calculates rpm
    oldtime=millis(); //saves the current time
    rev=0;
    attachInterrupt(digitalPinToInterrupt(RPMpin),isr,RISING);
  }
  */

  /*
// Function to get date and time from NTPClient
void getTimeStamp() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.println(dayStamp);
  
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.println(timeStamp);
}
*/

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}