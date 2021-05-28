/*
  AWS IoT WiFi

  This sketch securely connects to an AWS IoT using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a public
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

  The circuit:
  - Arduino MKR WiFi 1010 or MKR1000

  The following tutorial on Arduino Project Hub can be used
  to setup your AWS account and the MKR board:

  https://create.arduino.cc/projecthub/132016/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365

  This example code is in the public domain.
*/


#include <DHT.h> // downloaded by adafruit
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#define DHTTYPE    DHT11     // DHT 11

int redLed = 3;                                 //  Red LED at pin 7  .  Threshold for  Very BAD quality air
int greenLed = 6;                               //  Green LED at pin 8  . Threshold for BAD qulaity air
int orangeLed = 0;                            //  Orrange at 2           Threshold for good  qulaity air
int buzzer = 5;                                 //  buzzer at pin 5
int rst = 4;                                    //  Reset the sensor
int blueLed = 2;                                // blue on during RESET
int rststate = 0;                                 //  to hold the state value of reset
#define dht_apin A1
DHT dht(dht_apin, DHTTYPE);


#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
//#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000

#include <WiFi101.h> 

#include "arduino_secrets.h"

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();

    publishMessage();
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
}

void publishMessage() {
  Serial.println("Publishing message");
    //DHT.read11(dht_apin);// print out the value you read:
  //Serial.println(dht.temperature);

//float t = dht.readTemperature();
//Serial.println(t);

  
 StaticJsonDocument<128> jsonDoc;
JsonObject stateObj = jsonDoc.createNestedObject("state");
JsonObject reportedObj = stateObj.createNestedObject("reported");

int sensorValue = analogRead(A2);         // MQ135 sensor value Read
  
//  DHT.read11(dht_apin);// print out the value you read:
  rststate = digitalRead(rst);
      float temp = dht.readTemperature(); //  Read the DHT Temperature sensor


      float ppm = sensorValue * (.0825 * (273.15+temp)) / 46.07;

    //Serial.print("Room Humidity: ");
   // Serial.print(DHT.humidity);
    //Serial.print("% ");
    //Serial.print("\n");
    //Serial.print("Temperature  =  ");
    //Serial.print(DHT.temperature);
    //Serial.print("C ");
    //Serial.print("\n");
  if (ppm <= 500){
    digitalWrite(greenLed, HIGH);                       //greenLed On
    digitalWrite(orangeLed, LOW);  
    digitalWrite(redLed, LOW);
    digitalWrite(blueLed, LOW);
    float temp = dht.readTemperature(); //  Read the DHT Temperature sensor
  reportedObj["temperature"] = temp;

  float ppm = sensorValue * (.0825 * (273.15+temp)) / 46.07;
  reportedObj["ppm"] = ppm;
    
    reportedObj["AQI"] = "Good";

char jsonBuffer[512];
  serializeJsonPretty(jsonDoc,jsonBuffer);
  mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);

  mqttClient.endMessage();
    //Serial.print("\n");
    //Serial.print("Good quality air: ");
    //Serial.println(ppm);
    //Serial.print("\n");
    delay(1000);
    digitalWrite(buzzer,HIGH); // turn off the ALARM
     //tone(buzzer,20);          // Increase or decrease the number to hear different tones
  }
  else if(ppm > 500 && ppm < 520){
    digitalWrite(orangeLed, HIGH);                       //orangeLed ON
    digitalWrite(greenLed,LOW);
    digitalWrite(redLed, LOW);
    digitalWrite(blueLed, LOW);
    
    float temp = dht.readTemperature(); //  Read the DHT Temperature sensor
  reportedObj["temperature"] = temp;

  float ppm = sensorValue * (.0825 * (273.15+temp)) / 46.07;
  reportedObj["ppm"] = ppm;
   char jsonBuffer[512];
  serializeJsonPretty(jsonDoc,jsonBuffer); 
    mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);

  mqttClient.endMessage();
    //Serial.print("Moderate quality air: ");
    //Serial.println(ppm); 
    //Serial.print("\n");
    // digitalWrite(buzzer,HIGH); // sound the ALARM
    // tone(buzzer,0);
  }
  else{
    digitalWrite(redLed, HIGH);                       //redLed ON
    digitalWrite(greenLed,LOW);
    digitalWrite(orangeLed, LOW);
    digitalWrite(blueLed, LOW); 
    
    float temp = dht.readTemperature(); //  Read the DHT Temperature sensor
  reportedObj["temperature"] = temp;

  float ppm = sensorValue * (.0825 * (273.15+temp)) / 46.07;
  reportedObj["ppm"] = ppm;
   char jsonBuffer[512];
  serializeJsonPretty(jsonDoc,jsonBuffer); 
    mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);

  mqttClient.endMessage();
    
    //Serial.print("Bad quality air: ");
    //Serial.println(ppm); 
    //Serial.print("\n");
    // digitalWrite(buzzer,HIGH);                        // sound the ALARM
    // tone(buzzer,100);          // Increase or decrease the number to hear different tones
  }

  if (rststate == HIGH)
  {
    digitalWrite(blueLed, HIGH);                       //Blue ON
    Serial.print("Restting the Sensor \n");
    sensorValue = 0;
    ppm = sensorValue;
    Serial.println(ppm);
    sensorValue = 0;
    ppm = sensorValue;
    Serial.println(ppm);  
    sensorValue = 0;
    ppm = sensorValue;
    Serial.println(ppm);
    delay(10);
    
    digitalWrite(redLed, LOW);                       //red OFF
    digitalWrite(orangeLed, LOW);                       //Orrange OFF
    digitalWrite(greenLed, LOW);                       //Green OFF
    digitalWrite(blueLed, LOW);                       //Blue OFF
    delay(3000);

    // Need to reset the output
    
  }

  
  //mqttClient.println(jsonBuffer);
  
  // send message, the Print interface can be used to set the message contents


 //mqttClient.beginMessage("arduino/outgoing");
 // mqttClient.print(jsonBuffer);
  //mqttClient.print(millis());
 // mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}
