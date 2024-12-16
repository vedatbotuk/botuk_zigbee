const ota = require('zigbee-herdsman-converters/lib/ota');

const definition = {
  zigbeeModel: ['16384'], // The model ID from: Device with modelID 'lumi.sens' is not supported.
  model: '16384', // Vendor model number, look on the device for a model number
  vendor: 'Botuk', // Vendor of the device (only used for documentation and startup logging)
  description: 'router device', // Description of the device, copy from vendor site. (only used for documentation and startup logging)
  extend: [],
  meta: {},
  configure: async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
  },
  ota: ota.zigbeeOTA,
};

module.exports = definition;