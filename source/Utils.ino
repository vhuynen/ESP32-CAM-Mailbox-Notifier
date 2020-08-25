// How many minutes the ESP should sleep in minutes
const long deep_sleep_time = 1440;

void goToDeepSleep()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  //adc_power_off();
  //  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(deep_sleep_time * 60L * 1000000L);

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

void connectToWiFi(char* wifi_ssid, char* wifi_security_code, boolean wifi_ip_static, char* wifi_ip, char* wifi_gateway, char* wifi_subnet, char* wifi_dns_ip_primary, char* wifi_dns_ip_secondary) {
  unsigned long wifi_timeout = 10000; // 10 seconds in milliseconds
  Serial.print("Connecting to WiFi... ");
  if (wifi_ip_static) {
    //    IPAddress ip = ip.fromString(wifi_ip);
    //    IPAddress gateway = gateway.fromString(wifi_gateway);
    //    IPAddress subnet = subnet.fromString(wifi_subnet);
    //    IPAddress primaryDNS = primaryDNS.fromString(wifi_dns_ip_primary); //optional
    //    IPAddress secondaryDNS = secondaryDNS.fromString(wifi_dns_ip_secondary); //optional

    IPAddress ip(192, 168, 5, 141);
    IPAddress gateway(192, 168, 5, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(192, 168, 5, 1); //optional
    IPAddress secondaryDNS(192, 168, 5, 1); //optional
    if (!WiFi.config(ip, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_security_code);

  // Keep track of when we started our attempt to get a WiFi connection
  unsigned long startAttemptTime = millis();

  // Keep looping while we're not connected AND haven't reached the timeout
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifi_timeout) {
    delay(10);
  }

  // Make sure that we're actually connected, otherwise go to deep sleep
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("FAILED");
    goToDeepSleep();
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
  Serial.println("Connected !");
}

String urldecode(String str)
{

  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == '+') {
      encodedString += ' ';
    } else if (c == '%') {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    } else {

      encodedString += c;
    }

    yield();
  }

  return encodedString;
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;

}

unsigned char h2int(char c)
{
  if (c >= '0' && c <= '9') {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

// #################################################################################################
// Temporary function for test sending picture
int base64_enc_len(int plainLen) {
  int n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

void encodeTest() {

  SD.begin();
  // open the file for reading:
  File file = SD.open("/picture.png");
  const char* toEncode;
  size_t outputLength;
  
  while (file.available()) {
    toEncode = file.readString().c_str();
  }

  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  Serial.println("Longeur chaine à encoder :" + static_cast<int>(strlen(toEncode)));
  Serial.printf("%.*s", 10000, toEncode);
  unsigned char * encoded = base64_encode((const unsigned char *)toEncode, strlen(toEncode), &outputLength);
 
  Serial.print("Length of encoded message: ");
  Serial.println(outputLength);
 
  Serial.printf("%.*s", outputLength, encoded);
  free(encoded);
}
