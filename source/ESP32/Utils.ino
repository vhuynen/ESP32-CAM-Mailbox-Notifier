// How many minutes the ESP should sleep in minutes
const long deep_sleep_time = 1;

// Deep Sleep when everythings is right
void goToDeepSleep()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  adc_power_off();
  esp_bt_controller_disable();

  // Prepare settings before going to sleep
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  gpio_pulldown_en(GPIO_NUM_13);
  gpio_pulldown_en(GPIO_NUM_14);
  delay(3000);
  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

// Deep Sleep when something's wrong
void goToDeepSleepError()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  adc_power_off();
  esp_bt_controller_disable();

  delay(3000);
  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

// Compose IP Address from properties
IPAddress getIPAddressFromString(char* ipStr) {
  int part_1;
  int part_2;
  int part_3;
  int part_4;

  char* ip_cpy = (char *) malloc(15);
  strcpy(ip_cpy, ipStr);
  char * ip_tokenizer = strtok(ip_cpy, ".");
  int count = 1;
  while ( ip_tokenizer != NULL ) {
    if (count == 1) {
      part_1 = atoi(ip_tokenizer);
    }
    if (count == 2) {
      part_2 = atoi(ip_tokenizer);
    }
    if (count == 3) {
      part_3 = atoi(ip_tokenizer);
    }
    if (count == 4) {
      part_4 = atoi(ip_tokenizer);
    }
    ip_tokenizer = strtok(NULL, ".");
    count++;
  }
  return IPAddress(part_1, part_2, part_3, part_4);
}

void connectToWiFi(char* wifi_ssid, char* wifi_security_code, boolean wifi_ip_static, char* wifi_ip, char* wifi_gateway, char* wifi_subnet, char* wifi_dns_ip_primary, char* wifi_dns_ip_secondary) {
  unsigned long wifi_timeout = 10000; // 10 seconds in milliseconds
  Serial.print("Connecting to WiFi... ");
  if (wifi_ip_static) {
    IPAddress ip = getIPAddressFromString(wifi_ip);
    IPAddress gateway = getIPAddressFromString(wifi_gateway);
    IPAddress subnet = getIPAddressFromString(wifi_subnet);
    IPAddress primaryDNS = getIPAddressFromString(wifi_dns_ip_primary); //optional
    IPAddress secondaryDNS = getIPAddressFromString(wifi_dns_ip_secondary); //optional

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

// #############################################
// Encode content file in Base64
// #############################################
void encodeFile(String filePath) {

  SD.begin();
  // Open the file to be encoded
  File fileIn = SD.open(filePath);
  File fileOut;
  SD.remove(pathFileBase64);
  fileOut = SD.open(pathFileBase64, FILE_APPEND);

  // upload the file in 1350 byte chunks
  while (fileIn.available()) {
    int nextPacketSize = fileIn.available();
    if (nextPacketSize > 1350) {
      nextPacketSize = 1350;
    }
    String toEncode = "";
    for (int i = 0; i < nextPacketSize; i++) {
      toEncode += (char)fileIn.read();
    }
    size_t outputLength;
    unsigned char * encoded = base64_encode((const unsigned char *)toEncode.c_str(), nextPacketSize, &outputLength);
    // Need to open and close file each time to flush correctly the buffer. The function file.flush() doesn't work in my case.
    fileOut = SD.open(pathFileBase64, FILE_APPEND);
    fileOut.print((char *)encoded);
    fileOut.close();
    free(encoded);
  }
  fileIn.close();
  fileOut.close();
  SD.end();
}

// Update the firmware from SD Card
boolean updateFirmware() {
  boolean result = false;
  SD.begin();
  if (SD.exists("/firmware/mailbox.bin")) {
    File firmware = SD.open("/firmware/mailbox.bin");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
      Update.printError(Serial);
      return result;
    }
    while (firmware.available()) {
      uint8_t buffer[128];
      firmware.read((uint8_t *)buffer, 128);
      Update.write(buffer, sizeof(buffer));
    }
    if (Update.end(true)) {
      Serial.printf("Update Success: %u Ko\n", firmware.size());
      result = true;
    } else {
      Update.printError(Serial);
    }
  }
  SD.remove("/firmware/mailbox.bin");
  SD.end();
  return result;
}

// Set the pin which woke up the controller
void setGPIOWakeUp() {
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason)) / log(2), 0);
  pinWakeUp = (log(GPIO_reason)) / log(2);
}
