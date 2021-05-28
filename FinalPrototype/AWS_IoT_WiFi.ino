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
#include <dht.h>

#define dht_apin0  A0 
 
   

      dht DHT;

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
//#include <WiFiNINA.h> // change to 
#include <WiFi101.h>// for MKR1000
#include <ArduinoJson.h>
#include "arduino_secrets.h"

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;


int samplingTime = 280;                         // Setting the sampling time for the measurmeent
      int deltaTime = 40;
      int sleepTime = 9680;
      float voMeasured = 0;                           // init the variable that will hold the output
      float calcVoltage = 0;
      float dustDensity = 0;                          //  this is the value we will be printing

  int redLed = 2;                                 //  Red LED at pin 2  This is according to the Golden excell sheet  .  Threshold for  Very BAD quality air
      int greenLed = 4;                               //  Green LED at pin 4.   . Threshold for BAD qulaity air
      int yellowLed = 3;                              //  yellow at 2           Threshold for good  qulaity air
      int blueLed = 1;                                // blue on during RESET
      int buzzer = 5;                                 //  buzzer at pin D5.  It is on when there is an issue
      int rst = 7;                                    //  Reset the sensor
      int ledPower = 0;                               //  Connect 3 led driver pins of dust sensor to Arduino D0
      int measurePin = A5;                            //  Connect dust sensor to Arduino A5 pin

int rststate = 0;                                 //  to hold the state value of reset
int calibrate = 0;                                // to hold the sensor calibration value












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
      DHT.read11(dht_apin0);// print out the value you read:
    int MQ2SensorValue = analogRead(A1);              // MQ2 sensor value Read
    int MQ5SensorValue = analogRead(A2);              // MQ5 sensor value Read
    int MQ9SensorValue = analogRead(A3);              // MQ9 sensor value Read
    int MQ135SensorValue = analogRead(A4);            // MQ135 sensor value Read
    digitalWrite(ledPower,LOW);                       // power on the LED
    delayMicroseconds(samplingTime);

    voMeasured = analogRead(measurePin); // read the dust value

 delayMicroseconds(deltaTime);
    digitalWrite(ledPower,HIGH); // turn the LED off
    delayMicroseconds(sleepTime);
    rststate = digitalRead(rst);
  
    // 0 - 5V mapped to 0 - 1023 integer values
    // recover voltage
  
    calcVoltage = voMeasured * (5.0 / 1024.0);
    dustDensity = (0.17 * (calcVoltage -0.01))*1000;
    float  MQ135ppm = MQ135SensorValue * (.0825 * 294.15) / 46.07;
    float  MQ9ppm =   MQ9SensorValue  ;
    float  MQ5ppm =   MQ5SensorValue  ;
    float  MQ2ppm =   MQ2SensorValue  ;

    if ((MQ2ppm <= 40)&&(MQ5ppm <= 80)&&(MQ9ppm <= 180)&&(MQ135ppm <= 500)){
    digitalWrite(greenLed, HIGH);                       //greenLed On
    digitalWrite(yellowLed, LOW); 
    digitalWrite(redLed, LOW); 
    digitalWrite(blueLed, LOW);

    StaticJsonDocument<128> jsonDoc;
    jsonDoc["Humidity"]=DHT.humidity;
    jsonDoc["Temperature"]=DHT.temperature;
    jsonDoc["MQ2"]=MQ2SensorValue;
    jsonDoc["MQ5"]=MQ5SensorValue;
    jsonDoc["MQ9"]=MQ9SensorValue;
    jsonDoc["MQ135"]=MQ135ppm;

    jsonDoc["DustDensity"]=dustDensity;

    char jsonBuffer[512];
    serializeJsonPretty(jsonDoc,jsonBuffer);
    
     mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);
 // mqttClient.print(millis());
  mqttClient.endMessage();
  /*  Serial.print("\n");
    Serial.print("Good Quality Air: ");
    //Serial.println(ppm);
    
    Serial.print("Room Humidity =  ");
    Serial.print(DHT.humidity);
    Serial.print("\n");
    Serial.print("Temperature  =  ");
    Serial.print(DHT.temperature);
    Serial.print("C ");
    Serial.print("\n");
    Serial.print("MQ2 = ");
    Serial.print(MQ2SensorValue);
    Serial.print("\n");
    Serial.print("MQ5 = ");
    Serial.print(MQ5SensorValue);
    Serial.print("\n");
    Serial.print("MQ9 = ");
    Serial.print(MQ9SensorValue);
    Serial.print("\n");
    Serial.print("MQ135 = ");
    Serial.print(MQ135ppm);
    Serial.print("\n");
    Serial.print("Dust = ");
    Serial.println(dustDensity); // unit: ug/m3
    Serial.print("\n");
    */
    delay(300);
    
    //digitalWrite(buzzer,HIGH); // turn off the ALARM
    //tone(buzzer,20);          // Increase or decrease the number to hear different tones
  }
     else if((MQ2ppm > 40)||(MQ2ppm > 80)||(MQ9ppm > 150)||(MQ135ppm > 550)){
      
    
    digitalWrite(greenLed, LOW);                       //greenLed On
    digitalWrite(yellowLed, HIGH); 
    digitalWrite(redLed, LOW); 
    digitalWrite(blueLed, LOW);

    
    Serial.print("\n");
    
  //  Serial.println(ppm);
    DHT.read11(dht_apin0);// print out the value you read:
    StaticJsonDocument<128> jsonDoc;
    jsonDoc["Humidity"]=DHT.humidity;
    jsonDoc["Temperature"]=DHT.temperature;
    jsonDoc["MQ2"]=MQ2SensorValue;
    jsonDoc["MQ5"]=MQ5SensorValue;
    jsonDoc["MQ9"]=MQ9SensorValue;
    jsonDoc["MQ135"]=MQ135ppm;

    jsonDoc["DustDensity"]=dustDensity;
    char jsonBuffer[512];
    serializeJsonPretty(jsonDoc,jsonBuffer);
    
     mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);
 // mqttClient.print(millis());
  mqttClient.endMessage();
    
    /*
    Serial.print("Room Humidity =  ");
    Serial.print(DHT.humidity);
    Serial.print("% ");
    Serial.print("\n");
    Serial.print("Temperature  =  ");
    Serial.print(DHT.temperature);
    Serial.print("C ");
    Serial.print("\n");
    Serial.print("MQ2 = ");
    Serial.print(MQ2SensorValue);
    Serial.print("\n");
    Serial.print("MQ5 = ");
    Serial.print(MQ5SensorValue);
    Serial.print("\n");
    Serial.print("MQ9 = ");
    Serial.print(MQ9SensorValue);
    Serial.print("\n");
    Serial.print("MQ135 = ");
    Serial.print(MQ135ppm);
    Serial.print("\n");
    Serial.print("Dust = ");
   Serial.println(dustDensity); // unit: ug/m3
   Serial.print("\n");
    digitalWrite(buzzer,LOW);                        // sound the ALARM
   tone(buzzer,0);          // Increase or decrease the number to hear different tones
 delay(3000);
  
  */
  
  
  }
  else {
    digitalWrite(greenLed, LOW);                       //greenLed On
    digitalWrite(yellowLed, LOW); 
    digitalWrite(redLed, HIGH); 
    digitalWrite(blueLed, LOW);
    StaticJsonDocument<128> jsonDoc;
    jsonDoc["Humidity"]=DHT.humidity;
    jsonDoc["Temperature"]=DHT.temperature;
    jsonDoc["MQ2"]=MQ2SensorValue;
    jsonDoc["MQ5"]=MQ5SensorValue;
    jsonDoc["MQ9"]=MQ9SensorValue;
    jsonDoc["MQ135"]=MQ135ppm;

    jsonDoc["DustDensity"]=dustDensity;
   
    char jsonBuffer[512];
    serializeJsonPretty(jsonDoc,jsonBuffer);
    
     mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);
 // mqttClient.print(millis());
  mqttClient.endMessage();
    
    /*
    Serial.print("\n");
    Serial.print("Good Quality Air: ");
   // Serial.println(ppm);
   // DHT.read11(dht_apin0);// print out the value you read:
    Serial.print("Room Humidity: ");
    Serial.print(DHT.humidity);
    Serial.print("% ");
    Serial.print("\n");
    Serial.print("Temperature  =  ");
    Serial.print(DHT.temperature);
    Serial.print("C ");
    Serial.print("\n");
    Serial.print("MQ2 = ");
    Serial.print(MQ2SensorValue);
    Serial.print("\n");
    Serial.print("MQ5 = ");
    Serial.print(MQ5SensorValue);
    Serial.print("\n");
    Serial.print("MQ9 = ");
    Serial.print(MQ9SensorValue);
    Serial.print("\n");
    Serial.print("MQ135 = ");
    Serial.print(MQ135ppm);
    Serial.print("\n");
    Serial.print("Dust = ");
    Serial.println(dustDensity); // unit: ug/m3
    Serial.print("\n");
    
    
    */
    
   digitalWrite(buzzer,HIGH);                        // sound the ALARM
   tone(buzzer,100);          // Increase or decrease the number to hear different tones
   delay(2000);
  }

  if (rststate == LOW)
  {
    digitalWrite(blueLed, HIGH);                       //Blue ON
    digitalWrite(greenLed, LOW);                       //greenLed On
    digitalWrite(yellowLed, LOW); 
    digitalWrite(redLed, LOW); 
    StaticJsonDocument<128> jsonDoc;
    jsonDoc["Humidity"]=0;
    jsonDoc["Temperature"]=0;
    jsonDoc["MQ2"]=0;
    jsonDoc["MQ5"]=0;
    jsonDoc["MQ9"]=0;
    jsonDoc["MQ135"]=0;

    jsonDoc["DustDensity"]=0;
char jsonBuffer[512];
    serializeJsonPretty(jsonDoc,jsonBuffer);
    
     mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(jsonBuffer);
 // mqttClient.print(millis());
  mqttClient.endMessage();
    
    Serial.print("Resetting the Sensor \n");
   /*
   // Serial.println(ppm);
    DHT.read11(dht_apin0);// print out the value you read:
    //Serial.print("Room Humidity: ");
    DHT.humidity = 0;
   // Serial.print(DHT.humidity);
   // Serial.print("% ");
    //Serial.print("\n");
    //Serial.print("Temperature  =  ");
    DHT.temperature = 0;
    //Serial.print(DHT.temperature);
    //Serial.print("C ");
    //Serial.print("\n");
    //Serial.print("MQ2 = ");
    MQ2SensorValue = 0 ;
    //Serial.print(MQ2SensorValue);
    //Serial.print("\n");
    //Serial.print("MQ5 = ");
    MQ5SensorValue = 0 ;
    //Serial.print(MQ5SensorValue);
    //Serial.print("\n");
    //Serial.print("MQ9 = ");
    MQ9SensorValue = 0;
    //Serial.print(MQ9SensorValue);
    //Serial.print("\n"); 
    //Serial.print("MQ135 = ");
    MQ135ppm = 0;
    //Serial.print(MQ135ppm);
    //Serial.print("\n");
    //Serial.print("\n");
    //Serial.print("Dust = ");
    dustDensity = 0;
    //Serial.println(dustDensity); // unit: ug/m3
    */
    
    
   digitalWrite(buzzer,LOW); // turn off the ALARM
   tone(buzzer,0);          // Increase or decrease the number to hear different tones
   
    
    delay(1000);
    Serial.print("Resetting Complete \n");

    // Need to reset the output
    
  }



  // send message, the Print interface can be used to set the message contents
 
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
