#ifndef ARDUINO_CREDENTIALS_H
#define ARDUINO_CREDENTIALS_H

/* WiFi Credentials to connect Internet */
#define STA_SSID "Your SSID here"
#define STA_PASS "Your password here"

/* Provide MQTT broker credentials as denoted in maqiatto.com. */
#define MQTT_BROKER       "broker name"
#define MQTT_BROKER_PORT  1883
#define MQTT_USERNAME     "Username if any"
#define MQTT_KEY          "Password if any"

/* Provide topic as it is denoted in your topic list. 
 * For example mine is : cadominna@gmail.com/topic1
 * To add topics, see https://www.maqiatto.com/configure
 */
#define TOPIC    "topic to send messages"
#define TOPICSUB "topic to subscribe to"

#define DEVICE_ID "01"
#endif /* ARDUINO_CREDENTIALS_H */