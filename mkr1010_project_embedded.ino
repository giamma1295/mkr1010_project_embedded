//---Libraries include
#include <Wire.h> // library that allow using i2c devices
#include <LiquidCrystal_I2C.h> // library used to control an i2c connect LCD
#include <DHT.h> // library for the temperature sensor (DHT11)
#include <ArduinoJson.h> //library for json serialization e deserialization
#include <SPI.h>//library used to communicate with SPI devices, onboard wifi is an SPI device
#include <WiFiNINA.h>//wifi library
#include <RTCZero.h>// real time clock library, used to take control of time
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
#define DHTTYPE DHT22 //sensor type

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
RTCZero rtc;// Initialise the Rtc object
//-----------global variables------------

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
char apiToken[] = SECRET_TOKEN; //api token


unsigned long currentTime;//last config timestamp, 
unsigned long tmsLastConfig;//last config timestamp
float readTemp = 0.0;//temp read by the sensor
bool enabled = false;//boolean that enable/disable the thermostat, dispite the temp
float desiredTemp = 0.0;//Desirded temp, under which the thermostath will be enable, of course if and only if enabled is true and of course considering the delta
bool relayOpened = false;//current relay status -> true opened (Cooling Down), false closed (heating Up)
unsigned long lastRead = 0; //mills from last dht22 read
unsigned long lastRetrieve = 0; //millis from last api config request
unsigned long lastSwitch = 0; // millis from last toggleRelay invoke
bool configMauallyChanged = false; //configuration manualy changed by pressing a button
unsigned long lastBtnPress = 0; //millis from last button press, use in void loop as tollerance before send new config, reuqire in order to manage button press in rapid sequence

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

  //Set callback function to interrupts
  attachInterrupt(ENB_BTN, enbBtnCallback, RISING);//callback executed when will be pressed ENB_BTN 
  attachInterrupt(PLUS_BTN, plusBtnCallback, RISING);//callback executed when will be pressed PLUS_BTN 
  attachInterrupt(MINUS_BTN, minusBtnCallback, RISING);//callback executed when will be pressed MINUS_BTN 
  
  //get timestamp from the server and init rtc
  //this because arduino does not have an RTC (madule can be bought)
  //to solve this we can do a call to the server to retrieve the server time
  //and we with this we can emulate the RTC with the RTCzero library
  intiRtcWithServer();

  //get the config from the server
  //if true we got succesfully the config from the server
  //if false there was a problem with the server connection
  //or this is the first time that we use the termostat, so no config is stored onto the server
  //in the last case we will set default settings
  if(!getConfig(false)){//in this case we want the last config stored on to the server
    enabled = false;
    desiredTemp = 22.0;
    //we will also set configMauallyChanged = true, so in void loop we will send the stock config to the server and so we will init it!
    configMauallyChanged = true;
  }
  lastRetrieve =  millis();

  //inti DHT22 and first read
  dht.begin();
  readTemp = dht.readTemperature();
  printReadTemp();

  //relay init
  pinMode(RELAY_PIN, OUTPUT);
  //we need to define for the first time relayOpened
  //if read temp is lower than desired -> relayOpened = false (relay closed at t0)
  //if read temp is higher than desired -> relayOpened = true (relay opened at t0)
  //these condition apply only on the bootstrap phase, check relayOps for the regular condition
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
 
  //- every two seconds i will read the temperature (this becasue DHT22 reads new temps every 2 seconds), and update the display content
  if(millis() - lastRead > 2001){
    Serial.print("Reading temperature... ");
    readTemp = dht.readTemperature();
    printReadTemp();
    lastRead = millis();
    //refresh display
    mainScreen();
  }
  //- every five seconds i will call the server in order to get new config that may was set
  if(millis() - lastRetrieve > 5000){
    Serial.println("Contacting Server in order to check if a new config was set");
    getConfig(true);
    //refresh display
    lastRetrieve =  millis();
    mainScreen();
  }
  //- every ten seconds i will call the toggleRelay function, this function will do all the logic releated to the relay
  if(millis() - lastSwitch > 10000){
    Serial.print("Calling toggle relay... ");
    toggleRelay();
    lastSwitch = millis();
    //refresh display
    mainScreen();
  }
  //- if the boolean "configMauallyChanged" is true i will send to the server the new config
  // the new config will be sent to the server after 5 seconds from last btn press, this to avoid mulfonctioning if a multiple button are pressed in rapid sequence
  // and avoid useless api calls
  if(configMauallyChanged && (millis() - lastBtnPress > 5000)){
    Serial.print("Configuration has changed, push to the server... ");
    //send new config
    pushConfig();
    //after that i will set tmsLastConfig with the current timestamp, so next time i'll call the serve will not refresh the configuration
    tmsLastConfig = rtc.getEpoch();//update the current configuration timestamp
    //clean configMauallyChanged var
    configMauallyChanged = false;
    //refresh display
    mainScreen();
  }

}
