#ifndef CONFIG_H
#define CONFIG_H

// Wifi: SSID and password
const PROGMEM char* WIFI_SSID = "[Redacted]";
const PROGMEM char* WIFI_PASSWORD = "[Redacted]";

// If 0.0.0.0 use dhcp
const IPAddress local_IP(0,0,0,0);

// If neded set your Static IP address
//const IPAddress local_IP(192, 168, 0, 200);
// Set your Gateway IP address
//const IPAddress gateway(192, 168, 0, 1);
//const IPAddress subnet(255, 255, 255, 0);
// Optional DNS
//IPAddress primaryDNS(8, 8, 8, 8);   //optional
//IPAddress secondaryDNS(8, 8, 4, 4); //optional

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "home_sensor1_dht11";
const PROGMEM char* MQTT_SERVER_IP = "[Redacted]";
const uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "[Redacted]";
const PROGMEM char* MQTT_PASSWORD = "[Redacted]";

// MQTT: topics
const PROGMEM char* MQTT_SENSOR_TOPIC = "home/esp8266_1/sensor";
const PROGMEM char* MQTT_LOG_TOPIC = "home/esp8266_1/log";

// sleeping time
const uint16_t SLEEPING_TIME_IN_SECONDS = 20; 

// To adjust reads of DHT11

const float temp_slope = 1.005f;
const float temp_shift = -1.746f;
const float humid_slope = 1.775f;
const float humid_shift = -38.07f;

#ifdef  ARDUINO_ESP8266_GENERIC
#define D2 4
#define D3 0
#define D4 2
#define D9 3
#endif

#define DHTPIN D4
#define DHTTYPE DHT11

#endif
