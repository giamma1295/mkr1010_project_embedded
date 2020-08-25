void toggleRelay(){
  Serial.print("toggleRelay()  -- ");
   // The first thing to be checked is the enable variable
      // true -> ok we can do all the stuff needed
      // false -> the boiler should be turned off
  if(!enabled){
    openRelay();
    return;
  }

  if(relayOpened){
    //Boiler turned off
    //if read temp is <= desired - delta, we will turn on the boiler
    if(readTemp <= (desiredTemp - delta)){
      closeRelay();
      Serial.println( "!!!boiler turned on right now!!!");
    }
    else{
      Serial.println("!!!boiler remain turned off because temperature is not lower than desired - delta!!!");
    }
  }
  else{
    //Boiler turned on
    //if read temp is >= desired + delta, we will turn off the boiler
    if(readTemp >= (desiredTemp + delta)){
      openRelay();
      Serial.println( "!!!boiler turned off right now!!!");
    }
    else{
      Serial.println("!!!boiler remain turned on because temperature is not higher than desired + delta!!!");
    }
  }
}

void openRelay(){
  //spengere la caldaia
  digitalWrite(RELAY_PIN, HIGH);// HIGH -> relay turned off (opened)
  relayOpened = true;
  Serial.print("Boiler Turned Off -> readTemp : ");
  Serial.print(readTemp,2);
  Serial.print(" deiredTemp : ");
  Serial.println(desiredTemp,2);
  Serial.print(" enabled : ");
  Serial.println(enabled);
  
}

void closeRelay(){
   //accendere la caldaia
  digitalWrite(RELAY_PIN, LOW);// low -> relay turned on (closed)
  relayOpened = false;
  Serial.print("Boiler Turned on -> readTemp : ");
  Serial.print(readTemp,2);
  Serial.print(" deiredTemp : ");
  Serial.println(desiredTemp,2);
  Serial.print(" enabled : ");
  Serial.println(enabled);
}
