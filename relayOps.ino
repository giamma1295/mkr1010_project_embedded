void toggleRelay(){
    Serial.print("toggleRelay()  -- ");
      /*
      tre fasi
      1) la temperatura letta dal sensore è compresa tra desiredTemp - 1 e desiredTemp + 1 -> non deve commutare stato qualunque esso sia
      2) la temperatura letta dal sensore è minore    di desiredTemp - 1 -> bisogna accendere la caldaia
      3) la temperatura letta dal sensore è maggiore  di desiredTemp + 1 -> bisogna spegnere la caldaia

      //fase 2
      if(readTemp < (desiredTemp - delta)){
        //accendere la caldaia
        digitalWrite(RELAY_PIN, LOW);// low -> relay turned on (closed)
        relayOpened = false;
        Serial.print("Switched on -> readTemp : ");
        Serial.print((readTemp - delta),2);
        Serial.print(" deiredTemp : ");
        Serial.println(desiredTemp,2);
        
      }
      else if(readTemp > (desiredTemp + delta)){
        //spengere la caldaia
        digitalWrite(RELAY_PIN, HIGH);// HIGH -> relay turned off (opened)
        relayOpened = !relayOpened;
        Serial.print("Switched off -> readTemp : ");
        Serial.print((readTemp + delta),2);
        Serial.print(" deiredTemp : ");
        Serial.println(desiredTemp,2);
      
      }
      
      */
   //lower bound crossed turn on the heater, so close the relay
   if(readTemp < (desiredTemp - delta)){
        //accendere la caldaia
        digitalWrite(RELAY_PIN, LOW);// low -> relay turned on (closed)
        relayOpened = false;
        Serial.print("Switched on -> readTemp : ");
        Serial.print(readTemp,2);
        Serial.print(" deiredTemp : ");
        Serial.println(desiredTemp,2);
        
      }
      //higher bound crossed, so close the relay
      else if(readTemp > (desiredTemp + delta)){
        //spengere la caldaia
        digitalWrite(RELAY_PIN, HIGH);// HIGH -> relay turned off (opened)
        relayOpened = !relayOpened;
        Serial.print("Switched off -> readTemp : ");
        Serial.print(readTemp,2);
        Serial.print(" deiredTemp : ");
        Serial.println(desiredTemp,2);
      }

}
