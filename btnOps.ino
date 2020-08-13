void enbBtnCallback(){
  Serial.println("enbBtnCallback");
  enabled = !enabled;
  mainScreen();
}

void plusBtnCallback(){
  Serial.println("plusBtnCallback");
  if(desiredTemp < 30){
    desiredTemp = desiredTemp + 0.5f;
    mainScreen();
  }
  
}


void minusBtnCallback(){
  Serial.println("minusBtnCallback");
  if(desiredTemp > 15){
    desiredTemp = desiredTemp - 0.5f;
    mainScreen();
  }
}
