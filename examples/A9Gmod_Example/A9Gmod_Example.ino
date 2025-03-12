/*
  @Zaman Turja

  Basic demonstration of the A9Gmod library.

  Steps:
  1) Initialize module
  2) Check signal quality
  3) Attach GPRS, set APN
  4) Connect to MQTT
  5) Publish a sample message
  6) Wait for inbound MQTT messages
*/

#include <Arduino.h>
#include "A9Gmod.h"  // Include your library's main header

// For boards with multiple hardware serials, you might use Serial1.
// If your board has only one hardware serial, consider using SoftwareSerial (not recommended for high baud).
// Example: SoftwareSerial modemSerial(2,3); // RX, TX
// But for demonstration, let's assume we have Serial1 for the A9G module.
#define MODEM_SERIAL Serial1

A9G a9g(true);       // Enable debug logs
A9Gmod a9gmod(a9g);  // High-level MQTT wrapper

// Callback for incoming MQTT messages
void onMQTTMessage(const char *topic, const char *payload) {
  Serial.print("MQTT Message arrived:\n  Topic: ");
  Serial.println(topic);
  Serial.print("  Payload: ");
  Serial.println(payload);
}

void setup() {
  // Start our main Serial for debug logs
  Serial.begin(115200);
  delay(1000);

  // Start modem serial
  MODEM_SERIAL.begin(115200);
  delay(3000);

  Serial.println("Initializing A9G module...");

  // Initialize the A9G
  bool inited = a9g.init(&MODEM_SERIAL);
  if (!inited) {
    Serial.println("Failed to initialize A9G module!");
    while (true) { delay(1000); }
  }

  // Optional: Wait until the modem is ready (it prints "READY")
  // a9g.waitForModemReady();

  // Check signal quality
  a9g.readSignalQuality();

  // Attach GPRS (for MQTT)
  if (!a9g.attachGPRS()) {
    Serial.println("Failed to attach GPRS!");
  } else {
    // Set APN (if your SIM's APN is something else, change accordingly)
    a9g.setAPN("IP", "internet");
    // Activate PDP context
    a9g.activatePDP();
  }

  // Configure the MQTT server
  a9gmod.setMQTTServer("test.mosquitto.org", 1883);
  a9gmod.onMQTTMessage(onMQTTMessage);

  // Connect to MQTT
  Serial.println("Connecting to MQTT...");
  bool connected = a9gmod.connectMQTT("A9G_TestClient");
  if (connected) {
    Serial.println("MQTT connected!");
    // Subscribe to a test topic
    a9gmod.subscribeMQTT("A9Gmod/test");
    // Publish something
    a9gmod.publishMQTT("A9Gmod/test", "Hello from A9G!");
  } else {
    Serial.println("Failed to connect to MQTT broker!");
  }
}

void loop() {
  // Keep the library polling for inbound data
  a9gmod.processMQTT();

  // Example of sending an SMS (uncomment if needed):
  // a9g.sendSMS("+1234567890", "Hello from A9G!");

  // Add any custom logic here...
  delay(1000);
}
