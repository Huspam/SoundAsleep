#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "SparkFunLSM6DSO.h"
#include <iostream>
#include <cmath>

using namespace std;

// Accelerometer vars
LSM6DSO myIMU;
const float threshold = 0.3;
bool forward = false;
float X;
float Y;
float Z;

// Wifi vars
char ssid[50] = "HusamPhone"; // your network SSID (name)
char pass[50] = "iHusam0103"; // your network password (use for WPA, or use as key for WEP)
const int kNetworkTimeout = 30 * 1000; // Number of milliseconds to wait without receiving any data before we give up
const int kNetworkDelay = 1000; // Number of milliseconds to wait if no data is available before trying again
double move_degree = 0;
int move_int = 0;
double large_movement = 0;

void Wifi_Scan();
void Wifi_Connect();
double distance_3d(double x1, double y1, double z1, double x2, double y2, double z2);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  myIMU.begin();
  myIMU.initialize(BASIC_SETTINGS);

  // First coordinate
  X = myIMU.readFloatAccelX();
  Y = myIMU.readFloatAccelY();
  Z = myIMU.readFloatAccelZ();
  
  // Scan for WiFi
  Wifi_Scan();

  // Connect to a WiFi network
  Wifi_Connect();
}

void loop() {
  float nX = myIMU.readFloatAccelX();
  float nY = myIMU.readFloatAccelY();
  float nZ = myIMU.readFloatAccelZ();

  // check movement
  move_degree = distance_3d(X, Y, Z, nX, nY, nZ);
  Serial.println(move_degree);
  if (move_degree > threshold) {
    Serial.println(move_degree);
    if (move_degree > 2){
      move_degree = 2;
    }
    move_int = int((move_degree/2) * 100);
    Serial.println(move_int);

    int err = 0;
    WiFiClient c;
    HttpClient http(c);

    String query = "/?movement=" + String(move_int);
    err = http.get("13.52.74.129", 5000, query.c_str(), NULL);
    
    if (err == 0) {
      Serial.println("startedRequest ok");
      err = http.responseStatusCode();
      if (err >= 0) {
        Serial.print("Got status code: ");
        Serial.println(err);
        // Usually you'd check that the response code is 200 or a
        // similar "success" code (200-299) before carrying on,
        // but we'll print out whatever response we get
        err = http.skipResponseHeaders();
        if (err >= 0) {
          int bodyLen = http.contentLength();
          Serial.print("Content length is: ");
          Serial.println(bodyLen);
          Serial.println();
          Serial.println("Body returned follows:");

          // Now we've got to the body, so we can print it out
          unsigned long timeoutStart = millis();
          char c;

          // Whilst we haven't timed out & haven't reached the end of the body
          while ((http.connected() || http.available()) &&
          ((millis() - timeoutStart) < kNetworkTimeout)) {
            if (http.available()) {
              c = http.read();

              // Print out this character
              Serial.print(c);
              bodyLen--;

              // We read something, reset the timeout counter
              timeoutStart = millis();
            } else {
              // We haven't got any data, so let's pause to allow some to
              // arrive
              delay(kNetworkDelay);
            }
          }
        } else {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }
      } else {
        Serial.print("Getting response failed: ");
        Serial.println(err);
      }
    } else {
      Serial.print("Connect failed: ");
      Serial.println(err);
    }
  }  

  // update new coordinates
  X = nX; Y = nY; Z = nZ;

  delay(2000);
}


void Wifi_Scan() {
  Serial.println("Scanning for available networks...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found:");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }

}

void Wifi_Connect() {
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println(WL_CONNECTED);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

}

double distance_3d(double x1, double y1, double z1, double x2, double y2, double z2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}