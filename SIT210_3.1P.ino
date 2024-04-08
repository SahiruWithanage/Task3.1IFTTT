#include <WiFiNINA.h>
#include <BH1750FVI.h>
#include "secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS; 

WiFiSSLClient client;
BH1750FVI lightSensor(BH1750FVI::k_DevModeContLowRes);

String sunHit = "/hooks/catch/18488412/3pgwn1b";
String sunStop = "/hooks/catch/18488412/3pgg28y";
bool sunHitSet = false;
bool sunStopSet = false;

const int lightThreshold = 500; 

void setup() {
  Serial.begin(9600);
  while (!Serial)

  lightSensor.begin();

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);

  int retries = 0;  // Track connection attempts
  const int maxRetries = 5;  // Maximum retries before giving up

  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(5000);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected.");
  } else {
    Serial.println("\nConnection failed after retries.");
  }
/*
  while(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(5000);     
  } 
  Serial.println("\nConnected.");*/
}

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

  boolean waiting = true;
  while (waiting && client.connected()) {
    char c = client.read();
    if (c == '\n') {
      waiting = false;
      Serial.println("server responded");
    }
  }

  Serial.println("\nClosing connection.");
  Serial.println("------------------------------------");
  client.stop();
}

void loop() {
  float lux = lightSensor.GetLightIntensity();
  
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