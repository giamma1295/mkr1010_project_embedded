void enbBtnCallback(){
  //Serial.println("enbBtnCallback exec");
  enabled = !enabled; //toggle enable
  refreshScreen = true;
  lastBtnPress = millis();
  // due the pushConfig will be called after 5 sec from btn push
    // tmsLastConfig will be set 5 seconds from now
  tmsLastConfig = rtc.getEpoch() + 5;
  configMauallyChanged = true;
}

void plusBtnCallback(){
  //Serial.println("plusBtnCallback exec");
  if(desiredTemp < 36){
    desiredTemp = desiredTemp + 0.5f;
    refreshScreen = true;
    lastBtnPress = millis();
    // due the pushConfig will be called after 5 sec from btn push
    // tmsLastConfig will be set 5 seconds from now
    tmsLastConfig = rtc.getEpoch() + 5;
    configMauallyChanged = true;
  }
  
}

void minusBtnCallback(){
  //Serial.println("minusBtnCallback exec");
  if(desiredTemp > 15){
    desiredTemp = desiredTemp - 0.5f;
    refreshScreen = true;
    lastBtnPress = millis();
    // due the pushConfig will be called after 5 sec from btn push
    // tmsLastConfig will be set 5 seconds from now
    tmsLastConfig = rtc.getEpoch() + 5;
    configMauallyChanged = true;
  }
}
