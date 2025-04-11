#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// WiFi Credentials
const char* ssid = "SURYANSH.CSC 2.4G";
const char* password = "Divya@1985";

// ThingSpeak API Details
const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "W739X7G8TT127GLZ";
const int channelID = 2430548;

// Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define DHT_PIN 4
#define PIR_PIN 13
#define RED_LED 12
#define YELLOW_LED 14
#define GREEN_LED 27

// DHT11 Sensor
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
    Serial.begin(9600); // Changed baud rate for better compatibility
    delay(1000);
    dht.begin();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(PIR_PIN, INPUT_PULLDOWN); // Added internal pull-down
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) { // 20 attempts (20 sec timeout)
        delay(1000);
        Serial.print(".");
        Serial.println(WiFi.status()); // Debugging WiFi status
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
    } else {
        Serial.println("\nWiFi connection failed! Check SSID & Password.");
    }
    
    Serial.print("ThingSpeak Channel ID: ");
    Serial.println(channelID);
}

// Function to get distance from Ultrasonic Sensor
float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    return (duration * 0.0343) / 2; // Convert to cm
}

void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    delay(100); // Allow PIR sensor to settle
    int pirValue = digitalRead(PIR_PIN);
    Serial.print("PIR Value: ");
    Serial.println(pirValue);
    
    float distance = getDistance();
    
    // Assuming bin height is 30 cm, calculate fill level
    int fillLevel = map(distance, 30, 0, 0, 100);
    fillLevel = constrain(fillLevel, 0, 100);
    
    // LED Indications
    digitalWrite(GREEN_LED, fillLevel <= 30);
    digitalWrite(YELLOW_LED, fillLevel > 30 && fillLevel <= 70);
    digitalWrite(RED_LED, fillLevel > 70);

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(server);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String postData = "api_key=" + String(apiKey) + "&field1=" + String(temperature) + 
                          "&field2=" + String(humidity) + "&field3=" + String(fillLevel) + 
                          "&field4=" + String(pirValue);

        int httpResponseCode = http.POST(postData);
        if (httpResponseCode > 0) {
            Serial.println("Data sent successfully to ThingSpeak: " + String(httpResponseCode));
        } else {
            Serial.println("Error sending data: " + http.errorToString(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected! Reconnecting...");
        WiFi.begin(ssid, password);
    }

    delay(15000); // Send data every 15 seconds
}