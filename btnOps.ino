void enbBtnCallback(){
  //Serial.println("enbBtnCallback exec");
  enabled = !enabled; //toggle enable
  mainScreen();
  lastBtnPress = millis();
  configMauallyChanged = true;
}

void plusBtnCallback(){
  //Serial.println("plusBtnCallback exec");
  if(desiredTemp < 36){
    desiredTemp = desiredTemp + 0.5f;
    mainScreen();
    lastBtnPress = millis();
    configMauallyChanged = true;
  }
  
}

void minusBtnCallback(){
  //Serial.println("minusBtnCallback exec");
  if(desiredTemp > 15){
    desiredTemp = desiredTemp - 0.5f;
    mainScreen();
    lastBtnPress = millis();
    configMauallyChanged = true;
  }
}
