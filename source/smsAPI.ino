// Send SMS
boolean sendSMS(char* urls, char* msg) {
  boolean result = false;
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    HTTPClient http;
    result = true;
    // Make a copy of the original urls propertie
    char* urlcpy = (char *) malloc(200);
    strcpy(urlcpy, urls);
    // Extract all urls
    char* url_tokenizer = strtok(urlcpy, ",");
    // loop through the string to extract all other urls
    while ( url_tokenizer != NULL ) {
      printf( "Free url API : %s\n", url_tokenizer ); //printing each url API
      Serial.println(urlencode((String)msg));
      // Call free SMS API from "FREE" provider : https://smsapi.free-mobile.fr/sendmsg
      http.begin((String)url_tokenizer + urlencode("[" + String(wake_count) + "]" + (String)msg));
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          http.end(); //Free the resources
          url_tokenizer = strtok(NULL, ",");
          Serial.println("SMS sended !");
        } else {
          result = false;
          http.end(); //Free the resources
          Serial.println("SMS not sended !!!");
        }
      } else {
        result = false;
        http.end(); //Free the resources
        Serial.println("SMS not sended !!!");
      }
    }
    return result;
  }
  return result;
}
