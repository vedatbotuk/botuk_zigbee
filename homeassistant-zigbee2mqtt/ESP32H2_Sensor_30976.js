const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const ota = require('zigbee-herdsman-converters/lib/ota');
const e = exposes.presets;

const definition = {
  zigbeeModel: ['30976'],
  model: '30976',
  vendor: 'Botuk',
  description: 'Test router temperature, humidity, and water leak sensor with OTA support',
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

    // Configure reporting for temperature, humidity, and battery
    try {
      await reporting.temperature(endpoint, { min: 10, max: 60, change: 100 });
      await reporting.humidity(endpoint, { min: 10, max: 60, change: 100 });
      await reporting.batteryPercentageRemaining(endpoint, { min: 30, max: 60, change: 0 });
    } catch (error) {
      // Handle reporting configuration failure silently
    }
  },
  exposes: [
    e.temperature(),
    e.humidity(),
    e.battery(),
    e.water_leak(), // Water leakage detection
    e.battery_low(), // Low battery warning
    e.tamper() // Tamper detection
  ],
  ota: ota.zigbeeOTA
};

module.exports = definition;

