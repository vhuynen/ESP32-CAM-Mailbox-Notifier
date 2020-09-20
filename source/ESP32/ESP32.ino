//###################### Vincent HUYNEN ########################/
//################## vincent.huynen@gmail.com ##################/
//######################### AUGUST 2020 ########################/
//######################## Version 1.0.0 #######################/

/*
  Mailbox Notifier
*/

// Import external base64 library C
extern "C" {
#include "crypto/base64.h"
}

#include <WiFi.h>
#include <HTTPClient.h>
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <esp_bt.h>
#include <Update.h>
#include "driver/rtc_io.h"
#include "driver/adc.h"

// Variables persisted into RTC memory
RTC_DATA_ATTR int wake_count = 0;

// Filename properties
char* filename_properties = "/mailbox.cfg";

// Path for the Base64 temporary file
String pathFileBase64 = "/tmp/encode.b64";

// WiFi Properties
boolean wifi_ip_static = false;
char* wifi_ip = (char *) malloc(20);
char* wifi_gateway = (char *) malloc(20);
char* wifi_subnet = (char *) malloc(20);
char* wifi_dns_ip_primary = (char *) malloc(20);
char* wifi_dns_ip_secondary = (char *) malloc(20);
char* wifi_ssid = (char *) malloc(50);
char* wifi_security_code = (char *) malloc(50);

// OAuth Credentials for Gmail API
String bearer; // Access Token retreived from OAuth Google API
char* gmail_credentials_refresh_token = (char *) malloc(150);
char* gmail_credentials_client_id  = (char *) malloc(100);
char* gmail_credentials_client_secret  = (char *) malloc(50);

// Mail propeties
boolean email = false;
char* email_from  = (char *) malloc(50);
char* email_to  = (char *) malloc(100);
char* email_subject_door  = (char *) malloc(100);
char* email_body_door = (char *) malloc(150);
char* email_subject_flip_door = (char *) malloc(100);
char* email_body_flip_door = (char *) malloc(150);

//SMS properties
boolean sms = false;
char* url  = (char *) malloc(200);
char* sms_body_door  = (char *) malloc(100);
char* sms_body_flip_door  = (char *) malloc(100);

// Control properties
long retry = 1;
long overtime_open_door = 10000;

// Pin which handle the wake up
int pinFlipDoor = 14;
int pinDoor = 13;
int pinWakeUp = 0;
//  Bit mask of GPIO numbers which will cause wakeup - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
#define BUTTON_PIN_BITMASK 0x6000 // 2^14 + 2^13 in hex

void setup()
{

  Serial.begin(9600);

  // Only for the bootstrap, update the firmware from SD Card if a new one exists
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  if (wake_count == 0) {
    if (updateFirmware()) {
      Serial.println("The firmware has been updated...");
      digitalWrite(BUILTIN_LED, HIGH);
      delay(3000);
      Serial.println("ESP32 Rebooting...");
      ESP.restart();
    }
  }

  // Init pin wake up
  pinMode(pinFlipDoor, INPUT);
  pinMode(pinDoor, INPUT);
  // Activate de pulldown internal resistor
  gpio_pulldown_en(GPIO_NUM_14);
  gpio_pulldown_en(GPIO_NUM_13);

  // Init properties from SD Card
  initProperties(filename_properties);
  // Init the pin which occur the wake up of the nodeMCU
  setGPIOWakeUp();

  //  // Starting the WiFi
  //  connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
  //    if (retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret)) {
  //      Serial.println("Access Token has been retreived...");
  //    }
  //
  //
  // Send mail with attachemet by Gmail API
  //    if (sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door, "/tmp/mailbox.png")) {
  //      Serial.println("Send e-mail with attachement OK...");
  //    }

  //  Serial.println("Sending mail...");
  //  if (retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret)) {
  //    Serial.println("Access Token has been retreived...");
  //  }
  //  if (sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door)) {
  //    Serial.println("Mail are going...");
  //  }

  //  if (sendMail(email_from, email_to, email_subject_door, email_body_door)) {
  //    Serial.println("Mail are going...");
  //  }

  //  if (sendSMS(url, sms_body_door)) {
  //    Serial.println("SMS sms_body_door have been sended...");
  //  }
  //    if (sendSMS(url, sms_body_flip_door)) {
  //      Serial.println("SMS sms_body_flip_door have been sended...");
  //    }


  printAllProperties();
  wake_count++;
  Serial.println("WakeUp Pin" + (String)pinWakeUp);
}

void loop()
{

  // When you turn on the controller a first time
  if (wake_count == 1) {
    if (digitalRead(pinDoor) == 0  && digitalRead(pinFlipDoor) == 0) {
      // Only if all doors are closed
      goToDeepSleep();
    } else if (millis() > overtime_open_door) {
      // Connect to WiFI
      connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
      // Retreived Access Token
      retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret);
      // Send error mail by Gmail API
      sendMail(email_from, email_to, (char *) "[MAILBOX] BOOT ERROR", (char *) "Doors have been opened too long time when you have turned on the ESP32.\nReboot the ESP32 for reinitialize it !");
      // Going to sleep until reset or restart
      goToDeepSleepError();
    }
  }

  // Ignore wake up when you fetch your mail, open the flip door first before open the principal door
  if (pinWakeUp == pinFlipDoor) {
    if (digitalRead(pinDoor) == 1) {
      while (digitalRead(pinDoor) == 1  || digitalRead(pinFlipDoor) == 1)  {
        if (millis() > overtime_open_door) {
          // Connect to WiFI
          connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
          // Retreived Access Token
          retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret);
          // Send error mail by Gmail API
          sendMail(email_from, email_to, (char *) "[MAILBOX] FETCH MAIL ERROR", (char *) "Doors have been opened too long time when you have fetched your mail.\nReboot the ESP32 for reinitialize it !");
          // Going to sleep until reset or restart
          goToDeepSleepError();
        }
      }
      // Everythings is OK, you can sleep quietly
      goToDeepSleep();
    }
  }

  // You have got mail ! Let's go...
  if (pinWakeUp == pinDoor || pinWakeUp == pinFlipDoor) {
    if (digitalRead(pinDoor) == 0  && digitalRead(pinFlipDoor) == 0) {
      // Connect to WiFI
      connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
      // Retreived Access Token
      retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret);

      if (pinWakeUp == pinDoor) {
        sendMail(email_from, email_to, email_subject_door, email_body_door);
      }
      if (pinWakeUp == pinFlipDoor) {
        sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door);
      }
      // Everythings is OK, you can sleep quietly
      goToDeepSleep();
    }
    if (millis() > overtime_open_door) {
      // Connect to WiFI
      connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
      // Retreived Access Token
      retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret);
      // Send error mail by Gmail API
      sendMail(email_from, email_to, (char *) "[MAILBOX] POSTMAN ERROR", (char *) "Doors have been opened too long time by the postman.\nReboot the ESP32 for reinitialize it !");
      // Going to sleep until reset or restart
      goToDeepSleepError();
      // Something wring, going to sleep until reset or restart
      goToDeepSleepError();
    }
  }
}
