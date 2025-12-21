# Christmas Tree LED Controller

Addressable WS2812B LED strip controller with 900 LEDs, WiFi connectivity, MQTT control, and Over-The-Air (OTA) firmware updates. Features multiple holiday-themed animations and solid color displays controlled via MQTT commands.

## Hardware

- **Board**: Freenove ESP32-WROOM-32 v1.3 Development Board
- **MCU**: ESP32-D0WD-V3 (Dual Core, 240MHz)
- **Flash**: 4MB
- **RAM**: 320KB
- **Built-in LED**: GPIO2
- **LED Strip**: WS2812B (900 LEDs) connected to GPIO 33
- **Power Supply**: 5V 4A (with current limiting to 3.5A for safety)

## Features

### 🌐 WiFi Connectivity
- **Automatic Network Selection**: Scans available WiFi networks and automatically connects to the strongest known network
- **Multiple Network Support**: Configure multiple WiFi networks in the secrets file
- **Signal Monitoring**: Displays RSSI (signal strength) during connection
- **Auto-Reconnect**: Automatically attempts to reconnect if WiFi connection is lost
- **Network Information**: Reports SSID, IP address, MAC address, signal strength, and channel

### 📡 MQTT Communication
- **Broker Connection**: Connects to configurable MQTT broker (default: 192.168.2.21:1883)
- **Three Topics**:
  - `christmasTree-cmd` - Subscribes to receive commands
  - `christmasTree-msg` - Publishes device status messages
  - `christmasTree-log` - Publishes console log messages
- **Unique Client ID**: Auto-generated based on device MAC address (e.g., `ESP32-ChristmasTree-14:08:08:AB:51:4C`)
- **Message Prefixing**: All MQTT messages prefixed with client ID for multi-device identification
- **Auto-Reconnect**: Attempts to reconnect every 5 seconds if MQTT connection is lost
- **Console Mirroring**: All console messages automatically published to MQTT for remote monitoring

### 🔐 Security
- **Password-Protected OTA**: Secure firmware updates with configurable password
- **Credentials Management**: WiFi passwords and sensitive data stored in separate `secrets.h` file
- **Git Ignored**: Secrets file automatically excluded from version control

### 🔄 Over-The-Air (OTA) Updates
- **Network-Based Updates**: Upload new firmware over WiFi without USB connection
- **Authentication**: Password-protected (ChristmasTree2025!)
- **Progress Monitoring**: Real-time update progress reported via MQTT
- **Error Handling**: Comprehensive error reporting for failed updates
- **Hostname**: Auto-generated based on MAC address (e.g., `ChristmasTree-140808AB514C`)

### 💡 LED Status Indicator (GPIO2)
Visual feedback of system state:
- **OFF**: No WiFi connection
- **SLOW BLINK** (1 second): WiFi connected, MQTT disconnected
- **SOLID ON**: WiFi + MQTT both connected

### � LED Control System
- **WS2812B LED Strip**: 900 addressable RGB LEDs on GPIO 33
- **FastLED Library**: High-performance LED control with FastLED 3.7.0
- **Power Management**: Maximum brightness limited to 80/255 with 3.5A current limiting
- **Command Queue System**: Prevents watchdog timeouts during long animations
- **Status Display**: LEDs 0-1 show WiFi and MQTT connection status (green=connected, red=disconnected)
- **Multiple Effects**: 22+ commands including solid colors, blinking patterns, and themed animations

### 🌐 Web Interface
- **Built-in Web Server**: HTTP server on port 80 for direct browser control
- **No MQTT Required**: Control LEDs directly from any web browser on your network
- **Responsive Design**: Mobile-first design that adapts to desktop, tablet, and smartphone screens
- **Modern UI**: Beautiful gradient backgrounds, smooth animations, and intuitive button layouts
- **Real-time Feedback**: Instant command confirmation with color-coded status messages
- **Zero Configuration**: Automatically starts when device boots - just open browser to IP address
- **All Features**: Full access to all 22+ LED commands and effects from one convenient interface
- **Easy Access**: Simply navigate to the ESP32's IP address (e.g., http://192.168.2.159)
- **No Installation**: Works with any modern browser - Chrome, Firefox, Safari, Edge
- **Organized Controls**: Grouped by function - Status, Colors, Blink, Effects, Holidays

### 📝 Logging & Diagnostics
- **Comprehensive Console Output**: Detailed status messages for all operations
- **MQTT Log Publishing**: All log messages sent to broker for remote monitoring
- **Firmware Version Tracking**: Version number included in startup messages
- **Connection Details**: Full network and MQTT connection information

## Project Structure

```
christmasTree/
├── include/
│   └── secrets.h          # WiFi credentials, MQTT config, OTA password
├── lib/                   # Project-specific libraries
├── src/
│   └── main.cpp          # Main application code
├── platformio.ini        # PlatformIO configuration
├── ota-update.sh        # Shell script for OTA updates
├── .gitignore           # Excludes secrets and build artifacts
└── README.md            # This file
```

## Configuration

### Secrets File (`include/secrets.h`)

Contains sensitive configuration data:

```cpp
// WiFi Networks (add as many as needed)
const KnownNetwork knownNetworks[] = {
  {"YourSSID1", "password1"},
  {"YourSSID2", "password2"},
  {"YourSSID3", "password3"}
};

// MQTT Broker
const char* MQTT_BROKER = "192.168.2.21";
const int MQTT_PORT = 1883;

// OTA Password
const char* OTA_PASSWORD = "ChristmasTree2025!";
```

### PlatformIO Configuration (`platformio.ini`)

Key settings:
- **Platform**: Espressif32
- **Board**: esp32dev
- **Framework**: Arduino
- **Serial Speed**: 115200 baud
- **Upload Port**: /dev/cu.usbserial-8310 (or IP address for OTA)
- **Upload Speed**: 460800 baud

## Getting Started

### Prerequisites

#### Required Software
1. **Visual Studio Code** - [Download here](https://code.visualstudio.com/)
2. **PlatformIO IDE Extension** - Install from VS Code Extensions marketplace
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X / Cmd+Shift+X)
   - Search for "PlatformIO IDE"
   - Click Install

#### Required Hardware
1. **ESP32-WROOM-32 Development Board**
2. **WS2812B LED Strip** (900 LEDs or adjust NUM_LEDS in code)
3. **5V Power Supply** (4A minimum for 900 LEDs)
4. **USB Cable** (for initial programming)

#### MQTT Broker
You need an MQTT broker running on your network. Options include:
- **Mosquitto** (recommended) - [Installation guide](https://mosquitto.org/download/)
- **HiveMQ** 
- **EMQX**
- Cloud services like CloudMQTT or HiveMQ Cloud

To install Mosquitto on common platforms:
```bash
# Ubuntu/Debian
sudo apt-get install mosquitto mosquitto-clients

# macOS
brew install mosquitto

# Windows
# Download installer from https://mosquitto.org/download/
```

#### MQTT Client (for testing)
- **MQTT Explorer** (recommended) - [Download here](http://mqtt-explorer.com/)
- **mosquitto_pub/mosquitto_sub** (command line)
- **MQTT.fx**

### Initial Setup

#### 1. Clone the Repository

Using VS Code:
1. Open VS Code
2. Press `Ctrl+Shift+P` (Windows/Linux) or `Cmd+Shift+P` (macOS)
3. Type "Git: Clone" and press Enter
4. Enter repository URL: `https://github.com/theAgingApprentice/christmasTree.git`
5. Select a folder location
6. Click "Open" when prompted

Using command line:
```bash
git clone https://github.com/theAgingApprentice/christmasTree.git
cd christmasTree
```

Then open in VS Code:
```bash
code .
```

#### 2. Configure Secrets

Create or edit `include/secrets.h` with your WiFi and MQTT settings:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Network Structure
struct KnownNetwork {
  const char* ssid;
  const char* password;
};

// WiFi Networks (add as many as needed)
const KnownNetwork knownNetworks[] = {
  {"YourHomeWiFi", "your_wifi_password"},
  {"YourOfficeWiFi", "office_password"},
  {"GuestNetwork", "guest_password"}
};

const int numKnownNetworks = sizeof(knownNetworks) / sizeof(knownNetworks[0]);

// MQTT Broker Configuration
const char* MQTT_BROKER = "192.168.2.21";  // Change to your broker IP
const int MQTT_PORT = 1883;

// OTA Password
const char* OTA_PASSWORD = "ChristmasTree2025!";

#endif
```

**Important**: The `secrets.h` file is ignored by git to protect your credentials.

#### 3. Build and Upload

This project supports two firmware upload methods configured in `platformio.ini`:
- **Serial Upload** (`esp32dev`) - For new devices or when OTA is unavailable
- **OTA Upload** (`esp32dev-ota`) - For wirelessly updating existing devices

Each environment is independent, allowing you to choose the upload method at compile time without errors or conflicts.

##### Serial Upload (New Devices)

Use this method for brand new ESP32 devices or when OTA is not available.

**PlatformIO Configuration:**
```ini
[env:esp32dev]
upload_speed = 460800
upload_port = /dev/cu.usbserial-0265D23D
monitor_port = /dev/cu.usbserial-0265D23D
```

**Upload Steps:**

1. Connect ESP32 to computer via USB
2. Identify the serial port:
   - **macOS**: Usually `/dev/cu.usbserial-XXXXXXXX`
   - **Linux**: Usually `/dev/ttyUSB0` or `/dev/ttyACM0`
   - **Windows**: Usually `COM3`, `COM4`, etc.
3. Update `upload_port` in `platformio.ini` if needed
4. Upload using one of these methods:

**VS Code:**
- Click PlatformIO icon → PROJECT TASKS → esp32dev → Upload
- Or use PlatformIO toolbar: → (arrow) = Upload

**Command Line:**
```bash
# Build and upload to serial port (specifying environment)
platformio run -e esp32dev -t upload

# Or shorter version
pio run -e esp32dev -t upload

# Monitor serial output
pio device monitor
```

**Important:** Always specify the environment (`-e esp32dev` or `-e esp32dev-ota`) to avoid building both environments and getting false errors.

##### OTA Upload (Existing Devices)

Use this method for wirelessly updating ESP32 devices that are already running the firmware and connected to WiFi.

**PlatformIO Configuration:**
```ini
[env:esp32dev-ota]
upload_protocol = espota
upload_flags = 
    --auth=ChristmasTree2025!
upload_port = 192.168.1.xxx  ; Placeholder - use script or update manually
```

**Prerequisites:**
- Device must be running firmware with OTA enabled
- Device must be connected to WiFi
- You must know the device's IP address (check serial monitor or MQTT logs)

**Upload Steps:**

1. Find your device's IP address from:
   - Serial monitor output during startup
   - MQTT log messages (`christmasTree-log` topic)
   - Your router's DHCP client list

2. Use the OTA update script (recommended method):

**Using the OTA Script (Recommended):**
```bash
# Make script executable (first time only)
chmod +x ota-update.sh

# Upload to device at specified IP
./ota-update.sh 192.168.2.159
```

The script will:
- Validate the IP address format
- Build and upload only the OTA environment (no false errors)
- Display upload progress
- Show success/failure with clear messages

**VS Code (Alternative):**
1. Update `upload_port` in the `[env:esp32dev-ota]` section with your device's IP
2. Click PlatformIO icon → PROJECT TASKS → esp32dev-ota → Upload

**Command Line (Alternative):**
```bash
# Build and upload via OTA to specific IP
pio run -e esp32dev-ota -t upload --upload-port 192.168.2.159

# Or use the configured IP in platformio.ini
pio run -e esp32dev-ota -t upload
```

**OTA Upload Process:**
1. Firmware compiles
2. Connects to device over WiFi
3. Authenticates with password (`ChristmasTree2025!`)
4. Uploads new firmware
5. Device automatically reboots with new firmware

**Troubleshooting OTA:**
- Ensure device is powered on and connected to WiFi
- Verify IP address is correct (check MQTT logs or serial output)
- Check that both computer and ESP32 are on same network
- Confirm firewall isn't blocking port 3232 (ESP-OTA default)
- Try serial upload if OTA continues to fail

##### Switching Between Upload Methods

The project is configured with two independent environments. Always specify which one to use:

| Environment | Method | Use Case | Command |
|-------------|--------|----------|---------|
| `esp32dev` | Serial (USB) | New devices, OTA unavailable | `pio run -e esp32dev -t upload` |
| `esp32dev-ota` | OTA (WiFi) | Existing devices, wireless updates | `./ota-update.sh <IP>` |

**Why Specify the Environment?**
- Running `pio run -t upload` without `-e` will attempt to build **both** environments
- The non-connected environment will fail, showing confusing error messages
- Specifying `-e esp32dev` or `-e esp32dev-ota` ensures only one builds and uploads
- The OTA script automatically specifies `-e esp32dev-ota` for clean, error-free updates

**Quick Reference:**
```bash
# Serial upload (USB connected)
pio run -e esp32dev -t upload

# OTA upload (WiFi) - using script
./ota-update.sh 192.168.2.159

# OTA upload (WiFi) - manual command line
pio run -e esp32dev-ota -t upload --upload-port 192.168.2.159
```

### Updating Your Clone

To pull the latest changes from the repository:

Using VS Code:
1. Press `Ctrl+Shift+P` / `Cmd+Shift+P`
2. Type "Git: Pull" and press Enter

Using command line:
```bash
git pull origin main
```

After pulling updates:
1. Rebuild the project: `pio run`
2. Upload to device: See "Build and Upload" section above for serial or OTA methods

## MQTT Configuration

### Topics

The device uses three MQTT topics:

#### Subscribe Topic (Receive Commands)
- **Topic**: `christmasTree-cmd`
- **Purpose**: Send control commands to the LED controller
- **Message Format**: Plain text strings (see Commands section below)
- **Examples**:
  - `allRed` - Turn all LEDs red
  - `twinkle` - Start twinkling effect
  - `setSpeed:500` - Set blink speed to 500ms

#### Publish Topics (Send Status)

1. **Status Messages**
   - **Topic**: `christmasTree-msg`
   - **Purpose**: Device status and connection messages
   - **Example**: `ESP32-ChristmasTree-14:08:08:AB:51:4C: Christmas Tree Device Connected - MAC: 14:08:08:AB:51:4C`

2. **Log Messages**
   - **Topic**: `christmasTree-log`
   - **Purpose**: All console log messages for remote monitoring
   - **Example**: `ESP32-ChristmasTree-14:08:08:AB:51:4C: [MQTT] ✓ Connection successful!`

### Message Format

All published messages are prefixed with a unique client ID based on the device MAC address:
```
ESP32-ChristmasTree-<MAC>: <message>
```

This allows multiple devices on the same MQTT broker without conflicts.

### Testing MQTT

#### Using MQTT Explorer
1. Launch MQTT Explorer
2. Create new connection:
   - Name: `Christmas Tree`
   - Host: Your MQTT broker IP (e.g., `192.168.2.21`)
   - Port: `1883`
3. Connect
4. Subscribe to `christmasTree-log` to see device messages
5. Publish to `christmasTree-cmd` to send commands

#### Using Command Line
```bash
# Subscribe to status messages
mosquitto_sub -h 192.168.2.21 -t "christmasTree-msg"

# Subscribe to log messages
mosquitto_sub -h 192.168.2.21 -t "christmasTree-log"

# Send a command
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "allRed"

# Send multiple commands
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "rainbow"
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "setSpeed:1000"
```

## How to Use

The Christmas Tree LED Controller can be controlled in two ways:
1. **Web Interface** - Direct browser control (easiest, no additional software needed)
2. **MQTT Commands** - For automation and integration with home automation systems

### Web Interface Control

#### Accessing the Web Interface

1. Ensure your device and the ESP32 are on the same network
2. Find the ESP32's IP address from:
   - Serial monitor output during startup
   - MQTT log messages
   - Your router's DHCP client list
3. Open a web browser and navigate to: `http://<ESP32_IP_ADDRESS>`
   - Example: `http://192.168.2.159`

#### Using the Web Interface

The web interface provides an intuitive, mobile-friendly control panel with organized sections:

**Interface Layout:**

1. **Status & Control Section**
   - `Show Status` - Display WiFi/MQTT connection on LEDs 0-1
   - `Help` - View all available commands (outputs to MQTT log)

2. **Solid Colors Section**
   - Four color buttons: Red, Green, White, Blue
   - One-click to set all 900 LEDs to selected color
   - Color-coded buttons for easy identification

3. **Blinking Colors Section**
   - Blink controls for Red, Green, White, Blue
   - Built-in speed control slider (50-5000ms range)
   - Live speed adjustment without stopping current effect

4. **Special Effects Section**
   - Twinkle - Magical golden sparkles
   - Vegas - Wild rainbow party mode
   - Rainbow - Smooth spectrum animations

5. **Holiday Themes Section**
   - 8 holiday-specific effects with custom animations
   - Christmas, Wild Christmas, Halloween, Valentines
   - St. Patrick's, Birthday, Canada Day, May The 4th
   - Each with multiple sub-patterns that auto-cycle

**User Experience Features:**
- **Instant Feedback**: Each button click shows confirmation message
- **Visual Design**: Modern gradient UI with smooth transitions
- **Touch-Friendly**: Large buttons optimized for touchscreens
- **Status Messages**: Green for success, red for errors, blue for info
- **Auto-Hide**: Success messages disappear after 3 seconds
- **No Refresh**: Page stays loaded - click buttons repeatedly without reload

**Browser Compatibility:**
- ✅ Chrome/Chromium (desktop & mobile)
- ✅ Firefox (desktop & mobile)
- ✅ Safari (macOS & iOS)
- ✅ Edge (Windows)
- ✅ Opera
- ✅ Samsung Internet
- Works on any device with a modern web browser (2020+)

**Advantages:**
- No additional software installation required
- Works on any device with a web browser (phone, tablet, computer)
- Instant, user-friendly control
- Perfect for non-technical users
- No MQTT broker configuration needed

### MQTT Command Control

For automation, integration with smart home systems, or programmatic control, use MQTT commands.

### Available Commands

Send commands via MQTT to the `christmasTree-cmd` topic.

#### Status & Control
- `showStatus` - Display WiFi/MQTT connection status on LEDs 0-1 (Green=connected, Red=disconnected)
- `help` - Display all available commands in MQTT log topic

#### Solid Colors
- `allRed` - Set all 900 LEDs to solid red
- `allGreen` - Set all 900 LEDs to solid green  
- `allWhite` - Set all 900 LEDs to solid white
- `allBlue` - Set all 900 LEDs to solid blue

#### Blinking Colors
- `allRedBlink` - Blink all LEDs red
- `allGreenBlink` - Blink all LEDs green
- `allWhiteBlink` - Blink all LEDs white
- `allBlueBlink` - Blink all LEDs blue

#### Special Effects

**Holiday & Celebration Effects:**
- `twinkle` - Magical twinkling golden sparkles effect with 4 sub-patterns
- `christmas` - Festive red, green, white, and gold animations
  - Classic red/green waves
  - Twinkling white snowfall
  - Candy cane stripes
  - Golden star shimmer
- `wildChristmas` - Fast chaotic Christmas party mode
  - Crazy strobe (red/green/white flashes)
  - Lightning bolts
  - Spinning Christmas chaos
  - Explosive sparkles
- `halloween` - Spooky orange, purple, and green
  - Flickering jack-o-lanterns
  - Witch's cauldron (purple/green)
  - Haunted house random colors
  - Ghostly apparitions
- `valentines` - Romantic pink and red love theme with 4 patterns
- `stPatricks` - Irish green and gold shamrock luck with 4 patterns
- `birthday` - Colorful celebration
  - Confetti burst
  - Rainbow waves
  - Flickering birthday candles
  - Party lights dance
- `canadaDay` - Red and white patriotic Canadian celebration
  - Maple leaf stripes
  - Northern lights shimmer
  - Fireworks burst
  - Flag wave

**Other Effects:**
- `vegas` - Wild and crazy rainbow Las Vegas mode with 5 sub-patterns
- `rainbow` - Smooth spectrum animations
  - Classic flowing rainbow
  - Rainbow pulse
  - Rainbow segments
  - Rainbow sparkle
- `mayThe4th` - Star Wars themed animations
  - Lightsaber duel (blue vs red)
  - Hyperspace jump
  - Death Star tractor beam
  - Force energy (light/dark side)

#### Configuration
- `setSpeed:<milliseconds>` - Set blink speed for blinking effects
  - Range: 50ms to 5000ms
  - Example: `setSpeed:500` for half-second intervals
  - Example: `setSpeed:2000` for 2-second intervals

### Usage Examples

#### Web Interface Examples

Simply open your web browser and navigate to the ESP32's IP address:

**Example URL:** `http://192.168.2.159`

Then click any button on the interface to control the LEDs. No command-line knowledge required!

#### MQTT Command Examples

For automation and integration:

#### Basic Color Control
```bash
# Set all LEDs to red
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "allRed"

# Start green blinking at default speed
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "allGreenBlink"

# Change blink speed to 1 second
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "setSpeed:1000"

# Stop blinking and show status
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "showStatus"
```

#### Special Effects
```bash
# Start Christmas effect
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "christmas"

# Switch to rainbow effect
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "rainbow"

# Show Star Wars effect for May 4th
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "mayThe4th"

# Birthday celebration
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "birthday"
```

#### Integration Examples

**Home Assistant YAML:**
```yaml
# configuration.yaml
mqtt:
  broker: 192.168.2.21
  port: 1883

# scripts.yaml
christmas_tree_red:
  sequence:
    - service: mqtt.publish
      data:
        topic: christmasTree-cmd
        payload: allRed

christmas_tree_rainbow:
  sequence:
    - service: mqtt.publish
      data:
        topic: christmasTree-cmd
        payload: rainbow
```

**Node-RED Flow:**
```json
[
  {
    "type": "mqtt out",
    "topic": "christmasTree-cmd",
    "broker": "mqtt-broker",
    "name": "Christmas Tree Control"
  }
]
```

### Effect Behavior

- **Only one effect runs at a time**: Starting a new effect automatically stops the current one
- **Command queue**: Commands are queued and executed in the main loop to prevent watchdog timeouts
- **Animations loop**: All special effects have multiple sub-patterns that cycle automatically
- **Status override**: The `showStatus` command disables all effects and shows connection status

## MQTT Topics

### Subscribe (Receive Commands)
- **Topic**: `christmasTree-cmd`
- **Purpose**: Receive control commands from external systems
- **Format**: String commands

### Publish (Send Messages)
- **Topic**: `christmasTree-msg`
- **Purpose**: Device status and connection messages
- **Example**: `ESP32-ChristmasTree-14:08:08:AB:51:4C: Christmas Tree Device Connected - MAC: 14:08:08:AB:51:4C`

### Publish (Send Logs)
- **Topic**: `christmasTree-log`
- **Purpose**: All console log messages for remote monitoring
- **Example**: `ESP32-ChristmasTree-14:08:08:AB:51:4C: [MQTT] ✓ Connection successful!`

## Development

### Dependencies

Automatically managed by PlatformIO (defined in `platformio.ini`):
- **PubSubClient** (v2.8) - MQTT client library
- **ArduinoOTA** (v2.0.0) - Over-the-air update functionality
- **FastLED** (v3.7.0) - High-performance LED control library
- **WiFi** (v2.0.0) - ESP32 WiFi library

### Building

```bash
# Build project
pio run

# Clean build
pio run -t clean

# Upload via USB
pio run -t upload

# Upload via OTA
pio run -t upload --upload-port <IP_ADDRESS>

# Monitor serial output
pio device monitor
```

### Firmware Version

Current version is defined in `src/main.cpp`:
```cpp
#define FIRMWARE_VERSION "6.0.0"
```

Update this value when releasing new versions. The version is displayed:
- On startup in serial console
- In MQTT log messages
- Via the `help` command

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in `secrets.h`
- Check signal strength (device connects to strongest network)
- Ensure 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)

### MQTT Connection Issues
- Verify broker IP address and port in `secrets.h`
- Check firewall settings on broker machine
- Ensure broker is running: `sudo systemctl status mosquitto` (Linux)
- Test broker connectivity: `mosquitto_pub -h <broker_ip> -t test -m "hello"`
- Monitor `christmasTree-log` topic for error messages
- Verify broker allows anonymous connections or configure authentication

### LED Strip Issues
- Verify NUM_LEDS matches your actual LED count (default: 900)
- Check GPIO pin connection (should be GPIO 33)
- Ensure adequate power supply (5V 4A minimum for 900 LEDs)
- Verify LED strip is WS2812B with GRB color order
- Check FastLED initialization in code matches your hardware
- Test with `showStatus` command first (uses only 2 LEDs)

### Command Not Working
- Commands are case-sensitive: use `allRed` not `AllRed` or `allred`
- Check MQTT topic is exactly `christmasTree-cmd` (for MQTT control)
- For web interface, ensure you're on the same network as the ESP32
- Monitor `christmasTree-log` for "Unknown command" messages
- Use `help` command to see all available commands
- Ensure no extra spaces in command strings

### Web Interface Not Loading
- Verify ESP32 is powered on and connected to WiFi
- Check that you're using the correct IP address (look in serial monitor or MQTT logs)
- Ensure your device is on the same network as the ESP32
- Try accessing from a different browser
- Check serial monitor for web server startup messages: `[Web] ✓ Server started successfully!`
- Verify firewall isn't blocking port 80
- Try clearing browser cache or using incognito/private mode
- Test network connectivity: `ping <ESP32_IP>`
- If using company/school network, HTTP port 80 may be blocked

### Web Interface Performance Issues
- The interface uses client-side JavaScript - no page reloads needed
- If commands seem slow, check WiFi signal strength (RSSI in serial monitor)
- Each button click sends one HTTP request to /cmd endpoint
- Web server handles one request at a time (sequential processing)
- For rapid command sequences, consider using MQTT instead
- Clear browser cache if buttons become unresponsive

### OTA Update Failures
- Verify device IP address (check MQTT logs or serial monitor)
- Ensure OTA password matches in both `secrets.h` and `platformio.ini`
- Check network connectivity (ping the device)
- Verify firewall allows port 3232

### Serial Upload Issues
- Check USB cable and connection
- Verify correct serial port in `platformio.ini`
- Hold BOOT button on ESP32 during upload if auto-reset fails

## Status Messages

Device reports detailed status via serial console and MQTT:

**Startup Sequence:**
1. System initialization
2. WiFi network scan
3. Connection to strongest known network
4. MQTT broker connection
5. OTA service activation
6. Web server startup on port 80
7. LED status timer start
8. Setup complete with firmware version

The device will display its IP address in the serial console and MQTT logs, formatted as:
```
[Web] ✓ Server started successfully!
[Web] Access web interface at: http://192.168.2.159
```

**Runtime Messages:**
- WiFi connection status with RSSI (signal strength)
- MQTT subscription confirmations
- Incoming command notifications with command name
- LED effect activation messages
- Connection loss/recovery alerts
- OTA update progress and completion

**Command Responses:**
- Unknown command warnings with the invalid command shown
- Speed change confirmations
- Effect activation confirmations
- Help command output with full command list

## Quick Reference Card

### Web Interface Access
Simply open a browser and go to your ESP32's IP address:
```
http://192.168.2.159  (replace with your device's IP)
```

### Common Commands
| Command | Description |
|---------|-------------|
| `showStatus` | Show WiFi/MQTT status on LEDs 0-1 |
| `help` | List all commands |
| `allRed` | Solid red |
| `allGreen` | Solid green |
| `allWhite` | Solid white |
| `allRedBlink` | Blinking red |
| `setSpeed:1000` | Set blink to 1 second |
| `christmas` | Christmas theme |
| `rainbow` | Rainbow effect |
| `twinkle` | Twinkling lights |

### MQTT Quick Commands
```bash
# Connect and monitor (leave this running)
mosquitto_sub -h 192.168.2.21 -t "christmasTree-log"

# In another terminal, send commands:
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "christmas"
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "rainbow"
mosquitto_pub -h 192.168.2.21 -t "christmasTree-cmd" -m "showStatus"
```

## Technical Specifications

### Web Server
- **Port**: 80 (HTTP)
- **Protocol**: HTTP/1.1
- **Concurrent Connections**: 1 (sequential processing)
- **Response Time**: <100ms for command requests
- **HTML Size**: ~8KB (embedded in firmware)
- **Routes**: 
  - `/` - Main interface (GET)
  - `/cmd?command=<cmd>` - Command endpoint (GET)
- **MIME Types**: text/html, text/plain
- **No Authentication**: Open access on local network (assumes trusted network)

### Power Consumption
- **Maximum per LED**: 60mA at full white (WS2812B spec)
- **Theoretical maximum (900 LEDs)**: 54A at full white
- **Actual maximum (with limiting)**: 3.5A
- **Brightness limiting**: MAX_BRIGHTNESS = 80 (31% of full)
- **FastLED current limit**: 3500mA at 5V
- **Recommended supply**: 5V 4A minimum

### Performance
- **Animation update rates**: 25-50ms depending on effect
- **Command processing**: Queue-based, processed in main loop
- **Web request handling**: ~50-100ms response time
- **Watchdog**: Prevented via yield() calls during long operations
- **WiFi auto-reconnect**: Every 5 seconds when disconnected
- **MQTT auto-reconnect**: Every 5 seconds when disconnected
- **Web server**: Always active when WiFi connected

### Memory Usage
- **LED buffer**: 900 LEDs × 3 bytes = 2,700 bytes
- **Web server**: ~2KB RAM overhead
- **HTML interface**: 8KB Flash storage (program memory)
- **ESP32 RAM**: 320KB total
- **Available for effects**: ~295KB after WiFi/MQTT/Web overhead

### Network Requirements
- **WiFi**: 2.4GHz (802.11 b/g/n)
- **IP Assignment**: DHCP (automatic)
- **Ports Used**: 
  - 80 (HTTP web server)
  - 1883 (MQTT client connection)
  - 3232 (OTA updates)
- **Network Mode**: Station (STA) - connects to existing network
- **Access**: Must be on same subnet for web interface access

## Security Considerations

### Network Security
- **Web Interface**: No authentication - assumes trusted local network
- **Recommendation**: Use on private home network only
- **OTA Updates**: Password protected (ChristmasTree2025!)
- **MQTT**: No authentication by default (configure broker for security)
- **WiFi**: WPA2/WPA3 protected (credentials in secrets.h)

### Best Practices
- Keep the device on a private network segment
- Change OTA password in `secrets.h` before deployment
- Configure MQTT broker with username/password authentication
- Use firewall rules to restrict access if needed
- Regularly update firmware for security patches
- Monitor MQTT logs for unauthorized access attempts

### Privacy
- Device only communicates within local network (no cloud services)
- WiFi credentials stored locally on ESP32
- No data collection or external reporting
- All control is local - works without internet connection

## Customization

### Modifying the Web Interface

The web interface HTML is embedded in [main.cpp](src/main.cpp) lines ~730-965. To customize:

1. **Locate the `handleRoot()` function** in src/main.cpp
2. **Edit the HTML/CSS/JavaScript** within the `R"rawliteral(...)rawliteral"` string
3. **Available sections to customize**:
   - CSS styles (colors, fonts, layout)
   - Button labels and organization
   - JavaScript command functions
   - Page title and branding

4. **Rebuild and upload** the firmware:
   ```bash
   pio run -t upload
   ```

**Example Customizations:**
- Change color scheme by modifying CSS gradient backgrounds
- Add/remove buttons for specific effects
- Reorganize button layout by editing grid sections
- Add custom commands by creating new buttons with `onclick="sendCommand('yourCommand')"`
- Modify response messages and timeout durations

### Adding New Commands

To add new LED effects accessible via web interface:

1. **Create the effect function** in main.cpp
2. **Add command handler** in MQTT callback section
3. **Add to help message** in showHelp() function
4. **Add button to web interface** in handleRoot() HTML
5. **Rebuild and upload** firmware

See existing effects (christmas, rainbow, etc.) as templates.

## Future Development

Possible enhancements:
- ~~Web interface for direct control (without MQTT client)~~ ✅ **IMPLEMENTED v6.0.0**
- Web interface authentication/password protection
- HTTPS support for encrypted communication
- Preset scenes and playlists accessible from web UI
- Time-based automation with web configuration
- Brightness control slider in web interface
- Custom color picker (RGB/HSV selection)
- Effect speed control for all animations
- Audio reactive effects
- Segment control (control different sections independently)
- Save/restore last effect on power cycle
- Effect preview thumbnails in web interface
- Favorites/bookmarks for frequently used effects
- Transition effects between modes
- API documentation for programmatic control

## Changelog

### Version 6.0.0 (December 2025)
- ✨ Added built-in web interface for browser-based control
- ✨ No MQTT client required for basic operation
- ✨ Responsive mobile-friendly design
- ✨ Real-time command feedback
- 📝 Updated documentation with web interface details
- 📝 Added security considerations section
- 📝 Added customization guide

### Version 5.0.0
- Added multiple holiday-themed effects
- Added command queue system
- Power management optimization
- Comprehensive MQTT logging

## License

See LICENSE file for details.

## Author

theAgingApprentice

## Repository

https://github.com/theAgingApprentice/christmasTree
