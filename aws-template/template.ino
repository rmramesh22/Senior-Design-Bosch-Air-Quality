
#include "WiFi.h"
// Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
#include <DHT.h> 
//#include<AWS_IOT.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "certs.h"

#include <MQTTClient.h>
#define DHTPIN 27     // Digital pin connected to the DHT sensor

const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

#define DEVICE_NAME "esp"
#define AWS_IOT_ENDPOINT "am9qpk5amm5vb-ats.iot.us-east-1.amazonaws.com"
#define AWS_IOT_TOPIC "/shadow/update"
//#define AWS_IOT_TOPIC "test"

#define AWS_MAX_RECONNECT_TRIES 50


WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient();
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);
float humidity,temperature;
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

  // If we still couldn't connect to the WiFi, go to deep sleep for a minute and try again.
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
  
    dht.begin();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();



  // Write the temperature & humidity. Here you can use any C++ type (and you can refer to variables)
  reportedObj["temperature"] = 44.21;
  reportedObj["humidity"] = 66.62;
  reportedObj["wifi_strength"] = WiFi.RSSI();
  
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
  Serial.begin(9600);
  connectToWiFi();
  connectToAWS();

}

void loop() {
//humidity = dht.readHumidity();
//temperature = dht.readTemperature();
  //Serial.print("Temperature: ");
  //Serial.println(temperature);
  //Serial.print("Humidity: ");
  //Serial.println(humidity);

sendJsonToAWS();
  client.loop();
  delay(1000);



delay(2000);
 
  }



 
