const ota = require('zigbee-herdsman-converters/lib/ota');

const definition = {
  zigbeeModel: ['49152'], // The model ID from: Device with modelID 'lumi.sens' is not supported.
  model: '49152', // Vendor model number, look on the device for a model number
  vendor: 'Botuk', // Vendor of the device (only used for documentation and startup logging)
  description: 'test end device ota', // Description of the device, copy from vendor site. (only used for documentation and startup logging)
  extend: [],
  meta: {},
  configure: async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
  },
  ota: ota.zigbeeOTA,
};

module.exports = definition;