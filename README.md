# Christmas Tree Project

Addressable LED strips light display for Christmas tree with WiFi connectivity, MQTT control, and Over-The-Air (OTA) firmware updates.

## Hardware

- **Board**: Freenove ESP32-WROOM-32 v1.3 Development Board
- **MCU**: ESP32-D0WD-V3 (Dual Core, 240MHz)
- **Flash**: 4MB
- **RAM**: 320KB
- **Built-in LED**: GPIO2

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

### Initial Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/theAgingApprentice/christmasTree.git
   cd christmasTree
   ```

2. **Configure secrets**
   - Edit `include/secrets.h`
   - Add your WiFi network credentials
   - Configure MQTT broker IP address
   - Set OTA password

3. **Build the project**
   ```bash
   pio run
   ```

4. **Upload via USB** (first time)
   ```bash
   pio run -t upload
   ```

5. **Monitor serial output**
   ```bash
   pio device monitor
   ```

### Over-The-Air Updates

After initial USB upload, use OTA for subsequent updates:

```bash
./ota-update.sh <IP_ADDRESS>
```

Example:
```bash
./ota-update.sh 192.168.2.159
```

The script will:
- Validate the IP address
- Display target information
- Perform the OTA update
- Report success or failure with troubleshooting tips

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

Automatically managed by PlatformIO:
- **PubSubClient** (v2.8) - MQTT client library
- **ArduinoOTA** (v2.0.0) - Over-the-air update functionality
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
#define FIRMWARE_VERSION "1.0.0"
```

Update this value when releasing new versions.

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in `secrets.h`
- Check signal strength (device connects to strongest network)
- Ensure 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)

### MQTT Connection Issues
- Verify broker IP address and port
- Check firewall settings on broker
- Ensure broker is running and accessible
- Monitor `christmasTree-log` topic for error messages

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
6. LED status timer start
7. Setup complete with firmware version

**Runtime Messages:**
- WiFi connection status
- MQTT subscription confirmations
- Incoming command notifications
- Connection loss/recovery alerts
- OTA update progress

## Future Development

Phase 2 will add:
- LED strip control logic
- Animation patterns
- Command processing for MQTT control
- Configuration via MQTT
- Additional status reporting

## License

See LICENSE file for details.

## Author

theAgingApprentice

## Repository

https://github.com/theAgingApprentice/christmasTree
