// this method makes a HTTP connection to the server
bool getConfig(){
  wifiClient.setTimeout(10000);//set timeout for the connection

  if (!wifiClient.connect("www.mocky.io", 80)) { //connect to the remote host
    Serial.println(F("Connection failed"));
    return false;//if we wasn't able to connect to the server we will return false, so the caller can take the right choice
  }
  Serial.println(F("Connected!"));

  // Send HTTP request to the remote host
  wifiClient.println(F("GET /v2/5dd96fdd3200005f009a87d1 HTTP/1.1"));
  wifiClient.println(F("Host: www.mocky.io"));
  wifiClient.println(F("Connection: close"));
  
  if (wifiClient.println() == 0) {
    Serial.println(F("Failed to send request"));
    return false;//if we fail to send the request we will return false
  }

  // Check HTTP status
  char status[32] = {0};
  wifiClient.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return false;//if we get a response from the server that is non 200 OK we will return false
  }

  // At the end of an HTTP Header there are 2 line, i will skip them
  char endOfHeaders[] = "\r\n\r\n";
  if (!wifiClient.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return false;//if we dont have the two carriage return and the twa line feeder, so the response is malformed we will return false
  }

  // Allocate the right resource for the JSON document
  // This can be calculated using the arduinojson.org/v6/assistant .
  const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
  DynamicJsonDocument jsonParsed(capacity);//json mapping
  
  DeserializationError error = deserializeJson(jsonParsed, wifiClient);// Parse JSON object and collect possible deserialization error in "error"
  
  if (error) {//if there was an error deserializing the json contained into the response body
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());//print the error into the serial
    return false; //if the deserializzation will fail, i will return to the caller false
  }
  //set the params retrieved into the global variable
  enabled = jsonParsed["enabled"];
  desiredTemp = jsonParsed["desiredTemp"];

  //log the response value
  Serial.println(F("Response:"));
  Serial.print("enabled : ");
  Serial.println(enabled ? "true" : "false");
  Serial.print("desiredTemp : ");
  Serial.println(desiredTemp);
  
/*
  // Allocate the right resource for the JSON document
  // This can be calculated using the arduinojson.org/v6/assistant .
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(6) + 400;
  
  DynamicJsonDocument jsonParsed(capacity);//json mapping

  //my server hosting service have bad configured server, so they will return me unwanted characters
  //i've to skip them, if not the json mapping will fail
  char unwanted[10] = {0};
  wifiClient.readBytesUntil('\n', unwanted, sizeof(unwanted));
  
  // Parse JSON object
  DeserializationError error = deserializeJson(jsonParsed, wifiClient);
  wifiClient.stop();
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false; //if the deserializzation will fail, i will return to the caller false
  }

  // Extract values form the json and set new setting
  Serial.println(F("Response:"));
  Serial.println(jsonParsed["date"][0].as<char*>());
  Serial.println(jsonParsed["city"][0].as<char*>());
  Serial.println(jsonParsed["status"][0].as<char*>());
  Serial.println(jsonParsed["date"][1].as<char*>());
  Serial.println(jsonParsed["city"][1].as<char*>());
  Serial.println(jsonParsed["status"][1].as<char*>());
  // Disconnect
 */ 
  wifiClient.stop(); //close the connection to the server
  return true;
}


void sendConfigAndStatus(){
  //for first we need to serialize all the data into a json string
  //data serialized into the json will be
  //configuration, containing desiredTemp and enabled
  //status, containing readTemp and relayOpened


  
  
  
}
