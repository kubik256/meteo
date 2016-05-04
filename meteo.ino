/********************************************************
ESP8266 (ESP-12E) IoT weather station with:
-------------------------------------------
(pin2) DHT22   - humi,temp
(0x76) BME280  - humi,temp,pressure
(0x39) TSL2561 - light,IR
(0x60) SI1145  - lgiht,IR,UV
********************************************************/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <TSL2561.h>
#include <Adafruit_SI1145.h>
#include <BME280.h>
//#include <Adafruit_BME280.h>
/******************** DEFUALT CONFIGURATION **************/
const char* WIFI_NAME = "meteo";
const char* WIFI_SSID = "???????? WIFI NAME ??????????";
const char* WIFI_PASS = "???????? WIFI PASS ??????????";
const IPAddress WIFI_IP(192,168,1,10);
const IPAddress WIFI_GATE(192,168,1,1);
const char* MQTT_SERVER = "???????? MQTT SERVER ??????????";
const char* MQTT_USER = "???????? MQTT USER ??????????";
const char* MQTT_PASS = "???????? MQTT PASS ??????????";
const int MQTT_PORT = 1883; //18875
const int ESP_SLEEP = 600; /* SLEEP TIME (s)*/
/******************** ADC MACRO FOR VCC MESSUREMENT ******/
ADC_MODE(ADC_VCC);
/******************** MAIN VARIABLES *********************/
WiFiClient espClient;
PubSubClient client(MQTT_SERVER,MQTT_PORT,espClient);
DHT dht(2, DHT22); // DHT on PIN 2 ...other sensors are on I2C bus
TSL2561 tsl(TSL2561_ADDR_FLOAT);
Adafruit_SI1145 si = Adafruit_SI1145();
BME280 bme;
/******************** SETUP ON BOOT **********************/
void setup() {
  Serial.begin(115200);
  /* SENSORS BEGIN */
  dht.begin();
  tsl.begin();
  //tsl.setGain(TSL2561_GAIN_16X);
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);
  si.begin();
  bme.begin();
  /* WIFI CONNECTION */
  WiFi.mode(WIFI_STA);
  WiFi.config(WIFI_IP,WIFI_GATE,WIFI_GATE);
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  Serial.print("WIFI ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("CONNECTED.");
  delay(600);
  /* START MQTT CLIENT */
  client.connect("ESP",MQTT_USER,MQTT_PASS);
  Serial.println("MQTT CLIENT CONNECTED.");
}
/******************** MAIN LOOP **************************/
void loop() {
  delay(6000);
  /* DHT22 */
  float t = dht.readTemperature();
  client.publish("esp/dht/t",String(t,1).c_str());
  Serial.print("DHT-t: "); Serial.print(t,1); Serial.print(" C");
  float h = dht.readHumidity();
  client.publish("esp/dht/h",String(h,1).c_str());
  Serial.print("\tDHT-h: "); Serial.print(h,1); Serial.print(" %");
  /* TSL2561 */
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  int vis = (full - ir);
  client.publish("esp/tsl/vis",String(vis).c_str());
  Serial.print("\tTSL-vis: "); Serial.print(vis); Serial.print(" lx");
  client.publish("esp/tsl/ir",String(ir).c_str());
  Serial.print("\tTSL-ir: "); Serial.print(ir); Serial.print(" lx");
  /* SI1145 */
  uint16_t siv = si.readVisible();
  uint16_t sii = si.readIR();
  uint16_t siu = si.readUV();
  Serial.print("\tSI-vis: "); Serial.print(siv); Serial.print(" lx");
  client.publish("esp/si/vis",String(siv).c_str());
  Serial.print("\tSI-ir: "); Serial.print(sii); Serial.print(" lx");
  client.publish("esp/si/ir",String(sii).c_str());
  float uvindex = (siu / 100.0);
  Serial.print("\tSI-uv: ");  Serial.print(uvindex,2); Serial.print(" lx");
  client.publish("esp/si/uv",String(uvindex,2).c_str());
  /* BME280 */
  float temp,humi,pres;
  bme.ReadData(pres,temp,humi);
  pres /= 100.0;
  Serial.print("\tBME-t: "); Serial.print(temp,2); Serial.print(" C");
  client.publish("esp/bme/t",String(temp,2).c_str());
  Serial.print("\tBME-h: "); Serial.print(humi,3); Serial.print(" %");
  client.publish("esp/bme/h",String(humi,3).c_str());
  Serial.print("\tBME-p: "); Serial.print(pres,3); Serial.print(" hPa");
  client.publish("esp/bme/p",String(pres,3).c_str());
  /* VOLTAGE */
  float bat = (ESP.getVcc() / 1000.0);
  Serial.print("\tBAT: "); Serial.println(bat,3); Serial.print(" V");
  client.publish("esp/bat",String(bat,3).c_str());
  /* DEEP SLEEEP */
  ESP.deepSleep(ESP_SLEEP * 1000000);
}