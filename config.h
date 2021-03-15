// Fill [Redacted] llaces:

// Wifi: SSID and password
const char* WIFI_SSID = "[Redacted]";
const char* WIFI_PASSWORD = "[Redacted]";

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
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "[Redacted]";
const PROGMEM char* MQTT_PASSWORD = "[Redacted]";

// MQTT: topics
const PROGMEM char* MQTT_SENSOR_TOPIC = "home/esp8266_1/sensor";
const PROGMEM char* MQTT_LOG_TOPIC = "home/esp8266_1/log";

// sleeping time
//const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 600; // 10 minutes x 60 seconds
const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 20;

// To adjust reads of DHT11
const float temp_slope = 1.029f;
const float temp_shift = -5.01f;
const float humid_slope = 0.899f;
const float humid_shift = 21.285f;

