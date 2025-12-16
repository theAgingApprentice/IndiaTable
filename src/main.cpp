/**
 * @file main.cpp
 * @brief Christmas Tree Project - ESP32 WROOM32 v1.3 (Freenove)
 * @date December 15, 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

// Built-in LED pin (usually GPIO2 on ESP32 dev boards)
#define LED_BUILTIN 2

// MQTT topics
#define TOPIC_CMD "christmasTree-cmd"
#define TOPIC_MSG "christmasTree-msg"
#define TOPIC_LOG "christmasTree-log"

// Timer for LED blinking
hw_timer_t *ledTimer = NULL;
volatile bool ledState = false;
volatile bool mqttConnected = false;

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);
String mqttClientId = "";

/**
 * @brief Log message to both Serial console and MQTT broker
 * @param message Message to log
 */
void logMessage(const String& message) {
  // Always print to serial
  Serial.println(message);
  
  // Also publish to MQTT if connected
  if (mqttConnected && mqttClient.connected()) {
    String prefixedMsg = mqttClientId + ": " + message;
    mqttClient.publish(TOPIC_LOG, prefixedMsg.c_str());
  }
}

/**
 * @brief Log formatted message to both Serial console and MQTT broker
 * @param format Printf-style format string
 */
void logMessageF(const char* format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  // Print to serial
  Serial.println(buffer);
  
  // Also publish to MQTT if connected
  if (mqttConnected && mqttClient.connected()) {
    String prefixedMsg = mqttClientId + ": " + String(buffer);
    mqttClient.publish(TOPIC_LOG, prefixedMsg.c_str());
  }
}

/**
 * @brief Timer interrupt handler for LED blinking
 */
void IRAM_ATTR onLedTimer() {
  if (mqttConnected) {
    // Solid on when MQTT connected
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // Slow blink when WiFi connected but MQTT not connected
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
  }
}

/**
 * @brief MQTT callback for incoming messages
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  logMessageF("[MQTT] Message received on topic: %s", topic);
  logMessageF("[MQTT] Payload: %s", message.c_str());
  
  // Process commands here
  if (String(topic) == TOPIC_CMD) {
    logMessageF("[MQTT] Processing command: %s", message.c_str());
    // Add your command processing logic here
  }
}

/**
 * @brief Connect to MQTT broker
 */
bool connectToMQTT() {
  Serial.println("\n[MQTT] Attempting connection to broker...");
  Serial.printf("[MQTT] Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
  
  // Generate unique client ID
  mqttClientId = "ESP32-ChristmasTree-";
  mqttClientId += String(WiFi.macAddress());
  
  Serial.printf("[MQTT] Client ID: %s\n", mqttClientId.c_str());
  
  if (mqttClient.connect(mqttClientId.c_str())) {
    mqttConnected = true;  // Set this first so logMessage works
    
    logMessage("[MQTT] ✓ Connection successful!");
    
    // Subscribe to command topic
    logMessageF("[MQTT] Subscribing to topic: %s", TOPIC_CMD);
    if (mqttClient.subscribe(TOPIC_CMD)) {
      logMessage("[MQTT] ✓ Subscription successful!");
    } else {
      logMessage("[MQTT] ✗ Subscription failed!");
    }
    
    // Publish connection message
    String connectMsg = "Christmas Tree Device Connected - MAC: " + WiFi.macAddress();
    logMessageF("[MQTT] Publishing to topic: %s", TOPIC_MSG);
    if (mqttClient.publish(TOPIC_MSG, connectMsg.c_str())) {
      logMessage("[MQTT] ✓ Connection message published!");
    } else {
      logMessage("[MQTT] ✗ Failed to publish connection message!");
    }
    
    logMessage("[MQTT] LED set to SOLID (MQTT connected)");
    logMessage("[MQTT] Console messages now mirrored to MQTT topic: christmasTree-log");
    return true;
  } else {
    Serial.printf("[MQTT] ✗ Connection failed! State: %d\n", mqttClient.state());
    mqttConnected = false;
    Serial.println("[MQTT] LED set to SLOW BLINK (MQTT disconnected)");
    return false;
  }
}

/**
 * @brief Scan for WiFi networks and connect to the strongest known network
 * @return true if connection successful, false otherwise
 */
bool connectToStrongestKnownNetwork() {
  Serial.println("\n[WiFi] Starting network scan...");
  
  // Start WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Scan for available networks
  int networkCount = WiFi.scanNetworks();
  Serial.printf("[WiFi] Scan complete. Found %d networks\n", networkCount);
  
  if (networkCount == 0) {
    Serial.println("[WiFi] ERROR: No networks found!");
    return false;
  }
  
  // Display all found networks
  Serial.println("\n[WiFi] Available networks:");
  for (int i = 0; i < networkCount; i++) {
    Serial.printf("  %2d: %-32s | RSSI: %4d dBm | Ch: %2d | %s\n",
                  i + 1,
                  WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i),
                  WiFi.channel(i),
                  WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Encrypted");
  }
  
  // Find the strongest known network
  int bestNetworkIndex = -1;
  int bestRSSI = -1000;
  String bestSSID = "";
  String bestPassword = "";
  
  Serial.println("\n[WiFi] Checking for known networks...");
  
  for (int i = 0; i < networkCount; i++) {
    String scannedSSID = WiFi.SSID(i);
    int scannedRSSI = WiFi.RSSI(i);
    
    // Check if this network is in our known list
    for (int j = 0; j < numKnownNetworks; j++) {
      if (scannedSSID.equals(knownNetworks[j].ssid)) {
        Serial.printf("[WiFi] Found known network: %s (RSSI: %d dBm)\n", 
                      scannedSSID.c_str(), scannedRSSI);
        
        if (scannedRSSI > bestRSSI) {
          bestRSSI = scannedRSSI;
          bestSSID = scannedSSID;
          bestPassword = String(knownNetworks[j].password);
          bestNetworkIndex = i;
        }
      }
    }
  }
  
  // Clean up scan results
  WiFi.scanDelete();
  
  // Check if we found a known network
  if (bestNetworkIndex == -1) {
    Serial.println("[WiFi] ERROR: No known networks found!");
    return false;
  }
  
  // Attempt to connect to the best network
  Serial.printf("\n[WiFi] Connecting to strongest network: %s (RSSI: %d dBm)\n", 
                bestSSID.c_str(), bestRSSI);
  Serial.print("[WiFi] Connection progress: ");
  
  WiFi.begin(bestSSID.c_str(), bestPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" SUCCESS!\n");
    Serial.println("=================================");
    Serial.println("[WiFi] CONNECTION ESTABLISHED");
    Serial.println("=================================");
    Serial.printf("SSID:        %s\n", WiFi.SSID().c_str());
    Serial.printf("IP Address:  %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("Signal:      %d dBm\n", WiFi.RSSI());
    Serial.printf("Channel:     %d\n", WiFi.channel());
    Serial.println("=================================\n");
    return true;
  } else {
    Serial.println(" FAILED!");
    Serial.printf("[WiFi] ERROR: Could not connect to %s\n", bestSSID.c_str());
    return false;
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for serial port to connect
  delay(1000);
  
  Serial.println("\n=================================");
  Serial.println("Christmas Tree Project");
  Serial.println("ESP32-WROOM-32 v1.3 (Freenove)");
  Serial.println("=================================\n");
  
  // Configure the built-in LED pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("[System] Setup initializing...");
  
  // Attempt to connect to WiFi
  if (connectToStrongestKnownNetwork()) {
    // WiFi connection successful - now setup MQTT
    Serial.println("[System] Configuring MQTT client...");
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    // Attempt MQTT connection
    connectToMQTT();
    
    // Start LED status timer
    Serial.println("[System] Starting status LED timer...");
    
    // Configure timer: timer 0, prescaler 80 (1MHz), count up
    ledTimer = timerBegin(0, 80, true);
    
    // Attach interrupt handler
    timerAttachInterrupt(ledTimer, &onLedTimer, true);
    
    // Set timer to trigger every 1000ms (1000000 microseconds) for slow blink
    timerAlarmWrite(ledTimer, 1000000, true);
    
    // Enable the timer
    timerAlarmEnable(ledTimer);
    
    if (mqttConnected) {
      Serial.println("[System] Status LED: SOLID (WiFi + MQTT connected)");
    } else {
      Serial.println("[System] Status LED: SLOW BLINK (WiFi only, MQTT disconnected)");
    }
  } else {
    Serial.println("[System] WiFi connection failed - LED remains off");
  }
  
  Serial.println("\n[System] Setup complete!\n");
}

void loop() {
  // Maintain MQTT connection
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      static bool loggedDisconnect = false;
      if (!loggedDisconnect) {
        // Only log once when connection is lost
        Serial.println("[MQTT] Connection lost. Attempting to reconnect...");
        loggedDisconnect = true;
      }
      mqttConnected = false;
      
      // Attempt to reconnect every 5 seconds
      static unsigned long lastReconnectAttempt = 0;
      unsigned long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        if (connectToMQTT()) {
          loggedDisconnect = false;
        }
      }
    } else {
      // Process MQTT messages
      mqttClient.loop();
    }
  } else {
    logMessage("[WiFi] Connection lost! Attempting to reconnect...");
    connectToStrongestKnownNetwork();
  }
  
  delay(100);
}
