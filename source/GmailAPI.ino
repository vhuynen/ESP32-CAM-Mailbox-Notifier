// Retrieved Access Token for GMAIL API - scope OAuth https://www.googleapis.com/auth/gmail.send
boolean retrieved_access_token_gmail(char* gmail_credentials_refresh_token, char* gmail_credentials_client_id, char* gmail_credentials_client_secret) {
  boolean result = false;
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    result = true;
    // Compose the request for retrieved the Access Token from the Refresh Token
    String request = "{\"grant_type\":\"refresh_token\",\"refresh_token\":\"" + (String)gmail_credentials_refresh_token +
                     "\",\"client_id\":\"" + (String)gmail_credentials_client_id + "\",\"client_secret\":\"" + (String) gmail_credentials_client_secret + "\"}";

    HTTPClient http;
    http.setTimeout(10000);
    //Retreived Access Token from Refresh Token
    http.begin("https://www.googleapis.com/oauth2/v4/token");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(request);
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        const size_t capacity = JSON_OBJECT_SIZE(4) + 327;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, http.getString());
        //serializeJsonPretty(doc, Serial);
        // Access Token
        const char* access_token = doc["access_token"];
        // Set Authorization Bearer
        bearer = "Bearer " + String(access_token);
        http.end(); //Free the resources
        Serial.printf("Access Token retreived : %s\n", access_token);
      } else {
        result = false;
        http.end(); //Free the resources
        Serial.println("Access Token not retreived  !!!");
      }
    } else {
      result = false;
      http.end(); //Free the resources
      Serial.println("Access Token not retreived  !!!");
    }
  }
  return result;
}

// Send Mail
boolean sendMail(char* from, char* email_list, char* subject, char* body) {
  boolean result = false;
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    // Send Mail
    HTTPClient http;
    // Extract all recepients from a copy of email_list propertie
    char* email_list_cpy = (char *) malloc(100);
    strcpy(email_list_cpy, email_list);
    char * email_tokenizer = strtok(email_list_cpy, ",");
    String recipients = "";
    // loop through the string to extract all other addresses
    while ( email_tokenizer != NULL ) {
      //printf( " %s\n", email_tokenizer ); //printing each email address
      recipients = recipients + "to:" + (String)email_tokenizer + "\n";
      email_tokenizer = strtok(NULL, ",");
    }
    Serial.println("Recipients : " + (String)recipients);
    // Compose the mail's body
    String request = "from:" + (String)from + "\n"
                     + (String)recipients
                     + "subject:" + "[" + String(wake_count) + "]" + (String)subject + "\n"
                     + "\n" + (String)body + "\n";

    // "https://www.googleapis.com/upload/gmail/v1/users/:userId/messages/send?uploadType=media
    String url = "https://www.googleapis.com/upload/gmail/v1/users/" + (String)from + "/messages/send?uploadType=media";
    http.setTimeout(10000);
    //Send mail
    http.begin(url);
    http.addHeader("Content-Type", "message/rfc822");
    // Set Authorization Bearer
    http.addHeader("Authorization", bearer);
    int httpCode = http.POST(request);
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        http.end(); //Free the resources
        result = true;
      } else {
        result = false;
        http.end(); //Free the resources
        Serial.println("Mail not sending  !!!");
      }
    } else {
      result = false;
      http.end(); //Free the resources
      Serial.println("Mail not sending  !!!");
    }
  }
  return result;
}
