# Base Image
FROM ubuntu:latest

# Set HOME environment variable
ENV HOME=/root

# Define ARGs for maintainability
ARG ESP_IDF_VERSION=5.3.2
ARG ESP_IDF_ZIP_URL=https://dl.espressif.com/github_assets/espressif/esp-idf/releases/download/v${ESP_IDF_VERSION}/esp-idf-v${ESP_IDF_VERSION}.zip
ARG RELEASE_CLI_URL=https://gitlab.com/api/v4/projects/gitlab-org%2Frelease-cli/packages/generic/release-cli/latest/release-cli-linux-amd64

# Install required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget unzip flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    jq ca-certificates git curl && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Download and set up ESP-IDF
RUN mkdir -p ${HOME}/esp && \
    wget -O ${HOME}/esp/esp-idf.zip ${ESP_IDF_ZIP_URL} && \
    unzip ${HOME}/esp/esp-idf.zip -d ${HOME}/esp && \
    mv ${HOME}/esp/esp-idf-v${ESP_IDF_VERSION} ${HOME}/esp/esp-idf && \
    rm ${HOME}/esp/esp-idf.zip

# Install ESP-IDF dependencies
RUN ${HOME}/esp/esp-idf/install.sh esp32h2

# Set environment variables for ESP-IDF
ENV IDF_PATH=${HOME}/esp/esp-idf
ENV PATH="${IDF_PATH}/tools:${PATH}"

# Install Zigpy with ESP-IDF's Python environment
RUN . ${IDF_PATH}/export.sh && pip3 install zigpy

# Copy and install self-signed certificate
COPY ./botuk.crt /usr/local/share/ca-certificates/
RUN chmod 644 /usr/local/share/ca-certificates/botuk.crt && \
    update-ca-certificates

# Configure Git to use updated CA certificates
RUN git config --global http.sslCAInfo /etc/ssl/certs/ca-certificates.crt

# Install release-cli
RUN curl --location --output /usr/local/bin/release-cli "${RELEASE_CLI_URL}" && \
    chmod +x /usr/local/bin/release-cli

# Install yq
RUN wget -qO /usr/local/bin/yq https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64
RUN chmod a+x /usr/local/bin/yq

# Set the working directory
WORKDIR /workspace
