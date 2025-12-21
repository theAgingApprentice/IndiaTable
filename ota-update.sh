#!/bin/bash

###############################################################################
# OTA Update Script for ESP32 Christmas Tree Project
# 
# Usage: ./ota-update.sh <IP_ADDRESS>
# Example: ./ota-update.sh 192.168.2.100
#
# This script performs an Over-The-Air firmware update to the ESP32 device
# using PlatformIO.
###############################################################################

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to validate IP address format
validate_ip() {
    local ip=$1
    local valid_ip_regex="^([0-9]{1,3}\.){3}[0-9]{1,3}$"
    
    # Check if IP matches basic format
    if [[ ! $ip =~ $valid_ip_regex ]]; then
        return 1
    fi
    
    # Check each octet is between 0-255
    IFS='.' read -ra OCTETS <<< "$ip"
    for octet in "${OCTETS[@]}"; do
        if ((octet < 0 || octet > 255)); then
            return 1
        fi
    done
    
    return 0
}

# Check if IP address argument is provided
if [ -z "$1" ]; then
    echo -e "${RED}ERROR: No IP address provided${NC}"
    echo ""
    echo "Usage: $0 <IP_ADDRESS>"
    echo "Example: $0 192.168.2.100"
    echo ""
    echo "To find your ESP32's IP address, check the serial monitor output"
    echo "or your MQTT broker messages on the christmasTree-log topic."
    exit 1
fi

IP_ADDRESS=$1

# Validate IP address format
if ! validate_ip "$IP_ADDRESS"; then
    echo -e "${RED}ERROR: Invalid IP address format: $IP_ADDRESS${NC}"
    echo ""
    echo "Please provide a valid IPv4 address (e.g., 192.168.2.100)"
    exit 1
fi

# Display update information
echo -e "${GREEN}╔════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║  ESP32 Christmas Tree OTA Update          ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Target IP:${NC} $IP_ADDRESS"
echo -e "${YELLOW}Password:${NC} ChristmasTree2025!"
echo ""
echo "Starting OTA update..."
echo ""

# Perform the OTA update (only build and upload the OTA environment)
platformio run -e esp32dev-ota -t upload --upload-port "$IP_ADDRESS"

# Check the exit status
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ OTA Update Successful!                 ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════╝${NC}"
    echo ""
    echo "The ESP32 device at $IP_ADDRESS has been updated and is rebooting."
else
    echo ""
    echo -e "${RED}╔════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ OTA Update Failed!                     ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Possible causes:"
    echo "  - Device is not reachable at $IP_ADDRESS"
    echo "  - OTA password is incorrect"
    echo "  - Device is not connected to the network"
    echo "  - Firewall blocking the connection"
    exit 1
fi
