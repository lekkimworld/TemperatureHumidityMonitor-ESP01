#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "vars.h"

// defines
#define TEMP_DECIMALS 2
#define HUM_DECIMALS 2
#define DELAY_POST_DATA 120000L            // delay between updates, in milliseconds
#define DELAY_PRINT 15000L
#define DELAY_READ 5000L
#define DELAY_CONNECT_ATTEMPT 10000L
#define DHTPIN 2
#define DHTTYPE DHT22

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// data
uint8_t reconnect;
float temp;
float hum;

// **** WiFi *****
ESP8266WiFiMulti WiFiMulti;
unsigned long lastConnectAttempt = millis();
unsigned long lastPostData = millis();
unsigned long lastPrint = millis();
unsigned long lastRead = millis();

void setup() {
  // initialize serial
  Serial.begin(115200);
  
  // Start up the sensors
  Serial.println("Version: 6");
  Serial.println("Initializing sensors");
  Serial.println("Using DHT Sensor");
}

void loop() {
  wl_status_t status = WiFi.status();
  if (status != WL_CONNECTED && ((unsigned long)millis() - lastConnectAttempt > DELAY_CONNECT_ATTEMPT)) {
    // not connected - init wifi
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid1, pass1);
    if (strlen(ssid2) > 0) {
      WiFiMulti.addAP(ssid2, pass2);
    }
    Serial.println("Not connected attempting reconnect...");
    lastConnectAttempt = millis();
    if (WiFiMulti.run() != WL_CONNECTED) {
      reconnect++;
      Serial.print("Could not reconnect (attempt: ");
      Serial.print(reconnect);
      Serial.println(")...");
      if (reconnect >= 3) {
        Serial.println("Resetting...");
        ESP.reset();
        return;
      }
    }
    
  } else if (status == WL_CONNECTED) {
    reconnect = 0;
    // read sensors if applicable
    if ((((unsigned long)millis()) - lastRead) > DELAY_READ) {
      Serial.println("Read temperatures...");
      lastRead = millis();
      readData();
    }
    yield();

    // print temperatures
    if ((((unsigned long)millis()) - lastPrint) > DELAY_PRINT) {
      Serial.println("Print temperatures...");
      lastPrint = millis();
      printData();
    }
    yield();

    // post temperatures
    if ((((unsigned long)millis()) - lastPostData) >= DELAY_POST_DATA) {
      Serial.println("Post temperatures...");
      lastPostData = millis();
      sendData();
    }
    yield();
  }
  
  yield();
}

void readData() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
}

void printData() {
  // validate
  if (!ensureValidData()) {
    Serial.println("Temperatur or humidity is NaN - check for sensor or loose connections - aborting print...");
    return;
  }
  
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, ");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
}

void sendData() {
  // validate
  if (!ensureValidData()) {
    Serial.println("Temperatur or humidity is NaN - check for sensor or loose connections - aborting send...");
    return;
  }
  
  // prepare content
  String content = "[{\"sensorId\": \"";
  content += sensorId_Temp;
  content += String("\", \"sensorValue\": ");
  content += String(temp, TEMP_DECIMALS);
  content += String("}, {\"sensorId\": \"");
  content += sensorId_Hum;
  content += String("\", \"sensorValue\": ");
  content += String(hum, HUM_DECIMALS);
  content += String("}]");
  Serial.print("Content: ");
  Serial.println(content);

  // send
  HTTPClient http;
  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Length", String(content.length() + 4));
  int httpCode = http.POST(content);
  String payload = http.getString();                  //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();

  Serial.println("Sent to server...");
}

uint8_t ensureValidData() {
  if (isnan(temp) || isnan(hum)) {
    return false;
  } else {
    return true;
  }
}

