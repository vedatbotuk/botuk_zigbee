const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const ota = require('zigbee-herdsman-converters/lib/ota');
const e = exposes.presets;

const definition = {
  zigbeeModel: ['31488'],
  model: '31488',
  vendor: 'Botuk',
  description: 'Test router deep-sleep, temperature, humidity, and water leak sensor with OTA support',
  fromZigbee: [
    fz.temperature,
    fz.humidity,
    fz.ias_water_leak_alarm_1,
    fz.battery
  ],
  toZigbee: [], // No specific commands to send for this device
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

