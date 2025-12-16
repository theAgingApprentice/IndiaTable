/**
 * @file main.cpp
 * @brief Christmas Tree Project - ESP32 WROOM32 v1.3 (Freenove)
 * @date December 15, 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

// Built-in LED pin (usually GPIO2 on ESP32 dev boards)
#define LED_BUILTIN 2

// Timer for LED blinking
hw_timer_t *ledTimer = NULL;
volatile bool ledState = false;

/**
 * @brief Timer interrupt handler for LED blinking
 */
void IRAM_ATTR onLedTimer() {
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
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
    // Connection successful - start LED blinking timer
    Serial.println("[System] Starting status LED (solid green blink)...");
    
    // Configure timer: timer 0, prescaler 80 (1MHz), count up
    ledTimer = timerBegin(0, 80, true);
    
    // Attach interrupt handler
    timerAttachInterrupt(ledTimer, &onLedTimer, true);
    
    // Set timer to trigger every 500ms (500000 microseconds) for blinking
    timerAlarmWrite(ledTimer, 500000, true);
    
    // Enable the timer
    timerAlarmEnable(ledTimer);
    
    Serial.println("[System] Status LED active - indicating connected state");
  } else {
    Serial.println("[System] WiFi connection failed - LED remains off");
  }
  
  Serial.println("\n[System] Setup complete!\n");
}

void loop() {
  // Main loop can remain empty or handle other tasks
  // LED is controlled by timer interrupt
  delay(1000);
}
