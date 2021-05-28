#include "WiFi.h"

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "certs.h"
#include <DHT.h> 
#include <MQTTClient.h>
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

#define DEVICE_NAME "esp"
#define AWS_IOT_ENDPOINT "am9qpk5amm5vb-ats.iot.us-east-1.amazonaws.com"
#define AWS_IOT_TOPIC "/shadow/update"
#define AWS_MAX_RECONNECT_TRIES 50


WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient();
float humidity,temperature;


int redLed = 7;                                 //  Red LED at pin 7  .  Threshold for  Very BAD quality air
int greenLed = 8;                               //  Green LED at pin 8  . Threshold for BAD qulaity air
int orangeLed = 2;                            //  Orrange at 2           Threshold for good  qulaity air
int buzzer = 6;                                 //  buzzer at pin 5
int rst = 4;                                    //  Reset the sensor
int blueLed = 5;                                // blue on during RESET
int rststate = 0;                                 //  to hold the state value of reset
void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Only try 15 times to connect to the WiFi
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 15){
    delay(500);
    Serial.print(".");
    retries++;
}
  if(WiFi.status() != WL_CONNECTED){
    esp_sleep_enable_timer_wakeup(1 * 60L * 1000000L);
    esp_deep_sleep_start();
  }
}

void connectToAWS()
{
  // Configure WiFiClientSecure to use the AWS certificates we generated
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Try to connect to AWS and count how many times we retried.
  int retries = 0;
  Serial.print("Connecting to AWS IOT");

  while (!client.connect(DEVICE_NAME) && retries < AWS_MAX_RECONNECT_TRIES) {
    Serial.print(".");
    delay(100);
    retries++;
  }

  // Make sure that we did indeed successfully connect to the MQTT broker
  // If not we just end the function and wait for the next loop.
  if(!client.connected()){
    Serial.println(" Timeout!");
    return;
  }

  // If we land here, we have successfully connected to AWS!
  // And we can subscribe to topics and send messages.
  Serial.println("Connected!");
}

void sendJsonToAWS()
{
  StaticJsonDocument<128> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("state");
  JsonObject reportedObj = stateObj.createNestedObject("reported");
  

  int sensorValue = analogRead(A2);         // MQ135 sensor value Read
  // print out the value you read:
  rststate = digitalRead(rst);
  int ppm = sensorValue * (.0825 * 294.15) / 46.07;
  if (ppm <= 500){
    digitalWrite(greenLed, HIGH);                       //greenLed ON
    Serial.print("Good quality air: ");
    Serial.println(ppm);
    Serial.print("\n");
    digitalWrite(buzzer,HIGH); // turn off the ALARM
     tone(buzzer,20);          // Increase or decrease the number to hear different tones
  }
  else if(ppm > 500 && ppm < 1000){
    digitalWrite(orangeLed, HIGH);                       //orangeLed ON
    digitalWrite(greenLed,LOW);
    Serial.print("Moderate quality air: ");
    Serial.println(ppm); 
    Serial.print("\n");
    // digitalWrite(buzzer,HIGH); // sound the ALARM
    // tone(buzzer,0);
  }
  else{
    digitalWrite(redLed, HIGH);                       //redLed ON
    digitalWrite(greenLed,LOW);
    digitalWrite(orangeLed, LOW); 
    Serial.print("Bad quality air: ");
    Serial.println(ppm); 
    Serial.print("\n");
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
    delay(1000);

    // Need to reset the output
    
  }

   // dht.begin();
   // temperature = dht.readTemperature();
   // humidity = dht.readHumidity();
  // Write the temperature & humidity. Here you can use any C++ type (and you can refer to variables)
  reportedObj["temperature"] = 44.21;
  reportedObj["humidity"] = 66.62;
  reportedObj["AQI"] = ppm;
  
  // Create a nested object "location"
  JsonObject locationObj = reportedObj.createNestedObject("location");
  locationObj["Room"] = "Living";
  //serializeJson(doc, Serial);
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);
  
  client.publish(AWS_IOT_TOPIC, jsonBuffer);
  Serial.println("Publishing message to AWS...");
  serializeJson(jsonDoc, Serial);
}


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  connectToWiFi();
  connectToAWS();

// JSON_MQ135 Datapoint
// the loop routine runs over and over again forever:
pinMode(buzzer, OUTPUT);                    //  set the buzzer as output
pinMode(redLed, OUTPUT);                    //  set redLed as output
pinMode(greenLed, OUTPUT);                  //  set greenLed as output
pinMode(orangeLed, OUTPUT);                  //  set greenLed as output
pinMode(rst, INPUT);                          // Reset the sensor in case something goes wrong

Serial.print("Air Quality \n");

}
void loop() 
{
// read the input on analog pin 0:
  sendJsonToAWS();
  client.loop();
  delay(1000);        // delay in between reads for stability
  delay(2000);
}
