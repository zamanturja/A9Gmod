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

// If your board has only one hardware serial, consider using SoftwareSerial (not recommended for high baud).
// But for demonstration, let's assume we have [HardwareSerial 2 of the ESP32] for the A9G module.

const char* gprsApn = "internet";
const char* gprsUser = "";
const char* gprsPass = "";

const char* mqtt_broker = "PUT_YOUR_BROKER_LINK_HERE";
const char* topic_publish = "topic/publish";
const char* topic_subscribe = "topic/subscribe";

const int RX_PIN = 16;
const int TX_PIN = 17;
const uint32_t TARGET_GSM_BAUD = 115200;

HardwareSerial modemSerial(2);

A9G a9g(true);       // Enable debug logs
A9Gmod a9gmod(a9g);  // High-level MQTT wrapper

// Callback for incoming MQTT messages
void onMQTTMessage(const char* topic, const char* payload) {
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
  modemSerial.begin(TARGET_GSM_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(3000);

  Serial.println("Initializing A9G module...");

  // Initialize the A9G
  bool inited = a9g.init(&modemSerial);
  if (!inited) {
    Serial.println("Failed to initialize A9G module!");
    while (true) { delay(1000); }
  }

  // Optional: Wait until the modem is ready (it prints "READY")
  a9g.waitForModemReady();

  // Check signal quality
  a9g.readSignalQuality();

  // Attach GPRS (for MQTT)
  if (!a9g.attachGPRS(gprsApn, gprsUser, gprsPass)) {
    Serial.println("Failed to attach GPRS!");
  } else {
    // Set APN (if your SIM's APN is something else, change accordingly)
    a9g.setAPN("IP", "internet");
    // Activate PDP context
    a9g.activatePDP();
  }

  // Configure the MQTT server
  a9gmod.setMQTTServer(mqtt_broker, 1883);
  a9gmod.onMQTTMessage(onMQTTMessage);

  // Connect to MQTT
  Serial.println("Connecting to MQTT...");
  bool connected = a9gmod.connectMQTT("A9G_TestClient");
  if (connected) {
    Serial.println("MQTT connected!");
    // Subscribe to a test topic
    a9gmod.subscribeMQTT(topic_subscribe);
    // Publish something
    a9gmod.publishMQTT(topic_publish, "Hello from A9G!");
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
