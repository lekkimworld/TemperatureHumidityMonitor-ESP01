#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "vars.h"

// defines
#define TEMP_DECIMALS 2                 // decimals for temperature
#define HUM_DECIMALS 2                  // decimals for humidity
#define DELAY_POST_DATA 120000L         // delay between updates, in milliseconds
#define DELAY_PRINT 15000L              // delay between printing to the console, in milliseconds
#define DELAY_READ 5000L                // delay between reading the sensor(s), in milliseconds
#define DELAY_CONNECT_ATTEMPT 10000L    // delay between attempting wifi reconnect, in milliseconds
#define DHTPIN 2                        // pin of DHT sensor
#define DHTTYPE DHT22                   // type of DHT sensor

// initialize DHT sensor
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
  Serial.println("Version: 20180522T0816");
  Serial.println("Initializing sensors");
  Serial.println("Using DHT22 sensor");
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
      Serial.println("Read data...");
      lastRead = millis();
      readData();
    }
    yield();

    // print temperatures
    if ((((unsigned long)millis()) - lastPrint) > DELAY_PRINT) {
      Serial.println("Print data...");
      lastPrint = millis();
      printData();
    }
    yield();

    // post temperatures
    if ((((unsigned long)millis()) - lastPostData) >= DELAY_POST_DATA) {
      Serial.println("Post data...");
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
    return;
  }
  
  Serial.print("Humidity: ");
  Serial.print(hum, HUM_DECIMALS);
  Serial.print(" %, ");
  Serial.print("Temperature: ");
  Serial.print(temp, TEMP_DECIMALS);
  Serial.println(" C");
}

void sendData() {
  // validate
  if (!ensureValidData()) {
    return;
  }
  
  // prepare content
  // data = 20
  // obj_json = 33
  // array_brackets = 2
  // obj_sep_comma = 2
  // n*data + n * obj_json + array_brackets + (n-1)*obj_sep_comma
  char content[110];
  char str_temp[8];
  char str_hum[8];
  dtostrf(temp, 6, TEMP_DECIMALS, str_temp);
  dtostrf(hum, 6, HUM_DECIMALS, str_hum);
  
  strcpy(content, "[{\"sensorId\": \"");
  strcat(content, sensorId_Temp);
  strcat(content, "\", \"sensorValue\": ");
  strcat(content, str_temp);
  strcat(content, "}, {\"sensorId\": \"");
  strcat(content, sensorId_Hum);
  strcat(content, "\", \"sensorValue\": ");
  strcat(content, str_hum);
  strcat(content, "}]");
  
  // prepare headers
  uint8_t contentLength = strlen(content) + 4;
  char str_contentLength[4];
  sprintf (str_contentLength, "%03i", contentLength);
  Serial.print("Content-Length: ");
  Serial.println(str_contentLength);
  Serial.print("Payload: <");
  Serial.print(content);
  Serial.println(">");

  // send
  HTTPClient http;
  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Length", str_contentLength);
  int httpCode = http.POST(content);
  String payload = http.getString();                  //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();

  Serial.println("Sent to server...");
}

uint8_t ensureValidData() {
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Temperatur or humidity is NaN - check for sensor or loose connections - aborting...");
    return false;
  } else {
    return true;
  }
}

