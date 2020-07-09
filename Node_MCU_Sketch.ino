#include <ESP8266WiFi.h>
#include <Arduino.h>

#include "MQTTConnector.h"
#include "Credentials.h"

#define WIFI_TIMEOUT 1000
#define LOOP_TIMEOUT 10000

/*Function to make sure Serial communication is working and connect the NODEMCU to the wifi router*/
void WiFiBegin(const char* ssid, const char* pass)
{
  WiFi.begin(ssid, pass);
  while(!Serial){}
  Serial.printf("Waiting for AP connection ...\n");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_TIMEOUT);
    Serial.printf(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("\nConnected to AP. IP : %d.%d.%d.%d\n", 
    ip[0],ip[1],ip[2],ip[3]);
}

/*array of characters to store incoming messages*/
const byte numChars = 255;
char receivedChars[numChars];


boolean newData = false; //is true when there is an untreated new data

void setup() 
{
  Serial.begin(9600); //Start serial communication
  Serial.setDebugOutput(true);
  WiFiBegin(STA_SSID, STA_PASS);
  MQTTBegin(); //Initalize subscription to broker and topic
}

void loop() 
{
  recvWithStartEndMarkers();
  showNewData();
  sendNewData();
  MQTTLoop(); // makes sure the nodemcu is still connected and reconnects to broker and topic in case it disconnects
}

/*function to recieve the message from the arduino 
Nodemcu will ignore all incoming serial characters until the character "<" is sent
then incoming characters will be stored in receivedChars with a maximum of 255 characters
cahracters stop being stored when ">" is recieved and newData == true*/

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

/*function to display data on the serial port*/

void showNewData() {
    if (newData == true) {
        Serial.print("This just in ... ");
        Serial.println(receivedChars);
    }
}

/*function that checks if there is a message to send and initiate sending */
void sendNewData() {
    if (newData == true) {
        MQTTsend();
    }
}

/*Function that publishes the new message from the arduino in the designated topic */
void MQTTsend(){
  if(MQTTPublish(TOPIC,receivedChars))
  {
      newData=false;
      Serial.printf("MQTTPublish was successful.\n");
  }
}
