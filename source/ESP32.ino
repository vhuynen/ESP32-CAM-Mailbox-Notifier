//###################### Vincent HUYNEN ########################/
//################## vincent.huynen@gmail.com ##################/
//######################### AUGUST 2020 ########################/
//######################## Version 1.0.0 #######################/

/*
  Mailbox Notifier
*/

// Import external base64 API C
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

// Variables persisted into RTC memory
RTC_DATA_ATTR boolean starting = true;
RTC_DATA_ATTR int wake_count = 0;

// Filename properties
char* filename_properties = "/mailbox.cfg";

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

// Relay that control the power of the SD card
const int relay = 26;

void setup()
{

  // Initialize relay pin
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  Serial.begin(9600);
  Serial.println("starting init : " + (String)starting);
  // Only for the first boot, initialization of all properties from SD Card
  if (starting) {
    Serial.println("initialization of properties");
    initProperties(filename_properties);
  }

  // Starting the WiFi
  connectToWiFi(wifi_ssid, wifi_security_code, wifi_ip_static, wifi_ip , wifi_gateway, wifi_subnet, wifi_dns_ip_primary, wifi_dns_ip_secondary);
  if (retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret)) {
    Serial.println("Access Token has been retreived...");
  }
  //Temporary test for Base64 encoding
  if (sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door, true)) {
    Serial.println("Send e-mail with attachement OK...");
  }

  //  Serial.println("Sending mail...");
  //  Serial.println("starting status: " + (String)starting);
  //  if (retrieved_access_token_gmail(gmail_credentials_refresh_token, gmail_credentials_client_id, gmail_credentials_client_secret)) {
  //    Serial.println("Access Token has been retreived...");
  //  }
  //  if (sendMail(email_from, email_to, email_subject_flip_door, email_body_flip_door)) {
  //    Serial.println("Mail are going...");
  //  }

  //  if (sendSMS(url, sms_body_door)) {
  //    Serial.println("SMS sms_body_door have been sended...");
  //  }
  //    if (sendSMS(url, sms_body_flip_door)) {
  //      Serial.println("SMS sms_body_flip_door have been sended...");
  //    }

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

  Serial.println("Access Token : " + String(gmail_credentials_refresh_token));
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


  Serial.println("Timeout overtime open door : " + String(overtime_open_door));
  Serial.println("Starting  : " + String(starting));
  wake_count++;
  delay(5000);
  goToDeepSleep();

}

void loop()
{

}
