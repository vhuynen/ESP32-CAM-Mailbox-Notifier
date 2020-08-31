/**
   Function to deserialize file from SD
   DynamicJsonDocument doc(1024);
   JsonObject obj;
   obj = getJSonFromFile(&amp;doc, filename);
*/
JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true ) {

  // open the file for reading:
  File myFileSDCart = SD.open(filename);
  Serial.println("read file " + filename);
  if (myFileSDCart) {
    // read from the file until there's nothing else in it:
    //          if (myFileSDCart.available()) {
    //              firstWrite = false;
    //          }

    DeserializationError error = deserializeJson(*doc, myFileSDCart);
    if (error) {
      // if the file didn't open, print an error:
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

      if (forceCleanONJsonError) {
        return doc->to<JsonObject>();
      }
    }

    // close the file:
    myFileSDCart.close();
    return doc->as<JsonObject>();
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(filename);
    Serial.println(F("Empty json created"));
    return doc->to<JsonObject>();
  }
}

void initProperties(char* filename_properties) {

  delay(2000);
  // Initialize SD library
  while (!SD.begin()) {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }
  Serial.println(F("SD library initialized"));
  DynamicJsonDocument doc(2048);
  JsonObject obj;
  obj = getJSonFromFile(&doc, filename_properties);

  // Init WiFi Properties
  wifi_ip_static = doc["wifi"].containsKey("ip_static"); // Init IP Settings if ip_static attribut exist
  if (wifi_ip_static == true) {
    Serial.println("Chargement des Setting IP");
    strcpy(wifi_ip, doc["wifi"]["ip_static"].as<char*>()); // "IP"
    strcpy(wifi_gateway, doc["wifi"]["gateway"].as<char*>()); // "Gateway"
    strcpy(wifi_subnet, doc["wifi"]["subnet"].as<char*>()); // "Subnet"
    strcpy(wifi_dns_ip_primary, doc["wifi"]["dns_ip_primary"].as<char*>()); // "Subnet"
    strcpy(wifi_dns_ip_secondary, doc["wifi"]["dns_ip_secondary"].as<char*>()); // "Subnet"
    Serial.println("Fin Chargement des Setting IP");
  }
  strcpy(wifi_ssid, doc["wifi"]["ssid"].as<char*>()); // "SSID"
  strcpy(wifi_security_code, doc["wifi"]["security_code"].as<char*>()); // "Passphrase"

  // Init OAuth Gmail Credentials
  strcpy(gmail_credentials_refresh_token, doc["gmail_credentials"]["refresh_token"].as<char*>()); // "OAuth offline Refresh Token"
  strcpy(gmail_credentials_client_id, doc["gmail_credentials"]["client_id"].as<char*>()); // "OAuth ClientID"
  strcpy(gmail_credentials_client_secret, doc["gmail_credentials"]["client_secret"].as<char*>()); // "OAuth Client Secret"

  // Init Mail properties
  email = doc.containsKey("email"); // Send email if email root attribut exist
  strcpy(email_from, doc["email"]["from"].as<char*>()); //" From : your e-mail address
  strcpy(email_to, doc["email"]["to"].as<char*>()); // To : List of recipients delimited with comma separator: ,
  strcpy(email_subject_door, doc["email"]["subject_door"].as<char*>()); // Mail subject for great door
  strcpy(email_body_door, doc["email"]["body_door"].as<char*>()); // Mail body for great door
  strcpy(email_subject_flip_door, doc["email"]["subject_flip_door"].as<char*>()); // Mail subject for flip door
  strcpy(email_body_flip_door, doc["email"]["body_flip_door"].as<char*>()); // Mail body for flip door

  //Init sms properties
  sms = doc.containsKey("sms"); // Send sms if email root attribut exist
  if (sms) {
    strcpy(url, doc["sms"]["url"].as<char*>()); // urls : List of sms API delimited with comma separator: ,
    strcpy(sms_body_door, doc["sms"]["body_door"].as<char*>()); // sms for great door
    strcpy(sms_body_flip_door, doc["sms"]["body_flip_door"].as<char*>()); // sms for flip door
  }

  // Init control properties
  retry =  doc["control"]["retry"]; // Number of retry when the door is already open
  overtime_open_door = doc["control"]["overtime_open_door"]; // Timeout when one of door is open so long

  //printFile(filename_properties);
  SD.end();
}

// Prints the content of a file to the Serial
void printFile(const char *properties) {
  // Open file for reading
  File file = SD.open(properties);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char) file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

// Print all properties loaded by the initProperties function
void printAllProperties() {

  Serial.println("WiFi IP Static : " + (String)wifi_ip_static);
  if (wifi_ip_static) {
    Serial.println("WiFi IP Static  : " + String(wifi_ip));
    Serial.println("WiFi Gateway : " + String(wifi_gateway));
    Serial.println("WiFi Subnet  : " + String(wifi_subnet));
    Serial.println("WiFi DNS IP Primary : " + String(wifi_dns_ip_primary));
    Serial.println("WiFi DNS IP Secondary  : " + String(wifi_dns_ip_secondary));
  }
  Serial.println("WiFi SSID : " + String(wifi_ssid));
  Serial.println("WiFi Passphrase  : " + String(wifi_security_code));

  Serial.println("Refresh Token : " + String(gmail_credentials_refresh_token));
  Serial.println("Client ID : " + String(gmail_credentials_client_id));
  Serial.println("Client Secret : " + String(gmail_credentials_client_secret));

  Serial.println("Email : " + String(email));
  Serial.println("From : " + String(email_from));
  Serial.println("To : " + String(email_to));
  Serial.println("Subject door : " + String(email_subject_door));
  Serial.println("Body door : " + String(email_body_door));
  Serial.println("Subject flip door : " + String(email_subject_flip_door));
  Serial.println("Body flip door : " + String(email_body_flip_door));
  Serial.println("Retry : " + String(retry));

  if (sms == true) {
    Serial.println("sms : " + String(sms));
    Serial.println("url : " + String(url));
    Serial.println("sms great door : " + String(sms_body_door));
    Serial.println("sms flip door : " + String(sms_body_flip_door));
  }

  Serial.println("Retry : " + String(retry));
  Serial.println("Timeout overtime open door : " + String(overtime_open_door));
}
