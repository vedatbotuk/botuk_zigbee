const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const ota = require('zigbee-herdsman-converters/lib/ota');
const e = exposes.presets;

const definition = {
  zigbeeModel: ['64512'],
  model: '64512',
  vendor: 'Botuk',
  description: 'end device Temp/Hum Sensor with ota',
  fromZigbee: [
    fz.temperature,
    fz.humidity,
    fz.battery
  ],
  toZigbee: [],

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

    // Configure reporting for temperature, humidity, and battery
    try {
      await reporting.temperature(endpoint, { min: 30, max: 120, change: 10 });
      await reporting.humidity(endpoint, { min: 30, max: 120, change: 10 });
      await reporting.batteryPercentageRemaining(endpoint, { min: 60, max: 300, change: 1 });
    } catch (error) {
      // Handle reporting configuration failure silently
    }
  },
  exposes: [
    e.temperature(),
    e.humidity(),
    e.battery()
  ],
  ota: ota.zigbeeOTA
};

module.exports = definition;
