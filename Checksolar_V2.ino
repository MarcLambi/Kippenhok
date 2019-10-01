/*
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
*/
/*sparkfun thing/*
Check solar

Author:
Marc Lambrechts Marc.lambrechts@skynet.be
*/

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int Dag_Nacht = 1;
RTC_DATA_ATTR int Dag_Start = 1000;
RTC_DATA_ATTR int Nacht_Start = 30; 

const int Dag = 1;
const int Nacht = 0;
const int motorPinRe = 16; // the number of the switch pin
const int motorPinLi = 17; // the number of the motor pin
const int motorPin = 2;
const int poortOpenPin = 13;
const int poortToePin = 12;
const int InitValPin = 14;
const int solarPin = 4; //
const int motorDelay = 5;
const int motorMaxCycles = 2000;
/*
 * Wifi Setup

const char* ssid = "Kippenhotel";
const char* password = "Tobias2804";
*/
/*
Method to print the reason by which ESP32
has been awaken from sleep
*/

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void openPoort()
{
  digitalWrite(motorPinRe, LOW);
  digitalWrite(motorPinLi, HIGH);

  int motorCycles = motorMaxCycles;
 
// turn motor on until sensor Poort open:

  while(digitalRead(poortOpenPin)== HIGH)
  { 
    if (motorCycles --)
    {
      digitalWrite(motorPin, HIGH);
//    Serial.print(">");
      delay(motorDelay);
    } else {Serial.println ("OpenPoort: To Many cycles - break"); break;}
  }  
  digitalWrite(motorPin, LOW);
  Serial.println("Poort Open");
}

void sluitPoort()
{
  int motorCycles = motorMaxCycles;

  digitalWrite(motorPinRe, HIGH);
  digitalWrite(motorPinLi, LOW);
  
  while(digitalRead(poortToePin)== HIGH)
  { 
    if (motorCycles --)
    {
      digitalWrite(motorPin,   HIGH);
//    Serial.print("<");
      delay(motorDelay);
    } else {Serial.println ("Sluitpoort: To Many cycles - break"); break;}
  }
  digitalWrite(motorPin,   LOW);
  Serial.println("Poort toe");
}

int checkDagNacht()
{
   int val;
   
   pinMode(solarPin, INPUT); 
   
   val = analogRead(solarPin);
   
   Serial.print("Solar Value :");
   Serial.println(val);

   if (Dag_Nacht == Dag) { // Dag regime
     Serial.println("DAG");
     if (val <= Nacht_Start) {
       Dag_Nacht = Nacht;
       Serial.println("Switch van Dag naar Nacht");
       sluitPoort();  
     } // else openPoort();
   } 
   else { // Nacht regime
     Serial.println("nacht");  
     if (val >= Dag_Start) {
          Dag_Nacht = Dag;
          Serial.println("Switch van Nacht naar dag");
          openPoort();
       } //else sluitPoort();
   }
}

void setup(){
  pinMode(poortOpenPin, INPUT_PULLUP);
  pinMode(poortToePin,  INPUT_PULLUP);
  
  pinMode(motorPinRe, OUTPUT);
  pinMode(motorPinLi, OUTPUT);
  pinMode(motorPin, OUTPUT);
  
  Serial.begin(115200);

  delay(100); //Take some time to open up the Serial Monitor
  
//  Print the wakeup reason for ESP32
//  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
/*  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
*/

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
//  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
//  Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  checkDagNacht();
  
  esp_deep_sleep_start();
  
/*  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.println("Connected!");
  }  
  else  
  {
     Serial.println("Not Connected!");
     delay (100); 

     esp_deep_sleep_start();
  }
   
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
      
      ESP.restart();
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
*/
}

void loop() {

//  ArduinoOTA.handle();

}
