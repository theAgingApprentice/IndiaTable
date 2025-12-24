/**
 * @file main.cpp
 * @brief Christmas Tree Project - ESP32 WROOM32 v1.3 (Freenove)
 * @date December 15, 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WebServer.h>
#include "secrets.h"

// Built-in LED pin (usually GPIO2 on ESP32 dev boards)
#define LED_BUILTIN 2

// WS2812B LED Strip Configuration
#define LED_PIN 33
#define NUM_LEDS 900
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Power management - limit current draw
#define MAX_BRIGHTNESS 80  // Optimized for 5V 4A power supply (0-255)

// LED array
CRGB leds[NUM_LEDS];

// Firmware version
#define FIRMWARE_VERSION "8.0.5"

// MQTT topics
#define TOPIC_CMD "christmasTree-cmd"
#define TOPIC_MSG "christmasTree-msg"
#define TOPIC_LOG "christmasTree-log"

// Timer for LED blinking
hw_timer_t *ledTimer = NULL;
volatile bool ledState = false;
volatile bool mqttConnected = false;

// LED strip blink control
bool blinkEnabled = false;
bool blinkState = false;
unsigned long blinkSpeed = 500;  // Blink interval in milliseconds (default 500ms)
unsigned long lastBlinkTime = 0;
CRGB blinkColor = CRGB::Red;  // Current blink color

// Twinkle effect control
bool twinkleEnabled = false;
unsigned long lastTwinkleUpdate = 0;
const int TWINKLE_UPDATE_INTERVAL = 50;  // Update every 50ms for smooth effect
const int TWINKLE_LEDS_PER_UPDATE = 5;   // Number of LEDs to update each cycle

// Twinkle Plus effect control (more aggressive)
bool twinklePlusEnabled = false;
unsigned long lastTwinklePlusUpdate = 0;
const int TWINKLEPLUS_UPDATE_INTERVAL = 30;  // Faster updates for aggressive effect
const int TWINKLEPLUS_LEDS_PER_UPDATE = 15;  // More LEDs per update

// Gold effect control
bool goldEnabled = false;
unsigned long lastGoldUpdate = 0;
const int GOLD_UPDATE_INTERVAL = 30;  // Same as twinkle+ for matching twinkle rate
const int GOLD_LEDS_PER_UPDATE = 15;  // Same as twinkle+

// Vegas effect control
bool vegasEnabled = false;
unsigned long lastVegasUpdate = 0;
const int VEGAS_UPDATE_INTERVAL = 30;    // Fast updates for wild effect
uint8_t vegasHue = 0;                    // Rainbow hue tracker

// Valentines effect control
bool valentinesEnabled = false;
unsigned long lastValentinesUpdate = 0;
const int VALENTINES_UPDATE_INTERVAL = 40;  // Smooth romantic animation
uint8_t valentinesPhase = 0;                // Animation phase tracker

// St. Patrick's effect control
bool stPatricksEnabled = false;
unsigned long lastStPatricksUpdate = 0;
const int STPATRICKS_UPDATE_INTERVAL = 45;  // Smooth Irish animation
uint8_t stPatricksPhase = 0;                // Animation phase tracker

// Halloween effect control
bool halloweenEnabled = false;
unsigned long lastHalloweenUpdate = 0;
const int HALLOWEEN_UPDATE_INTERVAL = 35;   // Spooky animation timing
uint8_t halloweenPhase = 0;                 // Animation phase tracker

// Christmas effect control
bool christmasEnabled = false;
unsigned long lastChristmasUpdate = 0;
const int CHRISTMAS_UPDATE_INTERVAL = 40;   // Festive animation timing
uint8_t christmasPhase = 0;                 // Animation phase tracker

// Birthday effect control
bool birthdayEnabled = false;
unsigned long lastBirthdayUpdate = 0;
const int BIRTHDAY_UPDATE_INTERVAL = 35;    // Party animation timing
uint8_t birthdayPhase = 0;                  // Animation phase tracker

// Wild Christmas effect control
bool wildChristmasEnabled = false;
unsigned long lastWildChristmasUpdate = 0;
const int WILDCHRISTMAS_UPDATE_INTERVAL = 25;  // Fast chaotic timing
uint8_t wildChristmasPhase = 0;                // Animation phase tracker

// Christmas Basic effect control
bool christmasBasicEnabled = false;
unsigned long lastChristmasBasicUpdate = 0;
const int CHRISTMASBASIC_UPDATE_INTERVAL = 50;  // Twinkle update timing

// Christmas Train effect control
bool christmasTrainEnabled = false;
unsigned long lastChristmasTrainUpdate = 0;
unsigned long christmasTrainSpeed = 100;        // Rotation speed in ms (adjustable)
int christmasTrainOffset = 0;                   // Current rotation offset

// Rainbow effect control
bool rainbowEnabled = false;
unsigned long lastRainbowUpdate = 0;
const int RAINBOW_UPDATE_INTERVAL = 30;     // Smooth rainbow timing
uint8_t rainbowPhase = 0;                   // Animation phase tracker

// May The 4th effect control (Star Wars Day)
bool mayThe4thEnabled = false;
unsigned long lastMayThe4thUpdate = 0;
const int MAYTHE4TH_UPDATE_INTERVAL = 35;   // Epic space saga timing
uint8_t mayThe4thPhase = 0;                 // Animation phase tracker

// Canada Day effect control
bool canadaDayEnabled = false;
unsigned long lastCanadaDayUpdate = 0;
const int CANADADAY_UPDATE_INTERVAL = 40;   // Proud Canadian timing
uint8_t canadaDayPhase = 0;                 // Animation phase tracker

// New Years effect control
bool newYearsEnabled = false;
unsigned long lastNewYearsUpdate = 0;
const int NEWYEARS_UPDATE_INTERVAL = 35;    // Celebration timing
uint8_t newYearsPhase = 0;                  // Animation phase tracker

// Candy Cane effect control
bool candyCaneEnabled = false;
unsigned long lastCandyCaneUpdate = 0;
const int CANDYCANE_UPDATE_INTERVAL = 40;   // Stripe animation timing
uint8_t candyCanePhase = 0;                 // Animation phase tracker

// Serene effect control
bool sereneEnabled = false;
unsigned long lastSereneUpdate = 0;
const int SERENE_UPDATE_INTERVAL = 40;      // ~25 FPS smooth animation

// Command queue to avoid watchdog issues in MQTT callback
String pendingCommand = "";
unsigned long pendingCommandParam = 0;
String unknownCommand = "";  // Track unknown commands for logging

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);
String mqttClientId = "";

// Web Server on port 80
WebServer webServer(80);

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
 * @brief Clear all effect flags and LED strip
 * This ensures clean state transitions when switching between effects
 */
void clearAllEffects() {
  blinkEnabled = false;
  twinkleEnabled = false;
  twinklePlusEnabled = false;
  goldEnabled = false;
  vegasEnabled = false;
  valentinesEnabled = false;
  stPatricksEnabled = false;
  halloweenEnabled = false;
  christmasEnabled = false;
  birthdayEnabled = false;
  wildChristmasEnabled = false;
  christmasBasicEnabled = false;
  christmasTrainEnabled = false;
  rainbowEnabled = false;
  mayThe4thEnabled = false;
  canadaDayEnabled = false;
  newYearsEnabled = false;
  candyCaneEnabled = false;
  sereneEnabled = false;
  
  // Clear the LED strip to prevent artifacts
  FastLED.clear();
  FastLED.show();
}

/**
 * @brief Turn off all LEDs in the strip
 */
void turnOffAllLEDs() {
  clearAllEffects();
}

/**
 * @brief Set all LEDs to red
 */
void allRed() {
  clearAllEffects();
  
  // Use fill_solid for better performance
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  
  yield();  // Feed the watchdog
  FastLED.show();
  yield();  // Feed the watchdog again after show
  
  Serial.println("[LED Strip] All LEDs set to RED");
}

/**
 * @brief Set all LEDs to green
 */
void allGreen() {
  clearAllEffects();
  
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  
  yield();
  FastLED.show();
  yield();
  
  Serial.println("[LED Strip] All LEDs set to GREEN");
}

/**
 * @brief Set all LEDs to white
 */
void allWhite() {
  clearAllEffects();
  
  fill_solid(leds, NUM_LEDS, CRGB::White);
  
  yield();
  FastLED.show();
  yield();
  
  Serial.println("[LED Strip] All LEDs set to WHITE");
}

/**
 * @brief Set all LEDs to blue
 */
void allBlue() {
  clearAllEffects();
  
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  
  yield();
  FastLED.show();
  yield();
  
  Serial.println("[LED Strip] All LEDs set to BLUE");
}

/**
 * @brief Enable red blinking on all LEDs
 */
void allRedBlink() {
  clearAllEffects();
  blinkEnabled = true;
  blinkState = false;
  blinkColor = CRGB::Red;
  lastBlinkTime = millis();
  Serial.printf("[LED Strip] Red blink enabled (speed: %lu ms)\n", blinkSpeed);
}

/**
 * @brief Enable green blinking on all LEDs
 */
void allGreenBlink() {
  clearAllEffects();
  blinkEnabled = true;
  blinkState = false;
  blinkColor = CRGB::Green;
  lastBlinkTime = millis();
  Serial.printf("[LED Strip] Green blink enabled (speed: %lu ms)\n", blinkSpeed);
}

/**
 * @brief Enable white blinking on all LEDs
 */
void allWhiteBlink() {
  clearAllEffects();
  blinkEnabled = true;
  blinkState = false;
  blinkColor = CRGB::White;
  lastBlinkTime = millis();
  Serial.printf("[LED Strip] White blink enabled (speed: %lu ms)\n", blinkSpeed);
}

/**
 * @brief Enable blue blinking on all LEDs
 */
void allBlueBlink() {
  clearAllEffects();
  blinkEnabled = true;
  blinkState = false;
  blinkColor = CRGB::Blue;
  lastBlinkTime = millis();
  Serial.printf("[LED Strip] Blue blink enabled (speed: %lu ms)\n", blinkSpeed);
}

/**
 * @brief Enable magical twinkle effect
 */
void twinkle() {
  clearAllEffects();
  twinkleEnabled = true;
  lastTwinkleUpdate = millis();
  
  // Start with all LEDs off
  FastLED.clear();
  FastLED.show();
  
  Serial.println("[LED Strip] Twinkle effect enabled - magical mode");
}

/**
 * @brief Enable aggressive twinkle+ effect - faster and more intense twinkling
 */
void twinklePlus() {
  clearAllEffects();
  twinklePlusEnabled = true;
  lastTwinklePlusUpdate = millis();
  
  // Start with all LEDs off
  FastLED.clear();
  FastLED.show();
  
  Serial.println("[LED Strip] Twinkle+ effect enabled - aggressive magical mode!");
}

/**
 * @brief Enable gold effect - golden LEDs with twinkling
 */
void gold() {
  clearAllEffects();
  goldEnabled = true;
  lastGoldUpdate = millis();
  
  // Start with all LEDs as gold
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 180, 0);  // Gold color
  }
  FastLED.show();
  
  Serial.println("[LED Strip] Gold effect enabled - shimmering gold!");
}

/**
 * @brief Enable wild Vegas effect - crazy colors and patterns
 */
void vegas() {
  clearAllEffects();
  vegasEnabled = true;
  lastVegasUpdate = millis();
  vegasHue = 0;
  
  Serial.println("[LED Strip] VEGAS mode enabled - let's get WILD!");
}

/**
 * @brief Enable romantic Valentines effect - pink and red love
 */
void valentines() {
  clearAllEffects();
  valentinesEnabled = true;
  lastValentinesUpdate = millis();
  valentinesPhase = 0;
  
  Serial.println("[LED Strip] Valentine's mode enabled - spread the love!");
}

/**
 * @brief Enable St. Patrick's Day effect - Irish green and gold
 */
void stPatricks() {
  clearAllEffects();
  stPatricksEnabled = true;
  lastStPatricksUpdate = millis();
  stPatricksPhase = 0;
  
  Serial.println("[LED Strip] St. Patrick's mode enabled - Irish luck!");
}

/**
 * @brief Enable Halloween effect - spooky orange, purple, and green
 */
void halloween() {
  clearAllEffects();
  halloweenEnabled = true;
  lastHalloweenUpdate = millis();
  halloweenPhase = 0;
  
  Serial.println("[LED Strip] Halloween mode enabled - spooky time!");
}

/**
 * @brief Enable Christmas effect - festive red, green, white, and gold
 */
void christmas() {
  clearAllEffects();
  christmasEnabled = true;
  lastChristmasUpdate = millis();
  christmasPhase = 0;
  
  Serial.println("[LED Strip] Christmas mode enabled - ho ho ho!");
}

/**
 * @brief Enable Birthday effect - colorful celebration with confetti and candles
 */
void birthday() {
  clearAllEffects();
  birthdayEnabled = true;
  lastBirthdayUpdate = millis();
  birthdayPhase = 0;
  
  Serial.println("[LED Strip] Birthday mode enabled - happy birthday!");
}

/**
 * @brief Enable Wild Christmas effect - fast chaotic Christmas party mode
 */
void wildChristmas() {
  clearAllEffects();
  wildChristmasEnabled = true;
  lastWildChristmasUpdate = millis();
  wildChristmasPhase = 0;
  
  Serial.println("[LED Strip] Wild Christmas mode enabled - crazy festive!");
}

/**
 * @brief Enable Christmas Basic effect - alternating red, green, white with twinkling
 */
void christmasBasic() {
  clearAllEffects();
  christmasBasicEnabled = true;
  lastChristmasBasicUpdate = millis();
  
  // Set initial pattern - red, green, white repeating
  for (int i = 0; i < NUM_LEDS; i++) {
    int colorIndex = i % 3;
    if (colorIndex == 0) {
      leds[i] = CRGB::Red;
    } else if (colorIndex == 1) {
      leds[i] = CRGB::Green;
    } else {
      leds[i] = CRGB::White;
    }
  }
  FastLED.show();
  
  Serial.println("[LED Strip] Christmas Basic mode enabled - red, green, white with twinkling!");
}

/**
 * @brief Enable Christmas Train effect - rotating red, green, white pattern
 */
void christmasTrain() {
  clearAllEffects();
  christmasTrainEnabled = true;
  lastChristmasTrainUpdate = millis();
  christmasTrainOffset = 0;
  
  // Set initial pattern - red, green, white repeating
  for (int i = 0; i < NUM_LEDS; i++) {
    int colorIndex = i % 3;
    if (colorIndex == 0) {
      leds[i] = CRGB::Red;
    } else if (colorIndex == 1) {
      leds[i] = CRGB::Green;
    } else {
      leds[i] = CRGB::White;
    }
  }
  FastLED.show();
  
  Serial.printf("[LED Strip] Christmas Train mode enabled - motion at %lu ms speed!\n", christmasTrainSpeed);
}

/**
 * @brief Set the Christmas Train rotation speed
 * @param speed Speed in milliseconds (50-1000ms)
 */
void setTrainSpeed(unsigned long speed) {
  // Validate speed range
  if (speed < 50) {
    speed = 50;  // Minimum speed
    logMessage("[LED Strip] Train speed set to minimum: 50ms");
  } else if (speed > 1000) {
    speed = 1000;  // Maximum speed
    logMessage("[LED Strip] Train speed set to maximum: 1000ms");
  }
  
  christmasTrainSpeed = speed;
  
  char msg[100];
  snprintf(msg, sizeof(msg), "[LED Strip] Christmas Train speed set to %lu ms (lower=faster, higher=slower)", christmasTrainSpeed);
  logMessage(msg);
  
  // If train effect is running, show immediate feedback
  if (christmasTrainEnabled) {
    logMessage("[LED Strip] Speed change will take effect immediately!");
  }
}

/**
 * @brief Enable Rainbow effect - smooth spectrum animations
 */
void rainbow() {
  clearAllEffects();
  rainbowEnabled = true;
  lastRainbowUpdate = millis();
  rainbowPhase = 0;
  
  Serial.println("[LED Strip] Rainbow mode enabled - full spectrum!");
}

/**
 * @brief Enable May The 4th effect - Star Wars themed animations
 */
void mayThe4th() {
  clearAllEffects();
  mayThe4thEnabled = true;
  lastMayThe4thUpdate = millis();
  mayThe4thPhase = 0;
  
  Serial.println("[LED Strip] May The 4th mode enabled - may the force be with you!");
}

/**
 * @brief Enable Canada Day effect - red and white patriotic animations
 */
void canadaDay() {
  clearAllEffects();
  canadaDayEnabled = true;
  lastCanadaDayUpdate = millis();
  canadaDayPhase = 0;
  
  Serial.println("[LED Strip] Canada Day mode enabled - oh Canada!");
}

/**
 * @brief Enable New Years effect - gold, silver, and colorful celebration
 */
void newYears() {
  clearAllEffects();
  newYearsEnabled = true;
  lastNewYearsUpdate = millis();
  newYearsPhase = 0;
  
  Serial.println("[LED Strip] New Years mode enabled - happy new year!");
}

/**
 * @brief Enable Candy Cane effect - red and white stripes
 */
void candyCane() {
  clearAllEffects();
  candyCaneEnabled = true;
  lastCandyCaneUpdate = millis();
  candyCanePhase = 0;
  
  Serial.println("[LED Strip] Candy Cane mode enabled - sweet stripes!");
}

/**
 * @brief Enable serene sparkle effect - gentle Christmas palette sparkles
 */
void serene() {
  clearAllEffects();
  sereneEnabled = true;
  lastSereneUpdate = millis();
  
  // Start with all LEDs off for clean sparkle effect
  FastLED.clear();
  FastLED.show();
  
  Serial.println("[LED Strip] Serene effect enabled - peaceful sparkles!");
}

/**
 * @brief Set blink speed
 * @param speed Blink interval in milliseconds
 */
void setSpeed(unsigned long speed) {
  if (speed < 50) speed = 50;  // Minimum 50ms
  if (speed > 5000) speed = 5000;  // Maximum 5000ms
  blinkSpeed = speed;
  Serial.printf("[LED Strip] Blink speed set to %lu ms\n", blinkSpeed);
}

/**
 * @brief Show help information - list all available commands
 */
void showHelp() {
  logMessage("\n=================================");
  logMessage("  Available MQTT Commands");
  logMessage("=================================");
  logMessage("Status:");
  logMessage("  showStatus - Display WiFi/MQTT status on LEDs 0-1");
  logMessage("");
  logMessage("Solid Colors:");
  logMessage("  allRed     - Set all LEDs to red");
  logMessage("  allGreen   - Set all LEDs to green");
  logMessage("  allWhite   - Set all LEDs to white");
  logMessage("  allBlue    - Set all LEDs to blue");
  logMessage("");
  logMessage("Blinking Colors:");
  logMessage("  allRedBlink   - Blink all LEDs red");
  logMessage("  allGreenBlink - Blink all LEDs green");
  logMessage("  allWhiteBlink - Blink all LEDs white");
  logMessage("  allBlueBlink  - Blink all LEDs blue");
  logMessage("");
  logMessage("Special Effects:");
  logMessage("  twinkle    - Magical twinkling effect");
  logMessage("  twinkle+   - Aggressive fast twinkling effect");
  logMessage("  gold       - Shimmering gold twinkling effect");
  logMessage("  vegas      - Wild and crazy Las Vegas mode!");
  logMessage("  valentines - Romantic pink and red love theme");
  logMessage("  stPatricks - Irish green and gold shamrock luck");
  logMessage("  halloween  - Spooky orange, purple, and green");
  logMessage("  christmas  - Festive red, green, white, and gold");
  logMessage("  christmasBasic - Classic red, green, white with twinkling");
  logMessage("  christmasTrain - Rotating red, green, white motion");
  logMessage("  birthday   - Colorful celebration with confetti and candles");
  logMessage("  wildChristmas - Fast chaotic Christmas party mode");
  logMessage("  rainbow    - Smooth spectrum animations");
  logMessage("  mayThe4th  - Star Wars themed animations (May the 4th)");
  logMessage("  canadaDay  - Red and white patriotic Canadian celebration");
  logMessage("  newYears   - Gold, silver, and colorful New Year's celebration");
  logMessage("  candyCane  - Red and white candy cane stripes");
  logMessage("  serene     - Peaceful Christmas sparkles with gentle fading");
  logMessage("");
  logMessage("Configuration:");
  logMessage("  setSpeed:<ms>      - Set blink speed (50-5000ms)");
  logMessage("                       Example: setSpeed:500");
  logMessage("  setTrainSpeed:<ms> - Set train rotation speed (50-1000ms)");
  logMessage("                       Example: setTrainSpeed:150");
  logMessage("");
  logMessage("Information:");
  logMessage("  help - Show this help message");
  logMessage("=================================\n");
}

/**
 * @brief Clear all LED effects - helper function for clean state transitions
 */
/**
 * @brief Show connection status on first two LEDs
 * LED 0: Green = WiFi connected, Red = WiFi disconnected
 * LED 1: Green = MQTT connected, Red = MQTT disconnected
 */
void showStatus() {
  // Disable all effects first
  clearAllEffects();
  
  // Check WiFi status and set LED 0
  if (WiFi.status() == WL_CONNECTED) {
    leds[0] = CRGB::Green;
    Serial.println("[LED Strip] WiFi connected - LED 0 set to GREEN");
  } else {
    leds[0] = CRGB::Red;
    Serial.println("[LED Strip] WiFi disconnected - LED 0 set to RED");
  }
  
  // Check MQTT status and set LED 1
  if (mqttConnected) {
    leds[1] = CRGB::Green;
    Serial.println("[LED Strip] MQTT connected - LED 1 set to GREEN");
  } else {
    leds[1] = CRGB::Red;
    Serial.println("[LED Strip] MQTT disconnected - LED 1 set to RED");
  }
  
  // Update physical LEDs
  FastLED.show();
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
  
  // Trim whitespace and newlines
  message.trim();
  
  // Convert topic to String BEFORE using in logMessageF
  String topicStr = String(topic);
  
  // Use Serial only in callback to avoid re-entrancy issues with MQTT
  Serial.printf("[MQTT] Message received on topic: %s\n", topic);
  Serial.printf("[MQTT] Payload: %s\n", message.c_str());
  Serial.printf("[MQTT] Message length: %d\n", message.length());
  
  // Process commands here
  if (topicStr == String(TOPIC_CMD)) {
    Serial.printf("[MQTT] Queuing command: %s\n", message.c_str());
    
    if (message == "showStatus") {
      pendingCommand = "showStatus";
    }
    else if (message == "help") {
      pendingCommand = "help";
    }
    else if (message == "allRed") {
      pendingCommand = "allRed";
    }
    else if (message == "allRedBlink") {
      pendingCommand = "allRedBlink";
    }
    else if (message == "allGreen") {
      pendingCommand = "allGreen";
    }
    else if (message == "allGreenBlink") {
      pendingCommand = "allGreenBlink";
    }
    else if (message == "allWhite") {
      pendingCommand = "allWhite";
    }
    else if (message == "allWhiteBlink") {
      pendingCommand = "allWhiteBlink";
    }
    else if (message == "allBlue") {
      pendingCommand = "allBlue";
    }
    else if (message == "allBlueBlink") {
      pendingCommand = "allBlueBlink";
    }
    else if (message == "twinkle") {
      pendingCommand = "twinkle";
    }
    else if (message == "twinkle+") {
      pendingCommand = "twinkle+";
    }
    else if (message == "gold") {
      pendingCommand = "gold";
    }
    else if (message == "vegas") {
      pendingCommand = "vegas";
    }
    else if (message == "valentines") {
      pendingCommand = "valentines";
    }
    else if (message == "stPatricks") {
      pendingCommand = "stPatricks";
    }
    else if (message == "halloween") {
      pendingCommand = "halloween";
    }
    else if (message == "christmas") {
      pendingCommand = "christmas";
    }
    else if (message == "birthday") {
      pendingCommand = "birthday";
    }
    else if (message == "wildChristmas") {
      pendingCommand = "wildChristmas";
    }
    else if (message == "christmasBasic") {
      pendingCommand = "christmasBasic";
    }
    else if (message == "christmasTrain") {
      pendingCommand = "christmasTrain";
    }
    else if (message == "rainbow") {
      pendingCommand = "rainbow";
    }
    else if (message == "mayThe4th") {
      pendingCommand = "mayThe4th";
    }
    else if (message == "canadaDay") {
      pendingCommand = "canadaDay";
    }
    else if (message == "newYears") {
      pendingCommand = "newYears";
    }
    else if (message == "candyCane") {
      pendingCommand = "candyCane";
    }
    else if (message == "serene") {
      pendingCommand = "serene";
    }
    else if (message.startsWith("setSpeed:")) {
      // Parse speed value from "setSpeed:500" format
      int colonIndex = message.indexOf(':');
      if (colonIndex != -1) {
        unsigned long speed = message.substring(colonIndex + 1).toInt();
        Serial.printf("[MQTT] Queuing setSpeed command: %lu ms\n", speed);
        pendingCommand = "setSpeed";
        pendingCommandParam = speed;
      } else {
        Serial.println("[MQTT] Invalid setSpeed format. Use 'setSpeed:500'");
      }
    }
    else if (message.startsWith("setTrainSpeed:")) {
      // Parse train speed value from "setTrainSpeed:150" format
      int colonIndex = message.indexOf(':');
      if (colonIndex != -1) {
        unsigned long speed = message.substring(colonIndex + 1).toInt();
        Serial.printf("[MQTT] Queuing setTrainSpeed command: %lu ms\n", speed);
        pendingCommand = "setTrainSpeed";
        pendingCommandParam = speed;
      } else {
        Serial.println("[MQTT] Invalid setTrainSpeed format. Use 'setTrainSpeed:150'");
      }
    }
    else {
      Serial.printf("[MQTT] Command not recognized: %s\n", message.c_str());
      unknownCommand = message;  // Store for logging in loop
    }
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
    String connectMsg = mqttClientId + ": [MQTT] Christmas Tree Device Connected - MAC: " + WiFi.macAddress();
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
 * @brief Serve HTML web interface
 */
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Christmas Tree LED Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 30px;
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 10px;
            font-size: 2em;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 0.9em;
        }
        .section {
            margin-bottom: 25px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        .section h2 {
            color: #444;
            margin-bottom: 15px;
            font-size: 1.2em;
            border-bottom: 2px solid #667eea;
            padding-bottom: 5px;
        }
        .button-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 10px;
        }
        button {
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.2);
        }
        button:active {
            transform: translateY(0);
        }
        .btn-status { background: #6c757d; color: white; }
        .btn-red { background: #dc3545; color: white; }
        .btn-green { background: #28a745; color: white; }
        .btn-white { background: #f8f9fa; color: #333; border: 1px solid #ddd; }
        .btn-blue { background: #007bff; color: white; }
        .btn-effect { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }
        .btn-holiday { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); color: white; }
        .speed-control {
            margin-top: 15px;
        }
        .speed-control label {
            display: block;
            margin-bottom: 5px;
            color: #444;
            font-weight: 600;
        }
        .speed-input-group {
            display: flex;
            gap: 10px;
        }
        .speed-input-group input {
            flex: 1;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 14px;
        }
        .speed-input-group button {
            flex-shrink: 0;
        }
        .status-bar {
            text-align: center;
            padding: 15px;
            background: #e7f3ff;
            border-radius: 8px;
            margin-bottom: 20px;
            border-left: 4px solid #007bff;
        }
        .status-bar.success {
            background: #d4edda;
            border-left-color: #28a745;
        }
        .status-bar.error {
            background: #f8d7da;
            border-left-color: #dc3545;
        }
        #response {
            display: none;
            font-weight: 600;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎄 Christmas Tree LED Controller</h1>
        <div class="subtitle">ESP32 with 900 WS2812B LEDs · Firmware v)rawliteral";
  
  html += FIRMWARE_VERSION;
  
  html += R"rawliteral(</div>
        
        <div id="response" class="status-bar"></div>
        
        <div class="section">
            <h2>Status & Control</h2>
            <div class="button-grid">
                <button class="btn-status" onclick="sendCommand('showStatus')">Show Status</button>
                <button class="btn-status" onclick="sendCommand('help')">Help</button>
            </div>
        </div>
        
        <div class="section">
            <h2>Solid Colors</h2>
            <div class="button-grid">
                <button class="btn-red" onclick="sendCommand('allRed')">All Red</button>
                <button class="btn-green" onclick="sendCommand('allGreen')">All Green</button>
                <button class="btn-white" onclick="sendCommand('allWhite')">All White</button>
                <button class="btn-blue" onclick="sendCommand('allBlue')">All Blue</button>
            </div>
        </div>
        
        <div class="section">
            <h2>Blinking Colors</h2>
            <div class="button-grid">
                <button class="btn-red" onclick="sendCommand('allRedBlink')">Red Blink</button>
                <button class="btn-green" onclick="sendCommand('allGreenBlink')">Green Blink</button>
                <button class="btn-white" onclick="sendCommand('allWhiteBlink')">White Blink</button>
                <button class="btn-blue" onclick="sendCommand('allBlueBlink')">Blue Blink</button>
            </div>
            <div class="speed-control">
                <label>Blink Speed (50-5000 ms):</label>
                <div class="speed-input-group">
                    <input type="number" id="speedValue" min="50" max="5000" value="500" placeholder="500">
                    <button class="btn-status" onclick="setSpeed()">Set Speed</button>
                </div>
            </div>
            <div class="speed-control">
                <label>Train Speed (50-1000 ms):</label>
                <div class="speed-input-group">
                    <input type="number" id="trainSpeedValue" min="50" max="1000" value="100" placeholder="100">
                    <button class="btn-status" onclick="setTrainSpeed()">Set Train Speed</button>
                </div>
            </div>
        </div>
        
        <div class="section">
            <h2>Special Effects</h2>
            <div class="button-grid">
                <button class="btn-effect" onclick="sendCommand('twinkle')">Twinkle</button>
                <button class="btn-effect" onclick="sendCommand('twinkle+')">Twinkle+</button>
                <button class="btn-effect" onclick="sendCommand('gold')">Gold</button>
                <button class="btn-effect" onclick="sendCommand('vegas')">Vegas</button>
                <button class="btn-effect" onclick="sendCommand('rainbow')">Rainbow</button>
            </div>
        </div>
        
        <div class="section">
            <h2>Holiday Themes</h2>
            <div class="button-grid">
                <button class="btn-holiday" onclick="sendCommand('christmas')">Christmas</button>
                <button class="btn-holiday" onclick="sendCommand('christmasBasic')">Christmas Basic</button>
                <button class="btn-holiday" onclick="sendCommand('christmasTrain')">Christmas Train</button>
                <button class="btn-holiday" onclick="sendCommand('candyCane')">Candy Cane</button>
                <button class="btn-holiday" onclick="sendCommand('serene')">Serene</button>
                <button class="btn-holiday" onclick="sendCommand('wildChristmas')">Wild Christmas</button>
                <button class="btn-holiday" onclick="sendCommand('halloween')">Halloween</button>
                <button class="btn-holiday" onclick="sendCommand('valentines')">Valentines</button>
                <button class="btn-holiday" onclick="sendCommand('stPatricks')">St. Patrick's</button>
                <button class="btn-holiday" onclick="sendCommand('birthday')">Birthday</button>
                <button class="btn-holiday" onclick="sendCommand('canadaDay')">Canada Day</button>
                <button class="btn-holiday" onclick="sendCommand('newYears')">New Years</button>
                <button class="btn-holiday" onclick="sendCommand('mayThe4th')">May The 4th</button>
            </div>
        </div>
    </div>
    
    <script>
        function sendCommand(cmd) {
            showResponse('Sending: ' + cmd + '...', 'info');
            
            fetch('/cmd?command=' + encodeURIComponent(cmd))
                .then(response => response.text())
                .then(data => {
                    showResponse(data, 'success');
                })
                .catch(error => {
                    showResponse('Error: ' + error, 'error');
                });
        }
        
        function setSpeed() {
            const speed = document.getElementById('speedValue').value;
            if (speed < 50 || speed > 5000) {
                showResponse('Speed must be between 50 and 5000 ms', 'error');
                return;
            }
            sendCommand('setSpeed:' + speed);
        }
        
        function setTrainSpeed() {
            const speed = document.getElementById('trainSpeedValue').value;
            if (speed < 50 || speed > 1000) {
                showResponse('Train speed must be between 50 and 1000 ms', 'error');
                return;
            }
            sendCommand('setTrainSpeed:' + speed);
        }
        
        function showResponse(message, type) {
            const responseDiv = document.getElementById('response');
            responseDiv.textContent = message;
            responseDiv.className = 'status-bar ' + type;
            responseDiv.style.display = 'block';
            
            if (type === 'success') {
                setTimeout(() => {
                    responseDiv.style.display = 'none';
                }, 3000);
            }
        }
    </script>
</body>
</html>
)rawliteral";
  
  webServer.send(200, "text/html", html);
}

/**
 * @brief Handle command requests from web interface
 */
void handleCommand() {
  if (webServer.hasArg("command")) {
    String command = webServer.arg("command");
    pendingCommand = command;
    
    String response = "Command received: " + command;
    logMessage("[Web] " + response);
    webServer.send(200, "text/plain", response);
  } else {
    webServer.send(400, "text/plain", "Missing command parameter");
  }
}

/**
 * @brief Setup web server routes and start server
 */
void setupWebServer() {
  logMessage("[Web] Configuring web server...");
  
  // Route handlers
  webServer.on("/", handleRoot);
  webServer.on("/cmd", handleCommand);
  
  // Start server
  webServer.begin();
  
  String ipAddr = WiFi.localIP().toString();
  logMessage("[Web] ✓ Server started successfully!");
  logMessageF("[Web] Access web interface at: http://%s", ipAddr.c_str());
}

/**
 * @brief Setup and configure OTA (Over-The-Air) updates
 */
void setupOTA() {
  Serial.println();  // Add blank line to console
  logMessage("[OTA] Configuring Over-The-Air updates...");
  
  // Set OTA hostname
  String hostname = "ChristmasTree-" + WiFi.macAddress();
  hostname.replace(":", "");
  ArduinoOTA.setHostname(hostname.c_str());
  logMessageF("[OTA] Hostname: %s", hostname.c_str());
  
  // Set OTA password for security
  ArduinoOTA.setPassword(OTA_PASSWORD);
  logMessage("[OTA] Password protection enabled");
  
  // Configure OTA callbacks
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    logMessageF("[OTA] Update started: %s", type.c_str());
  });
  
  ArduinoOTA.onEnd([]() {
    logMessage("[OTA] Update completed successfully!");
    logMessage("[OTA] Rebooting...");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      Serial.printf("[OTA] Progress: %u%%\n", percent);
      lastPercent = percent;
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      logMessage("Authentication Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      logMessage("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      logMessage("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      logMessage("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      logMessage("End Failed");
    }
  });
  
  ArduinoOTA.begin();
  logMessage("[OTA] ✓ Ready for firmware updates");
  logMessageF("[OTA] IP Address: %s", WiFi.localIP().toString().c_str());
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
  
  // Initialize FastLED for WS2812B LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);  // Reduced brightness to limit power draw
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 3500);  // Limit to 3.5A @ 5V (safe margin on 4A supply)
  
  // Turn off all LEDs first
  turnOffAllLEDs();
  Serial.println("[LED Strip] WS2812B initialized");
  Serial.printf("[LED Strip] GPIO: %d, Number of LEDs: %d\n", LED_PIN, NUM_LEDS);
  
  Serial.println("[System] Setup initializing...");
  
  // Attempt to connect to WiFi
  if (connectToStrongestKnownNetwork()) {
    // WiFi connection successful - now setup MQTT
    Serial.println("[System] Configuring MQTT client...");
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    // Attempt MQTT connection
    connectToMQTT();
    
    // Show connection status on LEDs
    showStatus();
    
    // Setup OTA updates
    setupOTA();
    
    // Setup Web Server
    setupWebServer();
    
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
    // WiFi connection failed - show status
    showStatus();
    Serial.println("[System] WiFi connection failed");
  }
  
  Serial.println();  // Add blank line to console
  logMessageF("[System] Setup complete! Firmware v%s", FIRMWARE_VERSION);
}

void loop() {
  // Process pending commands (execute outside MQTT callback to avoid watchdog)
  if (pendingCommand != "") {
    Serial.printf("[MQTT] Executing pending command: %s\n", pendingCommand.c_str());
    
    if (pendingCommand == "showStatus") {
      showStatus();
    }
    else if (pendingCommand == "help") {
      showHelp();
    }
    else if (pendingCommand == "allRed") {
      allRed();
    }
    else if (pendingCommand == "allRedBlink") {
      allRedBlink();
    }
    else if (pendingCommand == "allGreen") {
      allGreen();
    }
    else if (pendingCommand == "allGreenBlink") {
      allGreenBlink();
    }
    else if (pendingCommand == "allWhite") {
      allWhite();
    }
    else if (pendingCommand == "allWhiteBlink") {
      allWhiteBlink();
    }
    else if (pendingCommand == "allBlue") {
      allBlue();
    }
    else if (pendingCommand == "allBlueBlink") {
      allBlueBlink();
    }
    else if (pendingCommand == "twinkle") {
      twinkle();
    }
    else if (pendingCommand == "twinkle+") {
      twinklePlus();
    }
    else if (pendingCommand == "gold") {
      gold();
    }
    else if (pendingCommand == "vegas") {
      vegas();
    }
    else if (pendingCommand == "valentines") {
      valentines();
    }
    else if (pendingCommand == "stPatricks") {
      stPatricks();
    }
    else if (pendingCommand == "halloween") {
      halloween();
    }
    else if (pendingCommand == "christmas") {
      christmas();
    }
    else if (pendingCommand == "birthday") {
      birthday();
    }
    else if (pendingCommand == "wildChristmas") {
      wildChristmas();
    }
    else if (pendingCommand == "christmasBasic") {
      christmasBasic();
    }
    else if (pendingCommand == "christmasTrain") {
      christmasTrain();
    }
    else if (pendingCommand == "rainbow") {
      rainbow();
    }
    else if (pendingCommand == "mayThe4th") {
      mayThe4th();
    }
    else if (pendingCommand == "canadaDay") {
      canadaDay();
    }
    else if (pendingCommand == "newYears") {
      newYears();
    }
    else if (pendingCommand == "candyCane") {
      candyCane();
    }
    else if (pendingCommand == "serene") {
      serene();
    }
    else if (pendingCommand == "setSpeed") {
      setSpeed(pendingCommandParam);
    }
    else if (pendingCommand == "setTrainSpeed") {
      setTrainSpeed(pendingCommandParam);
    }
    pendingCommand = "";  // Clear the command
    pendingCommandParam = 0;
    
    Serial.println("[MQTT] Command execution complete");
  }
  
  // Log unknown commands (safe to use logMessage here)
  if (unknownCommand != "") {
    logMessageF("[MQTT] Command not recognized: %s", unknownCommand.c_str());
    unknownCommand = "";  // Clear after logging
  }
  
  // Handle OTA updates
  ArduinoOTA.handle();
  
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
  
  // Handle web server requests
  webServer.handleClient();
  
  // Handle LED strip blinking
  if (blinkEnabled) {
    unsigned long now = millis();
    if (now - lastBlinkTime >= blinkSpeed) {
      lastBlinkTime = now;
      blinkState = !blinkState;
      
      if (blinkState) {
        // Turn all LEDs to the blink color
        fill_solid(leds, NUM_LEDS, blinkColor);
      } else {
        // Turn all LEDs off
        FastLED.clear();
      }
      FastLED.show();
    }
  }
  
  // Handle twinkle effect
  if (twinkleEnabled) {
    unsigned long now = millis();
    if (now - lastTwinkleUpdate >= TWINKLE_UPDATE_INTERVAL) {
      lastTwinkleUpdate = now;
      
      // Update a few random LEDs each cycle for smooth, magical effect
      for (int i = 0; i < TWINKLE_LEDS_PER_UPDATE; i++) {
        int ledIndex = random16(NUM_LEDS);
        
        // Random decision: twinkle on, fade, or off
        int action = random8(100);
        
        if (action < 15) {
          // 15% chance: Light up with warm white/golden color
          int brightness = random8(100, 255);
          leds[ledIndex] = CRGB(brightness, brightness * 0.8, brightness * 0.3); // Warm golden
        }
        else if (action < 30) {
          // 15% chance: Dim the LED
          leds[ledIndex].fadeToBlackBy(64);
        }
        else if (action < 40) {
          // 10% chance: Turn off completely
          leds[ledIndex] = CRGB::Black;
        }
        // 60% chance: Do nothing (keep current state)
      }
      
      // Fade all LEDs slightly for smooth transitions
      fadeToBlackBy(leds, NUM_LEDS, 8);
      
      FastLED.show();
    }
  }
  
  // Handle twinkle+ effect - MORE AGGRESSIVE TWINKLING!
  if (twinklePlusEnabled) {
    unsigned long now = millis();
    if (now - lastTwinklePlusUpdate >= TWINKLEPLUS_UPDATE_INTERVAL) {
      lastTwinklePlusUpdate = now;
      
      // Update many random LEDs each cycle for intense, aggressive effect
      for (int i = 0; i < TWINKLEPLUS_LEDS_PER_UPDATE; i++) {
        int ledIndex = random16(NUM_LEDS);
        
        // Random decision: twinkle on, fade, or off (more aggressive probabilities)
        int action = random8(100);
        
        if (action < 30) {
          // 30% chance: Light up with bright cool white sparkle
          int brightness = random8(150, 255);  // Brighter minimum
          leds[ledIndex] = CRGB(brightness, brightness, brightness); // Pure white sparkle
        }
        else if (action < 55) {
          // 25% chance: Dim the LED dramatically
          leds[ledIndex].fadeToBlackBy(100);  // More dramatic fade
        }
        else if (action < 70) {
          // 15% chance: Turn off completely
          leds[ledIndex] = CRGB::Black;
        }
        else if (action < 85) {
          // 15% chance: Flash to maximum brightness with slight blue tint
          leds[ledIndex] = CRGB(240, 245, 255);  // Bright cool white flash
        }
        // Only 15% chance: Do nothing (for more activity)
      }
      
      // More aggressive fade for faster transitions
      fadeToBlackBy(leds, NUM_LEDS, 15);  // Increased from 8 for faster changes
      
      FastLED.show();
    }
  }
  
  // Handle gold effect - Shimmering gold twinkling
  if (goldEnabled) {
    unsigned long now = millis();
    if (now - lastGoldUpdate >= GOLD_UPDATE_INTERVAL) {
      lastGoldUpdate = now;
      
      // Update many random LEDs each cycle for twinkling gold effect
      for (int i = 0; i < GOLD_LEDS_PER_UPDATE; i++) {
        int ledIndex = random16(NUM_LEDS);
        
        // Random decision: brighten, dim, or maintain
        int action = random8(100);
        
        if (action < 35) {
          // 35% chance: Brighten to full gold
          leds[ledIndex] = CRGB(255, 180, 0);  // Bright gold
        }
        else if (action < 60) {
          // 25% chance: Medium gold
          leds[ledIndex] = CRGB(200, 140, 0);  // Medium gold
        }
        else if (action < 75) {
          // 15% chance: Dim gold
          leds[ledIndex] = CRGB(150, 100, 0);  // Dim gold
        }
        else if (action < 85) {
          // 10% chance: Very bright shimmer
          leds[ledIndex] = CRGB(255, 215, 40);  // Bright shimmering gold
        }
        // 15% chance: Do nothing - maintain current state
      }
      
      // Gentle fade to keep the gold color present
      fadeToBlackBy(leds, NUM_LEDS, 8);  // Gentle fade
      
      FastLED.show();
    }
  }
  
  // Handle Vegas effect - WILD AND CRAZY!
  if (vegasEnabled) {
    unsigned long now = millis();
    if (now - lastVegasUpdate >= VEGAS_UPDATE_INTERVAL) {
      lastVegasUpdate = now;
      
      // Increment hue for rainbow cycling
      vegasHue += 4;
      
      // Choose random pattern each update
      int pattern = random8(5);
      
      switch(pattern) {
        case 0:
          // Rainbow chase - section by section
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV(vegasHue + (i * 3), 255, 255);
          }
          break;
          
        case 1:
          // Random color bursts
          for (int i = 0; i < 20; i++) {
            int ledIndex = random16(NUM_LEDS);
            leds[ledIndex] = CHSV(random8(), 255, 255);
          }
          break;
          
        case 2:
          // Sparkle madness
          fadeToBlackBy(leds, NUM_LEDS, 30);
          for (int i = 0; i < 30; i++) {
            leds[random16(NUM_LEDS)] = CHSV(random8(), 200, 255);
          }
          break;
          
        case 3:
          // Solid color flash (saturated colors)
          fill_solid(leds, NUM_LEDS, CHSV(vegasHue, 255, 255));
          break;
          
        case 4:
          // Dual color strobe
          for (int i = 0; i < NUM_LEDS; i++) {
            if (i % 2 == 0) {
              leds[i] = CHSV(vegasHue, 255, 255);
            } else {
              leds[i] = CHSV(vegasHue + 128, 255, 255);
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Valentines effect - Romantic pink and red love
  if (valentinesEnabled) {
    unsigned long now = millis();
    if (now - lastValentinesUpdate >= VALENTINES_UPDATE_INTERVAL) {
      lastValentinesUpdate = now;
      
      // Gentle pulsing hearts - alternating pink and red
      uint8_t brightness = beatsin8(30, 50, 255);  // Slow breathing effect
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i % 2 == 0) {
          leds[i] = CRGB(brightness, 0, brightness / 3);  // Pink
        } else {
          leds[i] = CRGB(brightness, 0, 0);  // Red
        }
      }
      
      FastLED.show();
    }
  }
  
  // Handle St. Patrick's effect - Irish green and gold luck
  if (stPatricksEnabled) {
    unsigned long now = millis();
    if (now - lastStPatricksUpdate >= STPATRICKS_UPDATE_INTERVAL) {
      lastStPatricksUpdate = now;
      
      stPatricksPhase++;
      
      // Choose Irish pattern based on phase
      int pattern = (stPatricksPhase / 60) % 4;  // Pattern changes every ~2.7 seconds
      
      switch(pattern) {
        case 0:
          // Emerald wave - flowing green gradient
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (stPatricksPhase + i * 3) % 256;
              if (pos < 128) {
                // Bright green gradient
                uint8_t brightness = 100 + pos;
                leds[i] = CRGB(0, brightness, pos / 4);
              } else {
                // Dark green gradient
                uint8_t brightness = 355 - pos;
                leds[i] = CRGB(0, brightness, 20);
              }
            }
          }
          break;
          
        case 1:
          // Leprechaun gold sparkles on green
          {
            // Base green layer
            fadeToBlackBy(leds, NUM_LEDS, 3);
            for (int i = 0; i < NUM_LEDS; i += 3) {
              leds[i] = CRGB(0, 120, 20);  // Deep green
            }
            
            // Random gold sparkles (pot of gold!)
            for (int i = 0; i < 12; i++) {
              int ledIndex = random16(NUM_LEDS);
              leds[ledIndex] = CRGB(255, 180, 0);  // Gold
            }
          }
          break;
          
        case 2:
          // Shamrock shimmer - green with white luck sparkles
          {
            uint8_t brightness = beatsin8(25, 80, 200);  // Gentle breathing
            for (int i = 0; i < NUM_LEDS; i++) {
              leds[i] = CRGB(0, brightness, brightness / 5);
            }
            
            // Lucky white sparkles
            for (int i = 0; i < 8; i++) {
              leds[random16(NUM_LEDS)] = CRGB(255, 255, 255);
            }
          }
          break;
          
        case 3:
          // Rainbow to pot of gold - green/gold alternating chase
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (stPatricksPhase * 2 + i * 5) % 256;
              if (pos < 128) {
                // Green
                leds[i] = CRGB(0, 200 - pos, 30);
              } else {
                // Gold
                pos = pos - 128;
                leds[i] = CRGB(200 + pos / 2, 150 + pos / 3, 0);
              }
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Halloween effect - Spooky orange, purple, and green
  if (halloweenEnabled) {
    unsigned long now = millis();
    if (now - lastHalloweenUpdate >= HALLOWEEN_UPDATE_INTERVAL) {
      lastHalloweenUpdate = now;
      
      halloweenPhase++;
      
      // Choose spooky pattern based on phase
      int pattern = (halloweenPhase / 70) % 4;  // Pattern changes every ~2.5 seconds
      
      switch(pattern) {
        case 0:
          // Flickering jack-o-lantern - pulsing orange with random flickers
          {
            uint8_t baseBrightness = beatsin8(20, 100, 255);  // Slow pulse
            
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t flicker = random8(3) == 0 ? random8(50, 100) : 0;  // Random flicker
              uint8_t brightness = baseBrightness - flicker;
              leds[i] = CRGB(brightness, brightness / 3, 0);  // Orange
            }
          }
          break;
          
        case 1:
          // Witch's cauldron - bubbling purple and green
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (halloweenPhase * 2 + i * 4) % 256;
              if (pos < 128) {
                // Purple
                uint8_t brightness = 150 + (pos / 2);
                leds[i] = CRGB(brightness / 2, 0, brightness);
              } else {
                // Eerie green
                pos = pos - 128;
                leds[i] = CRGB(0, 200 - pos, pos / 3);
              }
            }
          }
          break;
          
        case 2:
          // Haunted house - random spooky colors appearing
          {
            fadeToBlackBy(leds, NUM_LEDS, 15);
            
            // Random spooky lights
            for (int i = 0; i < 15; i++) {
              int ledIndex = random16(NUM_LEDS);
              int colorChoice = random8(3);
              
              if (colorChoice == 0) {
                leds[ledIndex] = CRGB(255, 100, 0);   // Orange
              } else if (colorChoice == 1) {
                leds[ledIndex] = CRGB(128, 0, 200);   // Purple
              } else {
                leds[ledIndex] = CRGB(0, 255, 50);    // Eerie green
              }
            }
          }
          break;
          
        case 3:
          // Ghostly apparition - floating white/green wisps
          {
            // Dark base
            for (int i = 0; i < NUM_LEDS; i++) {
              leds[i] = CRGB(10, 0, 20);  // Dark purple background
            }
            
            // Ghostly wisps moving through
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (halloweenPhase * 3 + i * 8) % 256;
              if (pos > 200 && pos < 240) {
                // Ghostly white-green
                uint8_t brightness = 255 - ((pos - 200) * 6);
                leds[i] = CRGB(brightness / 2, brightness, brightness / 2);
              }
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Christmas effect - Festive red, green, white, and gold
  if (christmasEnabled) {
    unsigned long now = millis();
    if (now - lastChristmasUpdate >= CHRISTMAS_UPDATE_INTERVAL) {
      lastChristmasUpdate = now;
      
      christmasPhase++;
      
      // Classic red and green waves
      for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t pos = (christmasPhase * 2 + i * 3) % 256;
        if (pos < 128) {
          // Festive red
          uint8_t brightness = 150 + pos;
          leds[i] = CRGB(brightness, 0, 0);
        } else {
          // Christmas green
          uint8_t brightness = 150 + (255 - pos);
          leds[i] = CRGB(0, brightness, 0);
        }
      }
      
      FastLED.show();
    }
  }
  
  // Handle Birthday effect - Colorful celebration with confetti and candles
  if (birthdayEnabled) {
    unsigned long now = millis();
    if (now - lastBirthdayUpdate >= BIRTHDAY_UPDATE_INTERVAL) {
      lastBirthdayUpdate = now;
      
      birthdayPhase++;
      
      // Confetti burst - random colorful sparkles
      fadeToBlackBy(leds, NUM_LEDS, 25);
      
      // Burst of colorful confetti
      for (int i = 0; i < 25; i++) {
        int ledIndex = random16(NUM_LEDS);
        uint8_t hue = random8();  // Random rainbow colors
        leds[ledIndex] = CHSV(hue, 255, 255);
      }
      
      FastLED.show();
    }
  }
  
  // Handle Wild Christmas effect - Fast chaotic Christmas party mode
  if (wildChristmasEnabled) {
    unsigned long now = millis();
    if (now - lastWildChristmasUpdate >= WILDCHRISTMAS_UPDATE_INTERVAL) {
      lastWildChristmasUpdate = now;
      
      wildChristmasPhase++;
      
      // Choose wild pattern based on phase
      int pattern = (wildChristmasPhase / 90) % 4;  // Fast pattern changes every ~2.2 seconds
      
      switch(pattern) {
        case 0:
          // Crazy strobe - rapid red/green/white flashes
          {
            int flashPattern = wildChristmasPhase % 9;
            CRGB color;
            
            if (flashPattern < 3) {
              color = CRGB(255, 0, 0);     // Bright red
            } else if (flashPattern < 6) {
              color = CRGB(0, 255, 0);     // Bright green
            } else {
              color = CRGB(255, 255, 255); // White flash
            }
            
            fill_solid(leds, NUM_LEDS, color);
          }
          break;
          
        case 1:
          // Lightning bolts - random white strikes on Christmas colors
          {
            // Base alternating red/green
            for (int i = 0; i < NUM_LEDS; i++) {
              if ((i + wildChristmasPhase / 2) % 6 < 3) {
                leds[i] = CRGB(150, 0, 0);   // Red
              } else {
                leds[i] = CRGB(0, 150, 0);   // Green
              }
            }
            
            // Random lightning strikes
            if (random8() > 180) {
              int strikePos = random16(NUM_LEDS);
              int strikeLen = random8(20, 60);
              for (int i = 0; i < strikeLen && (strikePos + i) < NUM_LEDS; i++) {
                leds[strikePos + i] = CRGB(255, 255, 255);
              }
            }
          }
          break;
          
        case 2:
          // Spinning Christmas chaos - fast rotating segments
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              int segment = ((i + wildChristmasPhase * 4) / 20) % 5;
              
              switch(segment) {
                case 0:
                  leds[i] = CRGB(255, 0, 0);      // Red
                  break;
                case 1:
                  leds[i] = CRGB(0, 255, 0);      // Green
                  break;
                case 2:
                  leds[i] = CRGB(255, 255, 255);  // White
                  break;
                case 3:
                  leds[i] = CRGB(200, 150, 0);    // Gold
                  break;
                case 4:
                  leds[i] = CRGB(0, 100, 200);    // Ice blue
                  break;
              }
            }
          }
          break;
          
        case 3:
          // Explosive sparkles - bursting Christmas colors everywhere
          {
            fadeToBlackBy(leds, NUM_LEDS, 40);
            
            // Massive sparkle explosions
            for (int i = 0; i < 35; i++) {
              int ledIndex = random16(NUM_LEDS);
              int colorChoice = random8(5);
              
              CRGB sparkleColor;
              switch(colorChoice) {
                case 0:
                  sparkleColor = CRGB(255, 0, 0);      // Red
                  break;
                case 1:
                  sparkleColor = CRGB(0, 255, 0);      // Green
                  break;
                case 2:
                  sparkleColor = CRGB(255, 255, 255);  // White
                  break;
                case 3:
                  sparkleColor = CRGB(255, 200, 0);    // Gold
                  break;
                case 4:
                  sparkleColor = CRGB(100, 200, 255);  // Ice blue
                  break;
              }
              
              leds[ledIndex] = sparkleColor;
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Christmas Basic effect - Red, Green, White alternating with twinkling
  if (christmasBasicEnabled) {
    unsigned long now = millis();
    if (now - lastChristmasBasicUpdate >= CHRISTMASBASIC_UPDATE_INTERVAL) {
      lastChristmasBasicUpdate = now;
      
      // Update random LEDs for twinkling effect
      for (int i = 0; i < 15; i++) {  // Update 15 random LEDs each cycle
        int ledIndex = random16(NUM_LEDS);
        
        // Determine base color for this LED position
        int colorIndex = ledIndex % 3;
        CRGB baseColor;
        if (colorIndex == 0) {
          baseColor = CRGB::Red;
        } else if (colorIndex == 1) {
          baseColor = CRGB::Green;
        } else {
          baseColor = CRGB::White;
        }
        
        // Random twinkle action
        int action = random8(100);
        
        if (action < 20) {
          // 20% chance: Brighten to full brightness (twinkle on)
          leds[ledIndex] = baseColor;
        }
        else if (action < 40) {
          // 20% chance: Dim the LED noticeably
          leds[ledIndex] = baseColor;
          leds[ledIndex].fadeToBlackBy(100);  // Dim to about 60% brightness
        }
        else if (action < 50) {
          // 10% chance: Very dim (almost off but noticeable)
          leds[ledIndex] = baseColor;
          leds[ledIndex].fadeToBlackBy(200);  // Dim to about 22% brightness
        }
        // 50% chance: Do nothing - maintain current state for persistence
      }
      
      // Gentle overall fade to create breathing/twinkling effect
      fadeToBlackBy(leds, NUM_LEDS, 3);  // Very subtle fade
      
      FastLED.show();
    }
  }
  
  // Handle Christmas Train effect - Rotating red, green, white pattern
  if (christmasTrainEnabled) {
    unsigned long now = millis();
    if (now - lastChristmasTrainUpdate >= christmasTrainSpeed) {
      lastChristmasTrainUpdate = now;
      
      // Increment offset to create rotation effect
      christmasTrainOffset++;
      if (christmasTrainOffset >= 3) {
        christmasTrainOffset = 0;  // Reset after full color cycle
      }
      
      // Update all LEDs with rotated pattern
      for (int i = 0; i < NUM_LEDS; i++) {
        int colorIndex = (i + christmasTrainOffset) % 3;
        if (colorIndex == 0) {
          leds[i] = CRGB::Red;
        } else if (colorIndex == 1) {
          leds[i] = CRGB::Green;
        } else {
          leds[i] = CRGB::White;
        }
      }
      
      FastLED.show();
    }
  }
  
  // Handle Rainbow effect - Smooth spectrum animations
  if (rainbowEnabled) {
    unsigned long now = millis();
    if (now - lastRainbowUpdate >= RAINBOW_UPDATE_INTERVAL) {
      lastRainbowUpdate = now;
      
      rainbowPhase++;
      
      // Choose rainbow pattern based on phase
      int pattern = (rainbowPhase / 80) % 4;  // Pattern changes every ~2.4 seconds
      
      switch(pattern) {
        case 0:
          // Classic flowing rainbow wave
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t hue = (rainbowPhase * 2 + i * 2) % 256;
              leds[i] = CHSV(hue, 255, 255);
            }
          }
          break;
          
        case 1:
          // Rainbow pulse - breathing full spectrum
          {
            uint8_t brightness = beatsin8(20, 100, 255);
            
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t hue = (i * 3) % 256;
              leds[i] = CHSV(hue, 255, brightness);
            }
          }
          break;
          
        case 2:
          // Rainbow segments - distinct color blocks moving
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t segment = ((i + rainbowPhase * 2) / 30) % 7;
              uint8_t hue = segment * 36;  // 7 colors evenly spaced around hue wheel
              leds[i] = CHSV(hue, 255, 255);
            }
          }
          break;
          
        case 3:
          // Rainbow sparkle - twinkling multi-color
          {
            fadeToBlackBy(leds, NUM_LEDS, 15);
            
            // Add rainbow sparkles
            for (int i = 0; i < 20; i++) {
              int ledIndex = random16(NUM_LEDS);
              uint8_t hue = random8();
              leds[ledIndex] = CHSV(hue, 255, 255);
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle May The 4th effect - Star Wars themed animations
  if (mayThe4thEnabled) {
    unsigned long now = millis();
    if (now - lastMayThe4thUpdate >= MAYTHE4TH_UPDATE_INTERVAL) {
      lastMayThe4thUpdate = now;
      
      mayThe4thPhase++;
      
      // Choose Star Wars pattern based on phase
      int pattern = (mayThe4thPhase / 75) % 4;  // Pattern changes every ~2.6 seconds
      
      switch(pattern) {
        case 0:
          // Lightsaber duel - blue vs red clashing
          {
            int duelPosition = (mayThe4thPhase * 4) % NUM_LEDS;
            
            for (int i = 0; i < NUM_LEDS; i++) {
              if (i < duelPosition) {
                // Blue lightsaber (Jedi)
                int distance = abs(i - duelPosition);
                if (distance < 30) {
                  uint8_t brightness = 255 - (distance * 8);
                  leds[i] = CRGB(brightness / 4, brightness / 4, brightness);
                } else {
                  leds[i] = CRGB(0, 0, 0);
                }
              } else {
                // Red lightsaber (Sith)
                int distance = abs(i - duelPosition);
                if (distance < 30) {
                  uint8_t brightness = 255 - (distance * 8);
                  leds[i] = CRGB(brightness, brightness / 8, brightness / 8);
                } else {
                  leds[i] = CRGB(0, 0, 0);
                }
              }
            }
            
            // Clash point - white flash
            for (int i = -3; i <= 3; i++) {
              int pos = duelPosition + i;
              if (pos >= 0 && pos < NUM_LEDS) {
                leds[pos] = CRGB(255, 255, 255);
              }
            }
          }
          break;
          
        case 1:
          // Hyperspace jump - streaking blue and white
          {
            fadeToBlackBy(leds, NUM_LEDS, 50);
            
            // Create hyperspace streaks
            for (int i = 0; i < 15; i++) {
              int streakStart = (mayThe4thPhase * 6 + i * 60) % NUM_LEDS;
              int streakLength = 20;
              
              for (int j = 0; j < streakLength; j++) {
                int pos = (streakStart + j) % NUM_LEDS;
                uint8_t brightness = 255 - (j * 12);
                if (i % 2 == 0) {
                  leds[pos] = CRGB(brightness / 2, brightness / 2, brightness);  // Blue streak
                } else {
                  leds[pos] = CRGB(brightness, brightness, brightness);  // White streak
                }
              }
            }
          }
          break;
          
        case 2:
          // Death Star tractor beam - pulsing green beams
          {
            // Space background
            for (int i = 0; i < NUM_LEDS; i++) {
              leds[i] = CRGB(2, 2, 5);  // Dark space
            }
            
            // Starfield twinkle
            if (random8() > 200) {
              int star = random16(NUM_LEDS);
              leds[star] = CRGB(255, 255, 255);
            }
            
            // Pulsing green tractor beams
            uint8_t beamBrightness = beatsin8(25, 50, 255);
            for (int i = 0; i < NUM_LEDS; i += 50) {
              int beamCenter = (i + mayThe4thPhase) % NUM_LEDS;
              
              for (int j = -8; j <= 8; j++) {
                int pos = beamCenter + j;
                if (pos >= 0 && pos < NUM_LEDS) {
                  uint8_t brightness = beamBrightness - (abs(j) * 15);
                  leds[pos] = CRGB(0, brightness, brightness / 3);
                }
              }
            }
          }
          break;
          
        case 3:
          // Force energy - alternating Jedi blue/green and Sith red
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t wave = sin8((mayThe4thPhase * 2 + i * 4) % 256);
              
              if (wave < 128) {
                // Light side - blue/green Force energy
                uint8_t brightness = wave * 2;
                if (i % 2 == 0) {
                  leds[i] = CRGB(brightness / 4, brightness / 2, brightness);  // Blue
                } else {
                  leds[i] = CRGB(brightness / 4, brightness, brightness / 4);  // Green
                }
              } else {
                // Dark side - red Force lightning
                uint8_t brightness = (255 - wave) * 2;
                leds[i] = CRGB(brightness, brightness / 8, 0);
              }
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Canada Day effect - Red and white patriotic Canadian celebration
  if (canadaDayEnabled) {
    unsigned long now = millis();
    if (now - lastCanadaDayUpdate >= CANADADAY_UPDATE_INTERVAL) {
      lastCanadaDayUpdate = now;
      
      canadaDayPhase++;
      
      // Choose Canadian pattern based on phase
      int pattern = (canadaDayPhase / 70) % 4;  // Pattern changes every ~2.8 seconds
      
      switch(pattern) {
        case 0:
          // Maple leaf stripes - alternating red and white bands
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (canadaDayPhase + i * 5) % 100;
              if (pos < 50) {
                // Canadian red
                leds[i] = CRGB(255, 0, 0);
              } else {
                // Pure white
                leds[i] = CRGB(255, 255, 255);
              }
            }
          }
          break;
          
        case 1:
          // Northern lights shimmer - red and white aurora
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t wave1 = sin8((canadaDayPhase * 2 + i * 3) % 256);
              uint8_t wave2 = sin8((canadaDayPhase * 3 + i * 2) % 256);
              
              if (wave1 > wave2) {
                // Red shimmer
                uint8_t brightness = (wave1 + wave2) / 2;
                leds[i] = CRGB(brightness, brightness / 8, brightness / 8);
              } else {
                // White shimmer
                uint8_t brightness = (wave1 + wave2) / 2;
                leds[i] = CRGB(brightness, brightness, brightness);
              }
            }
          }
          break;
          
        case 2:
          // Fireworks burst - red and white explosions
          {
            fadeToBlackBy(leds, NUM_LEDS, 20);
            
            // Create firework bursts
            if (canadaDayPhase % 15 == 0) {
              int burstCenter = random16(NUM_LEDS);
              bool isRed = random8() > 127;
              
              // Burst pattern
              for (int i = -20; i <= 20; i++) {
                int pos = burstCenter + i;
                if (pos >= 0 && pos < NUM_LEDS) {
                  uint8_t brightness = 255 - (abs(i) * 10);
                  if (isRed) {
                    leds[pos] = CRGB(brightness, 0, 0);
                  } else {
                    leds[pos] = CRGB(brightness, brightness, brightness);
                  }
                }
              }
            }
            
            // Sparkles
            for (int i = 0; i < 15; i++) {
              int ledIndex = random16(NUM_LEDS);
              if (random8() > 127) {
                leds[ledIndex] = CRGB(255, 0, 0);        // Red sparkle
              } else {
                leds[ledIndex] = CRGB(255, 255, 255);    // White sparkle
              }
            }
          }
          break;
          
        case 3:
          // Flag wave - flowing red/white/red pattern
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              // Create three sections like the Canadian flag
              uint8_t section = ((i + canadaDayPhase * 2) * 3 / NUM_LEDS);
              uint8_t wave = beatsin8(20, 150, 255, 0, i * 2);
              
              if (section == 0 || section == 2) {
                // Red sections (left and right of flag)
                leds[i] = CRGB(wave, 0, 0);
              } else {
                // White center section (where maple leaf would be)
                // Add slight red tint for maple leaf suggestion
                uint8_t maple = sin8((canadaDayPhase * 4 + i * 8) % 256);
                if (maple > 200) {
                  leds[i] = CRGB(wave, wave / 4, wave / 4);  // Red maple highlight
                } else {
                  leds[i] = CRGB(wave, wave, wave);  // White background
                }
              }
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle New Years effect - Gold, silver, and colorful celebration
  if (newYearsEnabled) {
    unsigned long now = millis();
    if (now - lastNewYearsUpdate >= NEWYEARS_UPDATE_INTERVAL) {
      lastNewYearsUpdate = now;
      
      newYearsPhase++;
      
      // Choose celebration pattern based on phase
      int pattern = (newYearsPhase / 75) % 4;  // Pattern changes every ~2.6 seconds
      
      switch(pattern) {
        case 0:
          // Champagne bubbles - rising gold and silver sparkles
          {
            fadeToBlackBy(leds, NUM_LEDS, 20);
            
            // Rising bubbles effect
            for (int i = 0; i < 30; i++) {
              int ledIndex = random16(NUM_LEDS);
              bool isGold = random8() > 127;
              
              if (isGold) {
                leds[ledIndex] = CRGB(255, 200, 0);      // Gold bubble
              } else {
                leds[ledIndex] = CRGB(220, 220, 255);    // Silver/white bubble
              }
            }
          }
          break;
          
        case 1:
          // Countdown sparkle - alternating gold and silver waves
          {
            for (int i = 0; i < NUM_LEDS; i++) {
              uint8_t pos = (newYearsPhase * 3 + i * 2) % 256;
              if (pos < 128) {
                // Gold wave
                uint8_t brightness = 150 + pos;
                leds[i] = CRGB(brightness, brightness * 0.7, 0);
              } else {
                // Silver wave
                uint8_t brightness = 150 + (255 - pos);
                leds[i] = CRGB(brightness * 0.8, brightness * 0.8, brightness);
              }
            }
          }
          break;
          
        case 2:
          // Fireworks burst - colorful explosions
          {
            fadeToBlackBy(leds, NUM_LEDS, 15);
            
            // Create firework bursts
            if (newYearsPhase % 12 == 0) {
              int burstCenter = random16(NUM_LEDS);
              uint8_t hue = random8();  // Random color
              
              // Burst pattern
              for (int i = -25; i <= 25; i++) {
                int pos = burstCenter + i;
                if (pos >= 0 && pos < NUM_LEDS) {
                  uint8_t brightness = 255 - (abs(i) * 8);
                  leds[pos] = CHSV(hue, 255, brightness);
                }
              }
            }
            
            // Add sparkles
            for (int i = 0; i < 20; i++) {
              int ledIndex = random16(NUM_LEDS);
              uint8_t sparkleHue = random8();
              leds[ledIndex] = CHSV(sparkleHue, 255, 255);
            }
          }
          break;
          
        case 3:
          // Confetti celebration - rapid multicolor bursts
          {
            fadeToBlackBy(leds, NUM_LEDS, 30);
            
            // Intense confetti burst
            for (int i = 0; i < 35; i++) {
              int ledIndex = random16(NUM_LEDS);
              uint8_t colorChoice = random8(5);
              
              switch(colorChoice) {
                case 0:
                  leds[ledIndex] = CRGB(255, 200, 0);    // Gold
                  break;
                case 1:
                  leds[ledIndex] = CRGB(220, 220, 255);  // Silver
                  break;
                case 2:
                  leds[ledIndex] = CRGB(255, 0, 100);    // Pink
                  break;
                case 3:
                  leds[ledIndex] = CRGB(0, 200, 255);    // Cyan
                  break;
                case 4:
                  leds[ledIndex] = CRGB(150, 0, 255);    // Purple
                  break;
              }
            }
          }
          break;
      }
      
      FastLED.show();
    }
  }
  
  // Handle Candy Cane effect - Red and white stripes
  if (candyCaneEnabled) {
    unsigned long now = millis();
    if (now - lastCandyCaneUpdate >= CANDYCANE_UPDATE_INTERVAL) {
      lastCandyCaneUpdate = now;
      
      candyCanePhase++;
      
      // Candy cane stripes - red and white
      for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t pos = (candyCanePhase + i * 10) % 80;
        if (pos < 40) {
          // Bright red stripe
          leds[i] = CRGB(255, 0, 0);
        } else {
          // Pure white stripe
          leds[i] = CRGB(255, 255, 255);
        }
      }
      
      FastLED.show();
    }
  }
  
  // Handle Serene effect - Gentle Christmas palette sparkles
  if (sereneEnabled) {
    unsigned long now = millis();
    if (now - lastSereneUpdate >= SERENE_UPDATE_INTERVAL) {
      lastSereneUpdate = now;
      
      // Gentle global fade - keep a soft tail
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].nscale8(230);
      }
      
      // Christmas palette seeds: warm white, soft red, soft green, gold
      const CRGB palette[] = {
        CRGB(255, 240, 200), // warm white
        CRGB(200, 30, 30),   // soft red
        CRGB(20, 160, 40),   // soft green
        CRGB(230, 180, 40)   // gold
      };
      
      // Seed a few random pixels
      uint8_t seeds = 3 + random8(3); // 3-5 sparks per frame
      for (uint8_t s = 0; s < seeds; s++) {
        int idx = random16(NUM_LEDS);
        CRGB base = palette[random8(sizeof(palette) / sizeof(palette[0]))];
        uint8_t boost = 140 + random8(115); // brightness 140-255
        CRGB c = base;
        c.nscale8(boost);
        // slight color variation
        c.r = qadd8(c.r, random8(10));
        c.g = qadd8(c.g, random8(10));
        c.b = qadd8(c.b, random8(10));
        leds[idx] = c;
      }
      
      FastLED.show();
    }
  }
  
  delay(100);
}
