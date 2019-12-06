// this method makes a HTTP connection to the server
bool getConfig(bool last){
  wifiClient.setTimeout(10000);//set timeout for the connection

  if (!wifiClient.connect(server, 80)) { //connect to the remote host
    Serial.println(F("Connection failed"));
    return false;//if we wasn't able to connect to the server we will return false, so the caller can take the right choice
  }
  Serial.println(F("Connected!"));

  //create the header
  char head[200]; 
  strcat( head, "GET /configuration?token=");
  strcat( head, apiToken);

  //if last is true we will pass also last to get config only last config is higher than the one store on arduino
  if(last){
    //head = head + +"&last=" + tmsLastConfig;
    strcat( head, "&last=");
    char tmsTmp[16];
    itoa(tmsLastConfig,tmsTmp, 10 );
    strcat( head, tmsTmp);
  }
  strcat( head, " HTTP/1.1");
  
  // Send HTTP request to the remote host
  wifiClient.println(head);
  wifiClient.print("Host: ");
  wifiClient.println(server);
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

/*
  //my server hosting service have bad configured server, so they will return me unwanted characters
  //i've to skip them, if not the json mapping will fail
  char unwanted[10] = {0};
  wifiClient.readBytesUntil('\n', unwanted, sizeof(unwanted));
*/ 

  // Allocate the right resource for the JSON document
  // This can be calculated using the arduinojson.org/v6/assistant .
  const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
  DynamicJsonDocument jsonParsed(capacity);//json mapping
  
  DeserializationError error = deserializeJson(jsonParsed, wifiClient);// Parse JSON object and collect possible deserialization error in "error"
  
  if (error) {//if there was an error deserializing the json contained into the response body
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());//print the error into the serial
    return false; //if the deserializzation will fail, i will return to the caller false
  }
  //set the params retrieved into the global variable
  //if the config on arduino is newer than the one on the server we will skip it and mantain the arduino one else we will set the newer

  if(jsonParsed["desiredTemp"] != 0.0 && jsonParsed["tmsConfig"] != 0.0){
    enabled = jsonParsed["enabled"];
    desiredTemp = jsonParsed["desiredTemp"];
    tmsLastConfig = jsonParsed["tmsConfig"];
    
  }

  //log the response value
  Serial.println(F("Response:"));
  Serial.print("enabled : ");
  Serial.println(enabled ? "true" : "false");
  Serial.print("desiredTemp : ");
  Serial.println(desiredTemp);
  
  wifiClient.stop(); //close the connection to the server
  return true;
}


// this method makes a HTTP request to the cloud function to retrieve the current time
bool getCurrentTime(){
  wifiClient.setTimeout(10000);//set timeout for the connection

  if (!wifiClient.connect(server, 80)) { //connect to the remote host
    Serial.println(F("Connection failed"));
    return false;//if we wasn't able to connect to the server we will return false, so the caller can take the right choice
  }
  Serial.println(F("Connected!"));

  //create the header
  String head = "GET /getTimestamp HTTP/1.1";
  
  // Send HTTP request to the remote host
  wifiClient.println(head);
  wifiClient.print("Host: ");
  wifiClient.println(server);
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

/*
  //my server hosting service have bad configured server, so they will return me unwanted characters
  //i've to skip them, if not the json mapping will fail
  char unwanted[10] = {0};
  wifiClient.readBytesUntil('\n', unwanted, sizeof(unwanted));
*/ 

  // Allocate the right resource for the JSON document
  // This can be calculated using the arduinojson.org/v6/assistant .
  const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonDocument jsonParsed(capacity);
  DeserializationError error = deserializeJson(jsonParsed, wifiClient);// Parse JSON object and collect possible deserialization error in "error"
  if (error) {//if there was an error deserializing the json contained into the response body
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());//print the error into the serial
    return false; //if the deserializzation will fail, i will return to the caller false
  }

  currentTime = jsonParsed["timestamp"];

  //log the response value
  Serial.print(F("Retrieved Timestamp from the server : "));
  Serial.print(currentTime);
  
  wifiClient.stop(); //close the connection to the server
  return true;
}


void pushConfig(){
  //for first we need to serialize all the data into a json string
  //data serialized into the json will be
  //configuration, containing desiredTemp and enabled
  //status, containing readTemp and relayOpened

  //serialize the config variable into a json
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);

  doc["desiredTemp"] = desiredTemp;
  doc["enabled"] = enabled;
  doc["tmsConfig"] = tmsLastConfig;

  String jsonSerialized = "";
  serializeJson(doc, jsonSerialized);
  Serial.println("Json Serialized : ");
  Serial.println(jsonSerialized);

  //connection
  wifiClient.setTimeout(10000);//set timeout for the connection

  if (!wifiClient.connect(server, 80)) { //connect to the remote host
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(F("Connected!"));

  //create the header
  char head[200]; 
  strcat( head, "GET /configuration?token=");
  strcat( head, apiToken);
  strcat( head, " HTTP/1.1");
  
  // Send HTTP request to the remote host
  wifiClient.println(head);
  wifiClient.print("Host: ");
  wifiClient.println(server);
  wifiClient.println(F("Connection: close"));
  wifiClient.println("Content-Type: application/json;");
  wifiClient.print("Content-Length: ");
  wifiClient.println(jsonSerialized.length());
  wifiClient.println();
  wifiClient.println(jsonSerialized);
  
  
  if (wifiClient.println() == 0) {
    Serial.println(F("Failed to send request"));
  }

  // Check HTTP status
  char status[32] = {0};
  wifiClient.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
  }

  wifiClient.stop(); //close the connection to the server
  
}

void pushReadTemp(){
  //for first we need to serialize all the data into a json string
  //data serialized into the json will be
  //configuration, containing desiredTemp and enabled
  //status, containing readTemp and relayOpened

  //serialize the config variable into a json
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);

  doc["readTemp"] = readTemp;
  doc["readHumidity"] = readTemp;
  doc["tmsRead"] = (currentTime + millis());

  String jsonSerialized = "";
  serializeJson(doc, jsonSerialized);
  Serial.println("Json Serialized : ");
  Serial.println(jsonSerialized);

  //connection
  wifiClient.setTimeout(10000);//set timeout for the connection

  if (!wifiClient.connect(server, 80)) { //connect to the remote host
    Serial.println(F("Connection failed"));
    return;//if we wasn't able to connect to the server we will return
  }
  Serial.println(F("Connected!"));

  //create the header
  char head[200]; 
  strcat( head, "GET /temperature?token=");
  strcat( head, apiToken);
  strcat( head, " HTTP/1.1");
  
  // Send HTTP request to the remote host
  wifiClient.println(head);
  wifiClient.print("Host: ");
  wifiClient.println(server);
  wifiClient.println(F("Connection: close"));
  wifiClient.println("Content-Type: application/json;");
  wifiClient.print("Content-Length: ");
  wifiClient.println(jsonSerialized.length());
  wifiClient.println();
  wifiClient.println(jsonSerialized);
  
  
  if (wifiClient.println() == 0) {
    Serial.println(F("Failed to send request"));
  }

  // Check HTTP status
  char status[32] = {0};
  wifiClient.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
  }

  wifiClient.stop(); //close the connection to the server
}
