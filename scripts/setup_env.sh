# setup_env.sh
#!/bin/bash

# Install dependencies
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# Install ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone -b v5.2.3 --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.sh esp32h2

# Install zigpy
. $HOME/esp/esp-idf/export.sh
pip3 install zigpy