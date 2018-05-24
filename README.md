# TemperatureHumidityMonitor from ESP-01

Is the Arduino C code for a temperature and humidity sensor running on top of an ESP-01 wifi-chip using a DHT22 sensor to measure temperature and humidity. The data is read contineously, printed to `Serial` on a contineous basis for debug as well as sent to a cloud service contineously. The data is POSTed as JSON in a fire-and-forget manner.

The device built is mounted in a ABS enclosure and use a 220V AC to 3.3V DC converter on the PCB. Besides this voltage conversion it's a very basic ESP-01 setup with 2 push button for reset, a push button to put the ESP-01 into flash mode (used with reset) a female headers to access to FTDI module for debugging, flashing and power.

## Parts
* HLK-PM03 AC-DC 220V to 3.3V Step Down Buck Converter
* ESP8266 ESP-01
* 3 10k Ohm resistors
* 2 push buttons
* 1 DHT22 sensor
* 1 100uF capacitor used as a decoupling capacitor for the ESP-01
* Female headers
* ABS enclosure
* Wires, PCB, solder and patience

## Code
Code is compiled using Arduino IDE and uploaded using the `esptool` as I often experience that the Arduino IDE fails to completely upload the code. For me the `esptool` works consistantly.

__Please note:__ Remember to create a header file called `vars.h` with SSID, Wifi password and the address of the 
server to post to. Example below:
```C
const char ssid1[]   = "AP_IoT";
const char pass1[]   = "Passw0rd";
//optional wifi access point 2 ssid and passphrase
//const char ssid2[]   = "AP_IoT2";   
//const char pass2[]   = "Passw0rd";
const char server[] = "http://funky-iot-28731.herokuapp.com";
const char sensorId_Temp[] = "dht1_temp";
const char sensorId_Hum[] = "dht1_hum";
```
