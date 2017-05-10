/********************************************************
ESP8266 (ESP-12E) IoT weather station with:
-------------------------------------------
(0x76) BME280  - temp,humi,pressure
(0x60) SI1145  - visible light,IR,UV
(0x40) HTU21D  - temp,humi
********************************************************/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Bme280BoschWrapper.h>
#include <Adafruit_SI1145.h>
#include <SparkFunHTU21D.h>
/******************** ADC MACRO FOR VCC MESSUREMENT ******/
ADC_MODE(ADC_VCC);
/******************** DEFUALT CONFIGURATION **************/
const int UNIT_NO = 0;
const char* MQTT_SERVER = "################## MQTT server address goes here ################";
const char* MQTT_USER = "################## MQTT client name goes here ################";
const char* MQTT_PASS = "################## MQTT client password goes here ################";
const int MQTT_PORT = 1883;
const int ESP_SLEEP = 300; /* SLEEP TIME (s)*/
/******************** MAIN VARIABLES *********************/
WiFiClient espClient;
PubSubClient client(MQTT_SERVER,MQTT_PORT,espClient);
Adafruit_SI1145 si = Adafruit_SI1145();
Bme280BoschWrapper bme(true);
HTU21D htu;
char buff [12];
char buff2 [256];
DynamicJsonBuffer jsonBuffer; 
/******************** SETUP ON BOOT **********************/
void setup(){
  /* WIFI MANAGER SETUP */
  WiFiManager wifiManager;
  if(!wifiManager.autoConnect()){
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
  /* SENSORS SETUP */
  si.begin();
  bme.beginI2C(0x76);
  htu.begin();
  /* START MQTT CLIENT */
  sprintf(buff, "unit%d", UNIT_NO);
  client.connect(buff,MQTT_USER,MQTT_PASS);
  delay(100);
}
/******************** MAIN LOOP **************************/
void loop(){
  /* SI1145 */
  uint16_t v = si.readVisible();
  uint16_t ir = si.readIR();
  uint16_t u = si.readUV();
  float uv = (u / 100.0);
  /* HTU21D */
  float h = htu.readHumidity();
  float t = htu.readTemperature();
  /* BME280 */
  float t,h,p;
  bme.measure();
  delay(60);
  p = bme.getPressureDouble() / 100.0;
  /* VOLTAGE */
  float b = (ESP.getVcc() / 1000.0);
  /* PUBLISH ALL VALUES AS JSON PAYLOAD VIA MQTT */
  sprintf(buff, "iot/unit%d", UNIT_NO);
  JsonObject& root = jsonBuffer.createObject();
  root["t"] = t; root["h"] = h; root["p"] = p; root["uv"] = uv; root["ir"] = ir; root["v"] = v; root["b"] = b;
  root.printTo(buff2, sizeof(buff2));
  client.publish(buff,buff2,true);
  /* DEEP SLEEEP */
  ESP.deepSleep(ESP_SLEEP * 1000000);
}
