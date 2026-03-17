const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const ota = require('zigbee-herdsman-converters/lib/ota');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
  zigbeeModel: ['52352_257'],
  model: '52352',
  vendor: 'Botuk',
  description: 'Builtin flashing LED for visual indication',
  fromZigbee: [
    fz.on_off,
    fz.light_color,
    fz.battery,
  ],
  toZigbee: [
    tz.on_off,
    tz.light_color,
  ],

  configure: async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
    if (!endpoint) return;

    const bindClusters = [
      'genOnOff',
      'lightingColorCtrl',
      'genPowerCfg',
    ];

    try {
      await reporting.bind(endpoint, coordinatorEndpoint, bindClusters);
    } catch (error) {
      // ignore binding failure
    }

    try {
      await reporting.batteryPercentageRemaining(endpoint, {min: 600, max: 65000, change: 1});
      await reporting.onOff(endpoint, {min: 0, max: 3600, change: 0});
    } catch (error) {
      // ignore reporting failure
    }
  },

  exposes: [
    e.switch(),
    e.battery(),
    e.color_hs(),             // <-- REQUIRED COLOR EXPOSE
  ],

  ota: true,
};

module.exports = definition;
