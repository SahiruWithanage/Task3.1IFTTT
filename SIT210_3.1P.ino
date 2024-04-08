#include <WiFiNINA.h>
#include <BH1750FVI.h>
#include "secrets.h"

//Credentials for the WiFi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS; 

WiFiSSLClient client;
BH1750FVI lightSensor(BH1750FVI::k_DevModeContLowRes);

//Two different webhooks to send emails according to the light condition
String sunHit = "/hooks/catch/18488412/3pgwn1b";
String sunStop = "/hooks/catch/18488412/3pgg28y";
bool sunHitSet = false;
bool sunStopSet = false;

//Light level threshold which considers as the sunlight is available above.
const int lightThreshold = 500; 

void setup() {
  Serial.begin(9600);
  while (!Serial)

  lightSensor.begin();

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);

  int retries = 0;  // Track connection attempts
  const int maxRetries = 5;  // Maximum retries before giving up

  //Connects to the WiFi
  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(5000);
    retries++;
  }

  //Checks the connectivity status and reports
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected.");
  } else {
    Serial.println("\nConnection failed after retries.");
  }
}

//Method to make a GET request to the webhook
void ping(String webhook) {
  Serial.println("------------------------------------");
  Serial.println("Pinging hooks.zapier.com...");

  if (client.connect("hooks.zapier.com", 443)) {
    Serial.println("Connected to server.");
    client.println("GET " + webhook + " HTTP/1.0");
    client.println("Host: hooks.zapier.com");
    client.println("Connection: close");
    client.println();
    Serial.println("Request sent.");
  }

  //Checks whether the server responded or not
  boolean waiting = true;
  while (waiting && client.connected()) {
    char c = client.read();
    if (c == '\n') {
      waiting = false;
      Serial.println("server responded");
    }
  }

  //Closes the connection to the server
  Serial.println("\nClosing connection.");
  Serial.println("------------------------------------");
  client.stop();
}

void loop() {
  float lux = lightSensor.GetLightIntensity();
  
  //Checks whether the sunlight is available or not and acts according to it
  if(lux > lightThreshold) {
    if(!sunHitSet) {
      ping(sunHit);
      sunHitSet = true;
      sunStopSet = false;
    }
  }
  else {
    if(!sunStopSet) {
      ping(sunStop);
      sunStopSet = true;
      sunHitSet = false;
    }
  }
}