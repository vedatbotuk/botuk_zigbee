const exposes = require('zigbee-herdsman-converters/lib/exposes');
const ea = exposes.access;

/**
 * Helper: write ON/OFF attribute to custom cluster
 * We use the numeric cluster ID and explicit data type to bypass 
 * some of the internal validation checks.
 */
function channelToZigbee(clusterName, exposeName) {
    return {
        key: [exposeName],
        convertSet: async (entity, key, value, meta) => {
            const on = value === 'ON' || value === 1 || value === true ? 1 : 0;
            
            // Look up the cluster ID from our customClusters definition
            const clusterId = meta.mapped.customClusters[clusterName].ID;

            try {
                // We use the raw attribute ID (0x0000) and type (0x10 = Boolean)
                await entity.write(
                    clusterId,
                    { 0x0000: { value: on, type: 0x10 } },
                    { 
                        manufacturerCode: null, 
                        disableResponse: false, // Set to false to see if device ACKs
                    }
                );

                return { state: { [exposeName]: on ? 'ON' : 'OFF' } };
            } catch (error) {
                throw new Error(`Failed to write ${exposeName} to cluster ${clusterName}: ${error.message}`);
            }
        },
    };
}

/**
 * Helper: parse attribute reports
 */
function channelFromZigbee(clusterName, exposeName) {
    return {
        cluster: clusterName,
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg) => {
            if (msg.data.onOff !== undefined) {
                return {
                    [exposeName]: msg.data.onOff ? 'ON' : 'OFF',
                };
            }
            // Also handle raw attribute ID if name isn't resolved
            if (msg.data[0] !== undefined) {
                return {
                    [exposeName]: msg.data[0] ? 'ON' : 'OFF',
                };
            }
        },
    };
}

module.exports = {
    zigbeeModel: ['50304'],
    model: '50304',
    vendor: 'Botuk',
    description: 'ESP32H2 RGBW LED (custom attribute clusters)',

    customClusters: {
        redLight: {
            ID: 0xFC07,
            attributes: {
                onOff: { ID: 0x0000, type: 0x10 },
            },
        },
        yellowLight: {
            ID: 0xFC08,
            attributes: {
                onOff: { ID: 0x0000, type: 0x10 },
            },
        },
        greenLight: {
            ID: 0xFC09,
            attributes: {
                onOff: { ID: 0x0000, type: 0x10 },
            },
        },
        whiteLight: {
            ID: 0xFC0A,
            attributes: {
                onOff: { ID: 0x0000, type: 0x10 },
            },
        },
    },

    toZigbee: [
        channelToZigbee('redLight', 'state_red'),
        channelToZigbee('yellowLight', 'state_yellow'),
        channelToZigbee('greenLight', 'state_green'),
        channelToZigbee('whiteLight', 'state_white'),
    ],

    fromZigbee: [
        channelFromZigbee('redLight', 'state_red'),
        channelFromZigbee('yellowLight', 'state_yellow'),
        channelFromZigbee('greenLight', 'state_green'),
        channelFromZigbee('whiteLight', 'state_white'),
    ],

    exposes: [
        exposes.binary('state_red', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('Red channel'),
        exposes.binary('state_yellow', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('Yellow channel'),
        exposes.binary('state_green', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('Green channel'),
        exposes.binary('state_white', ea.STATE_SET, 'ON', 'OFF')
            .withDescription('White channel'),
    ],
};