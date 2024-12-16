const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const ota = require('zigbee-herdsman-converters/lib/ota');
const e = exposes.presets;

const definition = {
  zigbeeModel: ['64256'],
  model: '64256',
  vendor: 'Botuk',
  description: 'Test router deep-sleep, temperature, humidity, and water leak sensor with OTA support',
  fromZigbee: [
    fz.temperature,
    fz.humidity,
    fz.ias_water_leak_alarm_1,
    fz.battery
  ],
  toZigbee: [], // No specific commands to send for this device
  configure: async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
    const bindClusters = [
      'msTemperatureMeasurement',
      'msRelativeHumidity',
      'genPowerCfg'
    ];

    if (!endpoint) {
      return; // Endpoint not available; cannot proceed with configuration
    }

    // Bind clusters to ensure proper reporting
    try {
      await reporting.bind(endpoint, coordinatorEndpoint, bindClusters);
    } catch (error) {
      // Handle binding failure silently
    }
  },
  exposes: [
    e.temperature(),
    e.humidity(),
    e.battery(),
    e.water_leak(), // Water leakage detection
  ],
  ota: ota.zigbeeOTA
};

module.exports = definition;

