/********************************************************
ESP8266 (ESP-12E) IoT weather module unit
---------------------------------------------------------
(0x76) BME280  - temp,humi,pressure
(0x40) HTU21D  - temp,humi
********************************************************/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <SparkFunHTU21D.h>
/******************** ADC MACRO FOR VCC MESSUREMENT ******/
ADC_MODE(ADC_VCC);
/******************** DEFUALT CONFIGURATION **************/
const int UNIT_NO = 1;
const char* MQTT_SERVER = "kubik256.uk.to";
const char* MQTT_USER = "root";
const char* MQTT_PASS = "mqtt_root";
const int MQTT_PORT = 1883;
const int ESP_SLEEP = 300; /* SLEEP TIME (s) */
/******************** MAIN VARIABLES *********************/
WiFiClient espClient;
PubSubClient client(MQTT_SERVER,MQTT_PORT,espClient);
HTU21D htu;
float t,h,b;
char buff [12]; 
/******************** SETUP ON BOOT **********************/
void setup(){
  //Serial.begin(9600);
  /* WIFI MANAGER PART */
  WiFiManager wifiManager;
  if(!wifiManager.autoConnect()){
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
  /* START SENSORS */
  htu.begin();
  /* START MQTT CLIENT */
  sprintf(buff, "unit%d", UNIT_NO);
  client.connect(buff,MQTT_USER,MQTT_PASS);
  delay(100);
}
/******************** MAIN LOOP **************************/
void loop(){
  /* HTU21D */
  h = htu.readHumidity();
  t = htu.readTemperature();
  /* VOLTAGE */
  b = ESP.getVcc() / 1000.0;
  /* PUBLISH ALL VALUES AS JSON PAYLOAD VIA MQTT */
  char buff2 [256];
  DynamicJsonBuffer jsonBuffer;
  sprintf(buff, "iot/unit%d", UNIT_NO);
  JsonObject& root = jsonBuffer.createObject();
  root["t"] = t; root["h"] = h; root["b"] = b;
  root.printTo(buff2, sizeof(buff2));
  client.publish(buff,buff2,true);
  /* DEEP SLEEEP */
  ESP.deepSleep(ESP_SLEEP * 1000000);
}
