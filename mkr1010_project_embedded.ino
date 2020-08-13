//---Libraries include
#include <Wire.h> // library that allow using i2c devices
#include <LiquidCrystal_I2C.h> // library used to control an i2c connect LCD
#include <DHT.h> // library for the temperature sensor (DHT11)
#include <ArduinoJson.h> //library for json serialization e deserialization
#include <SPI.h>//library used to communicate with SPI devices, onboard wifi is an SPI device
#include <WiFiNINA.h>//wifi library
#include "arduino_secrets.h"//arduino secret, used to store sensitive info, like wifi passwd or API secret token...

//---Costant value
//lcd releated costant
#define I2C_ADDR 0x27 
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

//DHT11 sensor costant
#define DHTPIN 6 //pin on which is connected the sensor
#define DHTTYPE DHT11 //sensor type

//Pin definition
#define RELAY_PIN 7 //pin for relay
#define ENB_BTN 0 //pin for on/off button
#define PLUS_BTN 1 //pin for increase desidered temp
#define MINUS_BTN 8 //pin for decrease desidered temp

//delta enable and disable the relay
//we cannot trigger the relay every time the read temp by the sensor cross the desired temp, because this will happen a lot of times in a short period
//this behaviour can break either the relay or, in an catostrofic scenario, the heater itself
//to avoid this behaviour there are many alternative strategies, the most widely used is the delta one
//here a basic explaination:
//at time t0 we have readTemp < desiredTemp - relay closed, heating Up phase.
//at time t1, t1>t0, we have read temp = desiredTemp, instead of opening the relay, so cooling down, we wait until readTemp will be >= desiredTemp + delta in order to open the relay and go to the cooling down phase,
//at time t2, t2>t1, we have readTemp >= desiredTemp + delta, so we can open the relay and enter into the cooling down phase.
//at time t3, t3>t2, we have readTemp = desiredTemp, instead of closing the relay we wait until readTemp will reach desiredTemp - delta.
//that's why this costant
#define delta 1.0 // 1 degree celsius


//Object
LiquidCrystal_I2C lcd(0x27, 16, 2); //class contain all necessary to control an LCD i2c.0x6B
DHT dht(DHTPIN, DHTTYPE);//Initialize the DHT object
WiFiClient wifiClient;// Initialize the client library
//-----------global variables------------

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
char apiToken[] = SECRET_TOKEN; //api token


unsigned long currentTime;//last config timestamp
unsigned long tmsLastConfig;//last config timestamp
float readTemp = 0.0;//temp read by the sensor
bool enabled = false;//boolean that enable/disable the thermostat, dispite the temp
float desiredTemp = 0.0;//Desirded temp, under which the thermostath will be enable, of course if and only if enabled is true and of course considering the delta
bool relayOpened = false;//current relay status -> true opened (Cooling Down), false closed (heating Up)
unsigned long lastRead = 0;
unsigned long lastSend = 0;
unsigned long lastSwitch = 0;

// Api Endpoint
char server[] = "scogiam95.altervista.org";
String timestampEndPoint = "/res/ems/timestamp.php";
String configurationEndPoint = "/res/ems/configuration.php";

void setup() {

  // initialize serial for debugging
  Serial.begin(9600);

  //lcd initializzation
  initDisplay();
  delay(1000);

  //WiFi Initializzation 
  wifiInit();

  //Buttons configuration
  pinMode(ENB_BTN, INPUT);
  pinMode(PLUS_BTN, INPUT);
  pinMode(MINUS_BTN, INPUT);

  attachInterrupt(ENB_BTN, enbBtnCallback, RISING);
  attachInterrupt(PLUS_BTN, plusBtnCallback, RISING);
  attachInterrupt(MINUS_BTN, minusBtnCallback, RISING);
  
  //get timestamp from the server
  //this because arduino does not have an RTC (madule can be bought)
  //to solve this we can do a call to the server to retrieve the server time
  //and we we need to now the current time we have to do currentTime + millis(), mills() return millisecond from startup not 100% accurate but enough for our use
  getCurrentTime();

  //get the config from the server
  //if true we got succesfully the config from the server
  //if false there was a problem with the server connection
  //or this is the first time that we use the termostat, so no config is stored onto the server
  //in the last case we will set default settings
  if(!getConfig(false)){//in this case we want the last config stored on to the server
    enabled = false;
    desiredTemp = 22.0;
    //and also we will send the config to the server for the first time
    //sendConfigAndStatus();
  }

  //inti DHT11 and first read
  dht.begin();
  readTemp = dht.readTemperature();
  printReadTemp();

  //relay init
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);//relay closed
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);//relay closed
  //we need to define for the first time relayOpened
  //if read temp is lower than desired -> relayOpened = false (relay closed at t0)
  //if read temp is higher than desired -> relayOpened = true (relay opened at t0)
  if(readTemp < desiredTemp){
    digitalWrite(RELAY_PIN, LOW);//relay closed
    relayOpened = false;
    Serial.println("Switched on");
  }
  else{
    digitalWrite(RELAY_PIN, HIGH);//relay opened
    relayOpened = true;
    Serial.println("Switched off");
  }
  //we can now print the Main Screen
  mainScreen();
  
}

void loop() { 
  
  /*lcd.setCursor(0,1);
  lcd.print(n);
  lcd.print(" ");
  delay(1000);
  n++;*/

  //into the loop we will
  //1) read the temp
  //2-) if buttons is pressed: 
  //  2.1) check if a button is pressed -> yes: do the action of that button (1-> toggle enable, 2 -> decrease desired tem, 3 -> increase desired temp)
  //  2.2) send data to the server -> both config and last read temp
  //3-)if buttons is not pressed:
  //  3.1) get data from the server (this only -> if millis - last request > 10000, so only every 10 second we will send data to the server)
  //  3.2) send data to the server(nevertheless we just got the config we will send both stored config and last temp read)
  //4) call the method for switch on/off the relay, so turn on/off the heater
  //5) refresh the screen by calling mainScreen()
  
  if(millis() - lastSwitch > 10000){
    toggleRelay();
    lastSwitch = millis();
  }
  if(millis() - lastRead > 2001){
    readTemp = dht.readTemperature();
    printReadTemp();
    lastRead = millis();
  }

}
