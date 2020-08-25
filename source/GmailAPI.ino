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

// #################################################################################################
// Gmail send multipart mail - join one attachement
boolean sendMail(char* from, char* email_list, char* subject, char* message, boolean attachement) {
  boolean result = false;
  String boundary = "foo_bar_baz";
  long timeout = 20000;
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    String bodyMessage =  messageBody((String)from, (String)email_list, (String)subject, (String)message, boundary);
    String bodyAttachement =  attachementBody(boundary);
    String bodyEnd = String("--") + boundary + String("--\r\n");

    SD.begin();
    File file = SD.open("/picture.txt");
    if (!file) {
      Serial.println(F("Failed to read file"));
    }
    //    Serial.print(F("Size picture.txt : "));
    //    Serial.print(file.size());
    //    Serial.println(F(" octets"));

    // Compute Content-Length
    long totalLength = bodyMessage.length() + bodyAttachement.length() + file.size() + bodyEnd.length();

    String headerTxt =  header((String)from, (String)totalLength);

    WiFiClientSecure client;
    if (!client.connect("www.googleapis.com", 443))
    {
      return ("Connection failed");
    }
    //Serial.println(headerTxt + bodyMessage + bodyAttachement + bodyEnd);

    client.print(headerTxt + bodyMessage + bodyAttachement);
    // send file contents to Gmail
    // upload the file in 1350 byte chunks
    while (file.available()) {
      int nextPacketSize = file.available();
      if (nextPacketSize > 1350) {
        nextPacketSize = 1350;
      }
      String buffer = "";
      for (int i = 0; i < nextPacketSize; i++) {
        buffer += (char)file.read();
      }
      client.print(buffer);
    }
    client.print("\r\n" + bodyEnd);

    file.close();
    SD.end();
    delay(20);

    // Handle the Http response
    long tOut = millis() + timeout;
    while (client.connected() && tOut > millis())
    {
      if (client.available())
      {
        String response = client.readStringUntil('\r');
        if (response.endsWith("200 OK")) {
          Serial.println("Sending mail with attachment successfully : " + response);
          break;
        } else {
          Serial.println("Somethings wrong with Gmail API: " + response);
          break;
        }
      }
    }
    client.flush();
    client.stop();
    return true;
  }
}
// Compose header
String header(String from, String length)
{
  String  data;
  data =  F("POST /upload/gmail/v1/users/");
  data += from;
  data += F("/messages/send?uploadType=multipart HTTP/1.1\r\n");
  data += F("Host: www.googleapis.com\r\n");
  data += F("content-length: ");
  data += String(length);
  data += F("\r\n");
  data += F("Content-Type: message/rfc822\r\n");
  data += F("Authorization: ");
  data += bearer;
  data += "\r\n";
  data += "\r\n";
  return (data);
}

// Compose Body Attachement
String attachementBody(String boundary)
{
  String data;
  data = "\r\n--";
  data += boundary;
  data += F("\r\n");
  data += F("Content-Type: image/png\r\n");
  data += F("MIME-Version: 1.0\r\n");
  data += F("Content-Transfer-Encoding: base64\r\n");
  data += F("Content-Disposition: attachment; filename=\"picture.png\"\r\n");
  data += F("\r\n");
  return (data);
}

// Compose Body Message
String messageBody(String from, String to, String subject, String message, String boundary) {
  String data;

  data += F("Content-Type: multipart/mixed; boundary=\"");
  data += boundary;
  data += F("\"\r\n");
  data += F("from:");
  data += from;
  data += F("\r\n");
  data += F("to:");
  data += to;
  data += F("\r\n");
  data += F("subject:");
  data += subject;
  data += F("\r\n");
  data += F("\r\n--");
  data += boundary;
  data += "\r\n";
  data += "\r\n";
  data += message;
  data += "\r\n";

  return (data);
}
