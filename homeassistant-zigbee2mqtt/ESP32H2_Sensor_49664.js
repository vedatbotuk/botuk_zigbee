const definition = {
  zigbeeModel: ['49664'], // The model ID from: Device with modelID 'lumi.sens' is not supported.
  model: '49664', // Vendor model number, look on the device for a model number
  vendor: 'Botuk', // Vendor of the device (only used for documentation and startup logging)
  description: 'test end device deep-sleep', // Description of the device, copy from vendor site. (only used for documentation and startup logging)
  extend: [],
  meta: {},
  configure: async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
  }
};

module.exports = definition;