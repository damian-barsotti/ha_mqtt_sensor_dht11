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
#include "DHT.h"
#include <ArduinoJson.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

#include "config_local.h" // File for testing outside git
#include "config.h"

#include "src/classes/HTReader/HTReader.h"

HTReader *sensor;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void publish(DynamicJsonDocument root, const char* topic){
    
    String data;
    
    //root.remove("log");
    serializeJson(root, data);
    
    Serial.print("Publish in topic "); Serial.println(topic);
    
    int res = client.publish(topic, data.c_str(), true);
    
    serializeJson(root, Serial);
    
    Serial.println();
    if (res != 1)
        Serial.println(String("Publish result: ") + String(res));  
}

String header_log(char* level, int n_log){
    return String(level) + " " + String(n_log) + ": ";
}

String header_log_info(int n_log){
    return header_log("INFO", n_log);
}

String header_log_warn(int n_log){
    return header_log("WARN", n_log);
}

// function to log throught Serial and mqtt topic
bool logger(String msg){
    
    if (client.connected()) {
        DynamicJsonDocument root(msg.length()+17);
        root["log"] = msg;
        //  Serial.println(out.length());
        //  Serial.println(root.capacity());
        //  Serial.println(root.memoryUsage());
        publish(root, MQTT_LOG_TOPIC);
        return true;
    } else {
        Serial.print("ERROR: failed MQTT connection, rc=");
        Serial.println(client.state());
        Serial.println(msg);
        return false;
    }
}

int n_log_info = 0;
int n_log_warn = 0;

void logger_info(String msg){
    if (logger(header_log_info(n_log_info) + msg))
        n_log_info++;
}

void logger_warn(String msg){
    if (logger(header_log_warn(n_log_info) + msg))
        n_log_warn++;
}

// function called to publish the temperature and the humidity
void publishData(float p_temperature, float p_humidity) {
    DynamicJsonDocument root(200);
    root["temperature"] = (String)p_temperature;
    root["humidity"] = (String)p_humidity;
    
    publish(root, MQTT_SENSOR_TOPIC);
    // yield();
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length){}

bool reconnect() {
    // Loop until we're reconnected
    static const int max_attempt = 10;
    int attempt = 0;
    
    while (attempt < max_attempt && !client.connected()) {
        Serial.println("INFO: Attempting MQTT connection...");
        // Attempt to connect
        if (! client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
            Serial.print("ERROR: failed, rc=");
            Serial.println(client.state());
            Serial.println("DEBUG: try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
        attempt++;
    }
    if (attempt < max_attempt){
        logger_info(String("IP address: ") + WiFi.localIP().toString());
        logger_info(String("MQTT SERVER IP: ") + MQTT_SERVER_IP + ":" + MQTT_SERVER_PORT);
        logger_info(String("MQTT CLIENT ID: ") + MQTT_CLIENT_ID);
        logger_info(String("temp slope: ") + temp_slope + String(", temp shift: ") + temp_shift
          + String(", humid slope: ") + humid_slope + String(", humid shift: ") + humid_shift);
        logger_info(String("Number of attempts: ") + String(attempt));
        return true;
    } else
        Serial.println("ERROR: max_attempt reached to MQTT connect");
    
    return attempt < max_attempt;
}

bool setup_wifi() {

    // init the WiFi connection
    Serial.println();
    Serial.print("INFO: Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    
    //  Enable light sleep
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    
    if (local_IP != IPAddress(0,0,0,0) && !WiFi.config(local_IP, gateway, subnet)){
        Serial.println("STA Failed to configure fixed IP");
        return false;    
    }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    static const int max_attempt = 100;
    int attempt = 0;
    while (attempt < max_attempt && WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempt++;
    }
    Serial.println("");

    return (attempt < max_attempt);
}

void setup() {
    // init the serial
    Serial.begin(115200);
    //Take some time to open up the Serial Monitor
    delay(1000);

    // Restart ESP if max attempt reached
    if (!setup_wifi()){
        Serial.println("ERROR: max_attempt reached to WiFi connect");
        Serial.print("Waiting and Restaring");
        delay(SLEEPING_TIME_IN_SECONDS * 1000);
        WiFi.disconnect();
        ESP.restart();
    }

    // init the MQTT connection
    client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    client.setCallback(callback);

    sensor = new HTReader(
        DHTPIN, DHTTYPE, SLEEPING_TIME_IN_SECONDS * 1000, N_AVG_SENSOR,
        temp_slope, temp_shift, humid_slope, humid_shift);

    while (sensor->error()){
        logger_warn("ERROR: sensor read. Retrying ...");
        delay(sensor->delay_ms());
        sensor->reset();
    }

}

void loop() {
    
    if (client.connected() || reconnect()) {
        
        client.loop();

        // Deep sleep restart setup function
        // so sensor reading is always done
        publishData(sensor->getTemp(), sensor->getHumid());

        logger_info("Closing the MQTT connection");
        client.disconnect();
    }
    
    Serial.println("INFO: Closing the Wifi connection");
    WiFi.disconnect();

    ESP.deepSleep(SLEEPING_TIME_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
    delay(500); // wait for deep sleep to happen
}
