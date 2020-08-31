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


void setup()
{

  Serial.begin(9600);

  // Only for the bootstrap, update de firmware from SD Card.
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  if (wake_count == 0) {
    if(updateFirmware()) {
      Serial.println("The firmware has been updated...");
      digitalWrite(BUILTIN_LED, HIGH);
      delay(3000);
      Serial.println("ESP32 Rebooting...");
      ESP.restart();      
    }
  }
    // Init properties from SD Card
    initProperties(filename_properties);

//  // Starting the WiFi
//  connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
//  if (retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret)) {
//    Serial.println("Access Token has been retreived...");
//  }
//
//
//  // Send mail with attachemet by Gmail API
//  if (sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door, "/tmp/mailbox.png")) {
//    Serial.println("Send e-mail with attachement OK...");
//  }

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
  delay(5000);
  goToDeepSleep();

}

void loop()
{

}
