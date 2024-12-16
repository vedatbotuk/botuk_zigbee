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

# Create a new file for the index
touch "$BOTUK_INDEX"
# Start the JSON array
echo "[" >> "$BOTUK_INDEX"


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
HW_VERSION=$(grep -oP '^HW_VERSION=\K.*' settings.conf)

echo ""
echo "----------------------------------------"
echo "Starting the build process..."
echo "----------------------------------------"
{
  read  # Skip the header line
  while IFS=',' read -r MODEL_ID DEVICE_NAME COMMENT || [ -n "$MODEL_ID" ]; do
      # Remove leading/trailing whitespaces
      MODEL_ID=$(echo "$MODEL_ID" | xargs)

      # Update the settings.conf file with the new MODEL_ID
      echo "Updating settings.conf with MODEL_ID: $MODEL_ID"
      sed -i "s/^MODEL_ID=.*/MODEL_ID=$MODEL_ID/" "$SETTINGS_FILE"

      # Set target for ESP32H2
      idf.py set-target esp32h2 > /dev/null 2>&1
      # Build
      idf.py build > /dev/null 2>&1

      # Clean up
      echo "Removing build directory and related files..."
      idf.py fullclean
      rm -f sdkconfig sdkconfig.old sdkconfig.defaults dependencies.lock || true
      echo "Clean up completed."

      # Add the data to OTA Index file
      echo "Adding data to OTA Index file..."

      # Convert MODEL_ID to decimal and store it in a variable
      MODEL_ID_DECIMAL=$((2#$MODEL_ID))

      # Format MODEL_ID_DECIMAL to be 5 digits long, with leading zeros if necessary
      MODEL_ID_DECIMAL_FIVE=$(printf "%05d" "$MODEL_ID_DECIMAL")

      # Create FILE_NAME in the desired format
      FILE_NAME="device_${MODEL_ID_DECIMAL_FIVE}_v${VERSION}prod.ota"
      FILE_PATH="ota/$FILE_NAME"

      # Get other metadata values
      FILE_SIZE=$(stat -c%s "$FILE_PATH") # Get file size
      SHA512=$(sha512sum "$FILE_PATH" | awk '{print $1}') # Calculate sha512 hash

      # Create a JSON object for the current iteration
      echo "  {" >> "$BOTUK_INDEX"
      echo "    \"fileName\": \"$FILE_NAME\"," >> "$BOTUK_INDEX"
      echo "    \"fileVersion\": $VERSION," >> "$BOTUK_INDEX"
      echo "    \"fileSize\": $FILE_SIZE," >> "$BOTUK_INDEX"
      echo "    \"manufacturerCode\": $MANUFACTURER," >> "$BOTUK_INDEX"
      echo "    \"imageType\": $MODEL_ID_DECIMAL," >> "$BOTUK_INDEX"
      echo "    \"sha512\": \"$SHA512\"," >> "$BOTUK_INDEX"
      echo "    \"url\": \"https://github.com/vedatbotuk/zigbee-with-esp32h2/releases/download/${FIRMWARE_VERSION}_${VERSION}/${FILE_NAME}\"," >> "$BOTUK_INDEX"
      echo "    \"otaHeaderString\": \"\"" >> "$BOTUK_INDEX"
      echo "  }," >> "$BOTUK_INDEX"

      # Indicate completion of the build
      echo "Build completed for MODEL_ID: $MODEL_ID"
      echo "----------------------------------------"
  done
} < "$CSV_FILE"

# Remove the last comma added after the last JSON object
sed -i '$ s/,$//' "$BOTUK_INDEX"
echo "]" >> "$BOTUK_INDEX"
echo "JSON file created as $BOTUK_INDEX"

# Restore the original settings.conf from the backup
mv "$BACKUP_FILE" "$SETTINGS_FILE"
echo ""
echo "settings.conf has been reset to its original state."
echo "Finished building all devices in the CSV file."
echo "OTA Files are in ./ota"
echo "----------------------------------------"
echo "OTA Build Script for Production Devices completed."
