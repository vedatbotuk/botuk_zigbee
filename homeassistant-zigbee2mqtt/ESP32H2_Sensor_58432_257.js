import * as m from 'zigbee-herdsman-converters/lib/modernExtend';

export default {
    zigbeeModel: ['58432_257'],
    model: '58432_257',
    vendor: 'Botuk',
    description: 'Noel Star with temperature and humidity sensor',
    extend: [
        m.humidity({
            reporting: {min: 10, max: 3600, change: 100} // min 10s, max 1h, change 1.0%
        }), 
        m.temperature({
            reporting: {min: 10, max: 3600, change: 10}  // min 10s, max 1h, change 0.1°C
        }), 
        m.onOff({"powerOnBehavior": false})
    ],
    ota: true
};