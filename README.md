# MQTT Sensor - Temperature and Humidity - Home Assistant
Get temperature and humidity from a DHT11 sensor connected to a NodeMCU board (ESP8266).
Based on [Home Assistant DHT22 MQTT Sensor](https://github.com/mertenats/Open-Home-Automation/tree/master/ha_mqtt_sensor_dht22).

## Improvements
- Upgrade [ArduinoJson](https://arduinojson.org) to version 6.
- Allow fixed IP configuration (in the case that router's dhcp doesn't work).
- Restart ESP if maximum number of attempts is reached. 
- Send log throught MQTT topic.
- Separate `congig.h` file.

## Configuration

### config.h 

Fill this file with your data (the `[redacted]` parts).

### configuration.yaml
```yaml
mqtt:
  sensor:
    - name: "Temperature ESP8266 1"
      state_topic: "home/esp8266_1/sensor"
      unit_of_measurement: "°C"
      device_class: "temperature"
      value_template: "{{ value_json.temperature }}"
    - name: "Humidity ESP8266 1"
      state_topic: "home/esp8266_1/sensor"
      unit_of_measurement: "%"
      device_class: "humidity"
      value_template: "{{ value_json.humidity }}"
    - name: "Logger ESP8266 1"
      state_topic: "home/esp8266_1/log"#    
      value_template: "{{ value_json.log }}"
```
