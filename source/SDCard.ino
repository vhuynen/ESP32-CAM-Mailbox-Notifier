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
  wifi_ip_static = doc.containsKey("ip_static"); // Send email if email root attribut exist
  strcpy(wifi_ip, doc["wifi"]["ip_static"].as<char*>()); // "IP"
  strcpy(wifi_gateway, doc["wifi"]["gateway"].as<char*>()); // "Gateway"
  strcpy(wifi_subnet, doc["wifi"]["subnet"].as<char*>()); // "Subnet"
  strcpy(wifi_dns_ip_primary, doc["wifi"]["dns_ip_primary"].as<char*>()); // "Subnet"
  strcpy(wifi_dns_ip_secondary, doc["wifi"]["dns_ip_secondary"].as<char*>()); // "Subnet"  
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
  strcpy(url, doc["sms"]["url"].as<char*>()); // urls : List of sms API delimited with comma separator: ,
  strcpy(sms_body_door, doc["sms"]["body_door"].as<char*>()); // sms for great door
  strcpy(sms_body_flip_door, doc["sms"]["body_flip_door"].as<char*>()); // sms for flip door

  // Init control properties
  retry =  doc["control"]["retry"]; // Number of retry when the door is already open
  overtime_open_door = doc["control"]["overtime_open_door"]; // Timeout when one of door is open so long 
    
  //printFile(properties);
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
