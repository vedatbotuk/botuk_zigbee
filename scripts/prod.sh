#!/bin/bash

echo "OTA Build Script for Production Devices"
echo "----------------------------------------"

# Define the CSV file and the settings.conf file
CSV_FILE="prod_devices.csv"
SETTINGS_FILE="settings.conf"
BACKUP_FILE="settings.conf.cache"
BOTUK_INDEX="ota/botuk_index.json"

# Create the ota directory if it does not exist
mkdir -p ota || true
echo "OTA directory created."

# Check if the CSV file exists
if [ ! -f "$CSV_FILE" ]; then
  echo "CSV file $CSV_FILE does not exist. Exiting script."
  exit 1
fi

# Check if the settings.conf file exists
if [ ! -f "$SETTINGS_FILE" ]; then
  echo "Settings file $SETTINGS_FILE does not exist. Exiting script."
  exit 1
fi

# Backup the original settings.conf
cp "$SETTINGS_FILE" "$BACKUP_FILE"
echo "Backup of ${SETTINGS_FILE} created as ${BACKUP_FILE}."

# Check if the file exists
if [[ -f "$BOTUK_INDEX" ]]; then
  # Get the current timestamp
  TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

  # Create a backup copy with the timestamp
  cp "$BOTUK_INDEX" "${BOTUK_INDEX%.json}_backup_$TIMESTAMP.json"
  echo "Backup created: ${BOTUK_INDEX%.json}_backup_$TIMESTAMP.json"
else
  echo "$BOTUK_INDEX does not exist, no backup needed."
fi

# Check if DEV_PROD exists and change it to prod in settings.conf only once
if grep -q "^DEV_PROD=" "$SETTINGS_FILE"; then
    echo "Changing DEV_PROD to prod in settings.conf"
    sed -i "s/^DEV_PROD=.*/DEV_PROD=prod/" "$SETTINGS_FILE"
else
  echo "DEV_PROD key does not exist in settings.conf. Exiting script."
  exit 1
fi

# Source the ESP-IDF environment script
echo "Sourcing the ESP-IDF environment..."
if . "$HOME/esp/esp-idf/export.sh" > /dev/null 2>&1; then
    echo "ESP-IDF environment sourced successfully."
else
    echo "Failed to source ESP-IDF environment. Please check the path and try again."
    exit 1
fi

# Extract each variable from settings.conf
FIRMWARE_VERSION=$(grep -oP '^FIRMWARE_VERSION=\K.*' settings.conf)
VERSION=$(grep -oP '^VERSION=\K.*' settings.conf)
MANUFACTURER=$(grep -oP '^MANUFACTURER=\K.*' settings.conf)

echo ""
echo "----------------------------------------"
echo "Starting the build process..."
echo "----------------------------------------"
{
  read  # Skip the header line
  while IFS=',' read -r MODEL_ID MODEL_ID_DECIMAL HW_VERSION DEVICE_NAME COMMENT || [ -n "$MODEL_ID" ]; do
      # Remove leading/trailing whitespaces
      MODEL_ID=$(echo "$MODEL_ID" | xargs)
      HW_VERSION=$(echo "$HW_VERSION" | xargs)

      # Update the settings.conf file with the new MODEL_ID
      echo "Updating settings.conf with MODEL_ID: $MODEL_ID"
      echo "Updating settings.conf with MODEL_ID_DECIMAL: $MODEL_ID_DECIMAL"
      sed -i "s/^MODEL_ID=.*/MODEL_ID=$MODEL_ID/" "$SETTINGS_FILE"

      # Update the settings.conf file with the new HW_VERSION
      echo "Updating settings.conf with HW_VERSION: $HW_VERSION"
      sed -i "s/^HW_VERSION=.*/HW_VERSION=$HW_VERSION/" "$SETTINGS_FILE"

      # Set target for ESP32H2
      idf.py set-target esp32h2 > /dev/null 2>&1
      # Build
      idf.py build > /dev/null 2>&1

      # Clean up
      echo "Removing build directory and related files..."
      idf.py fullclean
      rm -f sdkconfig sdkconfig.old sdkconfig.defaults dependencies.lock || true
      echo "Clean up completed."

      echo "Build completed for MODEL_ID: $MODEL_ID"
      echo "----------------------------------------"
  done
} < "$CSV_FILE"

echo "Building completed. Generating OTA Index file..."
echo "----------------------------------------"

# Phase 2: Scan the ota/ directory and generate index.json
# Create a new file for the index
touch "$BOTUK_INDEX"
echo "[" > "$BOTUK_INDEX"

# Enable nullglob so the loop behaves correctly if no .ota files exist
shopt -s nullglob
OTA_FILES=(ota/*.ota)
TOTAL_FILES=${#OTA_FILES[@]}
CURRENT_FILE=0

for FILE_PATH in "${OTA_FILES[@]}"; do
    CURRENT_FILE=$((CURRENT_FILE+1))
    FILE_NAME=$(basename "$FILE_PATH")

    # Filename: 50304_126_9_to_10_prod.ota
    # Segments: 1(50304) 2(126) 3(9) 4(to) 5(10) 6(prod)
    
    IMG_TYPE_VAL=$(echo "$FILE_NAME" | cut -d'_' -f1)
    HW_VERSION=$(echo "$FILE_NAME" | cut -d'_' -f2)
    BASE_VER_VAL=$(echo "$FILE_NAME" | cut -d'_' -f3)  # Extract '9'
    NEW_VER_VAL=$(echo "$FILE_NAME" | cut -d'_' -f5)   # Extract '10'

    # Get other metadata values
    FILE_SIZE=$(stat -c%s "$FILE_PATH") # Get file size
    SHA512=$(sha512sum "$FILE_PATH" | awk '{print $1}') # Calculate sha512 hash

    # Append JSON object
    echo "  {" >> "$BOTUK_INDEX"
    echo "    \"fileName\": \"$FILE_NAME\"," >> "$BOTUK_INDEX"
    echo "    \"fileVersion\": $NEW_VER_VAL," >> "$BOTUK_INDEX"
    echo "    \"fileSize\": $FILE_SIZE," >> "$BOTUK_INDEX"
    echo "    \"manufacturerCode\": $MANUFACTURER," >> "$BOTUK_INDEX"
    echo "    \"imageType\": $IMG_TYPE_VAL," >> "$BOTUK_INDEX"
    echo "    \"hardwareVersion\": $HW_VERSION," >> "$BOTUK_INDEX"
    echo "    \"minFileVersion\": $BASE_VER_VAL," >> "$BOTUK_INDEX"
    echo "    \"maxFileVersion\": $BASE_VER_VAL," >> "$BOTUK_INDEX"
    echo "    \"sha512\": \"$SHA512\"," >> "$BOTUK_INDEX"
    echo "    \"url\": \"https://github.com/vedatbotuk/zigbee-with-esp32h2/releases/download/${FIRMWARE_VERSION}_${VERSION}/${FILE_NAME}\"" >> "$BOTUK_INDEX"
    echo "    \"otaHeaderString\": \"\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\\u0000\"" >> "$BOTUK_INDEX"

    if [ "$CURRENT_FILE" -eq "$TOTAL_FILES" ]; then
        echo "  }" >> "$BOTUK_INDEX"
    else
        echo "  }," >> "$BOTUK_INDEX"
    fi
done

echo "]" >> "$BOTUK_INDEX"
echo "JSON file created as $BOTUK_INDEX"

# Restore the original settings.conf from the backup
mv "$BACKUP_FILE" "$SETTINGS_FILE"
echo ""
echo "settings.conf has been reset to its original state."
echo "Finished building all devices in the CSV file and mapping all OTA updates."
echo "OTA Files are in ./ota"
echo "----------------------------------------"
echo "OTA Build Script for Production Devices completed."