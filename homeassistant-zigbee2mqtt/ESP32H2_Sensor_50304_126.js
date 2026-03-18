const {
    binary,
    deviceAddCustomCluster
} = require('zigbee-herdsman-converters/lib/modernExtend');

const { logger } = require('zigbee-herdsman-converters/lib/logger');
const reporting = require('zigbee-herdsman-converters/lib/reporting');

const NS = 'zhc:botuk';

/**
 * Custom Clusters Definition
 */
const addCustomClusters = () => [
    deviceAddCustomCluster('redLight', {
        ID: 0xFC07,
        attributes: { onOff: { ID: 0x0000, type: 0x10 } },
        commands: {}, commandsResponse: {},
    })
];

const definition = {
    zigbeeModel: ['50304_126'],
    model: '50304_126',
    vendor: 'Botuk',
    description: 'ESP32H2 LED status indicator',

    extend: [
        ...addCustomClusters(),

        // Using name: 'state_red' here creates the 'state_red' expose automatically
        binary({
            name: 'state_red',
            cluster: 'redLight',
            attribute: 'onOff',
            valueOn: ['ON', 1],
            valueOff: ['OFF', 0],
            description: 'Red LED on/off state',
            // reporting: { min: 1, max: 3600, change: 1 },
            access: 'ALL', // This enables GET, SET, and STATE (reporting)
        })
    ],

    fromZigbee: [{
        cluster: /.*Light/, // Matches redLight, yellowLight, etc.
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const state = msg.data['onOff'] !== undefined ? (msg.data['onOff'] ? 'ON' : 'OFF') : null;
            if (state) {
                // Map cluster ID back to our state name
                const clusterMap = { 0xFC07: 'state_red', 0xFC08: 'state_yellow', 0xFC09: 'state_green', 0xFC0A: 'state_white' };
                return { [clusterMap[msg.cluster]]: state };
            }
        },
    }],

    toZigbee: [{
        key: ['state_red', 'state_yellow', 'state_green', 'state_white'],
        convertSet: async (entity, key, value, meta) => {
            const clusterMap = {
                'state_red': 0xFC07
            };

            const clusterId = clusterMap[key];
            const on = value.toLowerCase() === 'on' ? 1 : 0;

            await entity.write(
                clusterId,
                { 0x0000: { value: on, type: 0x10 } },
                {
                    timeout: 30000,
                    disableDefaultResponse: true
                }
            );

            return { state: { [key]: value.toUpperCase() } };
        },

        convertGet: async (entity, key, meta) => {
            const clusterMap = {
                'state_red': 0xFC07
            };

            await entity.read(
                clusterMap[key],
                ['onOff'],
                { timeout: 30000 }
            );
        },
    }],

    // TODO: This we dont nedd
    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(10);
        const clusters = [0xFC07];
        for (const cluster of clusters) {
            try {
                await endpoint.bind(cluster, coordinatorEndpoint);
                await endpoint.configureReporting(cluster, [{
                    attribute: 'onOff',
                    minimumReportInterval: 1,
                    maximumReportInterval: 3600,
                    reportableChange: 0
                }]);
                logger.info(`Configured cluster ${cluster} for ${device.ieeeAddress}`, NS);
            } catch (error) {
                logger.warning(`Failed to configure cluster ${cluster}: ${error}`, NS);
            }
        }
    },

    ota: true
};


module.exports = definition;