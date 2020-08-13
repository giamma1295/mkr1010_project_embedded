//Print the main screen
//FIRST LINE -> Heating Status(Idle/Heating) and the temp read by the sensor
//SECOND LINE -> Enabled status(Enabled/Disabled) and the desired temp set by the user

void initDisplay(){
  lcd.init();
  lcd.backlight();
  lcd.home();
  lcd.setCursor(0,0);
  lcd.print("BASIC THERMOSTAT");
  lcd.setCursor(0,1);
  lcd.print("GIANMARIA SCORZA");

}

void printInitWifiMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("INITIALIZING");
  lcd.setCursor(0,1);
  lcd.print("WIFI");
}


void printSSIDConnectedMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CONNECTED TO :");
  lcd.setCursor(0,1);
  lcd.print(ssid);
}

void mainScreen(){
  //print main screen 
 
  lcd.clear(); //clear the lcd.
  lcd.setCursor(0,0);//go to the first row,first columnn and 
  //first line will show information about the relay(Opened -> we are cooling down and so will show Idle, vice versa will show Heating(Closed)) followed by the read temp.
  if(relayOpened){
    lcd.print("Idle    RT " + String(readTemp, 1) + "C");
  }
  else{
    lcd.print("Heating RT " + String(readTemp, 1) + "C");
   }
   
    lcd.setCursor(0,1);  //Defining positon to write from second row,first column .
   //second line will show information about the user setting, Enable/Disable followed by the desired temp.
  if(enabled){
    lcd.print("Enabled DT " + String(desiredTemp, 1) + "C");
  }
  else{
    lcd.print("Disbled DT " + String(desiredTemp, 1) + "C");
    
  }
}

//screen that show to the user a feedback for the "increase desired temp action"
void increaseScreen(){
  
}

//screen that show to the user a feedback for the "decrease desired temp action"
void decreaseScreen(){
  
}

//screen that show to the user a feedback for the "enable/disable action"
void enableScreen(){
  
}
