/*
 *   MQTT Sensor - Temperature and Humidity (DHT22) for Home-Assistant - NodeMCU (ESP8266)
 *   https://home-assistant.io/components/sensor.mqtt/
 * 
 *   Libraries :
 *    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
 *    - PubSubClient : https://github.com/knolleary/pubsubclient
 *    - DHT : https://github.com/adafruit/DHT-sensor-library
 *    - ArduinoJson : https://github.com/bblanchon/ArduinoJson
 * 
 *   Sources :
 *    - File > Examples > ES8266WiFi > WiFiClient
 *    - File > Examples > PubSubClient > mqtt_auth
 *    - File > Examples > PubSubClient > mqtt_esp8266
 *    - File > Examples > DHT sensor library > DHTtester
 *    - File > Examples > ArduinoJson > JsonGeneratorExample
 *    - http://www.jerome-bernard.com/blog/2015/10/04/wifi-temperature-sensor-with-nodemcu-esp8266/
 * 
 *   Schematic :
 *    - https://github.com/mertenats/open-home-automation/blob/master/ha_mqtt_sensor_dht22/Schematic.png
 *    - DHT22 leg 1 - VCC
 *    - DHT22 leg 2 - D1/GPIO5 - Resistor 4.7K Ohms - GND
 *    - DHT22 leg 4 - GND
 *    - D0/GPIO16 - RST (wake-up purpose)
 * 
 *   Configuration (HA) :
sensor 1:
    platform: mqtt
    name: "Temperature ESP8266 1"
    state_topic: "home/esp8266_1/sensor"
    unit_of_measurement: "°C"
    device_class: "temperature"
    value_template: "{{ value_json.temperature }}"

sensor 2:
    platform: mqtt
    name: "Humidity ESP8266 1"
    state_topic: "home/esp8266_1/sensor"
    unit_of_measurement: "%"
    device_class: "humidity"
    value_template: "{{ value_json.humidity }}"

sensor 3:
    platform: mqtt
    name: "Logger ESP8266 1"
    state_topic: "home/esp8266_1/log"#    
    value_template: "{{ value_json.log }}"

Damian Barsotti - v1.0 02.2021
Based on:
 *   Samuel M. - v1.1 - 08.2016
 *   https://github.com/mertenats/open-home-automation
 * 
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#include "src/esp8266_controllers/Wifi/Wifi.h"
#include "src/esp8266_controllers/HTReader/HTReader.h"
#include "src/esp8266_controllers/Mqtt/Mqtt.h"

#if __has_include("config_local.h")
#include "config_local.h" // File for testing outside git
#else
#include "config.h"
#endif

#ifdef STATIC_ADDRESS
Wifi wifi(Serial, WIFI_SSID, WIFI_PASSWORD, local_IP, gateway, subnet);
#else
Wifi wifi(Serial, WIFI_SSID, WIFI_PASSWORD);
#endif

HTReader ht_sensor(
    DHTPIN, DHTTYPE, SLEEPING_TIME_IN_MSECONDS,
    temp_slope, temp_shift,
    humid_slope, humid_shift,
    n_reads);

std::array topics{MQTT_SENSOR_TOPIC};

Mqtt mqtt(Serial, MQTT_SERVER_IP, MQTT_SERVER_PORT,
          MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD,
          MQTT_WILL_TOPIC, MQTT_LOG_TOPIC,
          topics.data(), topics.size(),
          callback);

String header_log(char const *level, int n_log)
{
    return String(level) + " " + String(n_log) + ": ";
}

String header_log_info(int n_log)
{
    return header_log("INFO", n_log);
}

String header_log_warn(int n_log)
{
    return header_log("WARN", n_log);
}

int n_log_info = 0;
int n_log_warn = 0;

void logger_info(String msg)
{
    if (mqtt.log(header_log_info(n_log_info) + msg))
        n_log_info++;
}

void logger_warn(String msg)
{
    if (mqtt.log(header_log_warn(n_log_warn) + msg))
        n_log_warn++;
}

// function called to publish the temperature and the humidity
void publish_data_sensor(float p_temperature, float p_humidity)
{
    DynamicJsonDocument root(200);
    root["temperature"] = (String)p_temperature;
    root["humidity"] = (String)p_humidity;

    mqtt.publish(root, MQTT_SENSOR_TOPIC);
}

// function called when a MQTT message arrived
void callback(char *topic, byte *payload, unsigned int length){}

void send_mqtt_connect_info(int attempt)
{

    if (LOG_MQTT_CONNECT)
    {
        logger_info(String("IP address: ") + wifi.localIP().toString());
        logger_info(String("MQTT SERVER IP: ") + MQTT_SERVER_IP + ":" + MQTT_SERVER_PORT);
        logger_info(String("MQTT CLIENT ID: ") + MQTT_CLIENT_ID);
        logger_info(String("temp slope: ") + temp_slope + String(", temp shift: ") + temp_shift + String(", humid slope: ") + humid_slope + String(", humid shift: ") + humid_shift);
        logger_info(String("Number of attempts: ") + String(attempt));
    }
}

void setup() {

    // init the serial
    Serial.begin(115200);
    //Take some time to open up the Serial Monitor
    delay(1000);

     // Restart ESP if max attempt reached
    if (!wifi.begin())
    {
        Serial.println("ERROR: max_attempt reached to WiFi connect");
        Serial.print("Waiting and Restaring");
        wifi.disconnect();
        delay(1000);
        ESP.restart();
    }

    Serial.println(String("IP: ") + wifi.localIP().toString());

    // init the MQTT connection
    if (!mqtt.begin() && mqtt.attempt() > mqtt_max_attempt)
    {
        Serial.println("MQTT error");
        Serial.print("Waiting and Restaring");
        wifi.disconnect();
        delay(1000);
        ESP.restart();
    }

    ht_sensor.begin();
    if (ht_sensor.error())
        logger_warn("Failed to read from DHT sensor!");

}

void loop() {
    
    float t, h;

    Serial.print("-");

    if (mqtt.beginLoop())
    {
        if (ht_sensor.error())
            Serial.println("Failed to read from sensor!");

        if (ht_sensor.beginLoop())
        {
            Serial.println();
            publish_data_sensor(ht_sensor.getTemp(), ht_sensor.getHumid());
        }

        if (ht_sensor.error())
            logger_warn("Failed to read from DHT sensor!");
    }
    else if (mqtt.attempt() > mqtt_max_attempt)
    {
        Serial.println("Cannot connect to mqtt");
        Serial.print("Waiting and Restaring");
        mqtt.reset();
        wifi.disconnect();
        delay(1000);
        ESP.restart();
    }

    delay(SLEEPING_TIME_IN_MSECONDS);

    // ESP.deepSleep(SLEEPING_TIME_IN_MSECONDS * 1000, WAKE_RF_DEFAULT);
    // delay(500); // wait for deep sleep to happen
}
