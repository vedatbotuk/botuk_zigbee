const {
    temperature,
    humidity,
    co2,
} = require('zigbee-herdsman-converters/lib/modernExtend');

const { logger } = require('zigbee-herdsman-converters/lib/logger');
const utils = require('zigbee-herdsman-converters/lib/utils');

const NS = 'zhc:sensirion';

// Define Custom Cluster IDs
const CLUSTER_IAQ = 0xFC04;
const CLUSTER_VOC = 0xFC05;
const CLUSTER_GAS = 0xFC06;

const definition = {
    zigbeeModel: ['37888'],
    model: '37888',
    vendor: 'Botuk',
    description: 'ESP32H2 Sensor Device Air Quality Sensor',

    // Standard extends for standard clusters
    extend: [
        temperature(),
        humidity(),
        co2(),
    ],

    // Manual converter to handle Custom Clusters by ID
    fromZigbee: [
        {
            // We listen to all attribute reports and filter by cluster ID inside
            cluster: 'manuSpecificTuya', // Placeholder to ensure this runs (or omit if relying on generic)
            // Better approach: Use a custom matcher that matches the raw Cluster ID
            // Z2M allows matching by stringified Cluster ID if name is unknown
            cluster: [String(CLUSTER_IAQ), String(CLUSTER_VOC), String(CLUSTER_GAS), '64516', '64517', '64518'],
            type: ['attributeReport', 'readResponse'],
            convert: (model, msg, publish, options, meta) => {
                const result = {};
                const clusterId = msg.cluster; // This might be number or string

                // Helper to check ID match
                const isCluster = (id) => clusterId == id || clusterId === String(id);

                // 1. IAQ Cluster (0xFC04 / 64516)
                if (isCluster(CLUSTER_IAQ) && msg.data.measuredValue !== undefined) {
                    const iaq = utils.toNumber(msg.data.measuredValue);
                    result.iaq_index = iaq;
                    
                    // TVOC-derived CO₂-equivalent calculation
                    const factorCo2 = 10;
                    result.eco2 = iaq * factorCo2 + 500;
                }

                // 2. VOC Cluster (0xFC05 / 64517)
                if (isCluster(CLUSTER_VOC) && msg.data.measuredValue !== undefined) {
                    result.voc_index = utils.toNumber(msg.data.measuredValue);
                }

                // 3. Gas Resistance Cluster (0xFC06 / 64518)
                if (isCluster(CLUSTER_GAS) && msg.data.measuredValue !== undefined) {
                    result.gas_resistance = utils.toNumber(msg.data.measuredValue);
                }

                return result;
            },
        },
    ],

    // Expose the data points to Home Assistant / MQTT
    exposes: [
        // Standard extends handle temp/hum/co2 exposes automatically.
        // We manually add the others:
        (e) => {
            const expose = require('zigbee-herdsman-converters/lib/exposes');
            return [
                expose.numeric('iaq_index', 'state').withUnit('index').withDescription('Measured IAQ index value'),
                expose.numeric('voc_index', 'state').withUnit('index').withDescription('Measured VOC index value'),
                expose.numeric('eco2', 'state').withUnit('ppm').withDescription('TVOC-derived CO₂-equivalent'),
                expose.numeric('gas_resistance', 'state').withUnit('Ohm').withDescription('Gas resistance value'),
            ];
        }
    ].flat(),

    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(1);
        const customClusters = [CLUSTER_IAQ, CLUSTER_VOC, CLUSTER_GAS];

        // 1. Manually add clusters to the device (Client/Server) to ensure Z2M knows them
        // This is the fallback for deviceAddCustomCluster
        for (const clusterId of customClusters) {
            if (!endpoint.inputClusters.includes(clusterId)) {
                endpoint.inputClusters.push(clusterId);
            }
        }
        
        device.save();

        // 2. Bind and Configure Reporting
        for (const cluster of customClusters) {
            try {
                await endpoint.bind(cluster, coordinatorEndpoint);
                
                // Configure Reporting Manually
                // Payload: Attribute 0x0000, Type 0x39 (Single Precision Float), Min 1, Max 3600, Change 1
                await endpoint.configureReporting(cluster, [{
                    attribute: { ID: 0x0000, type: 0x39 }, 
                    minimumReportInterval: 1,
                    maximumReportInterval: 3600,
                    reportableChange: 1,
                }]);
                
                logger.info(`Configured custom cluster ${cluster} for ${device.ieeeAddress}`, NS);
            } catch (error) {
                logger.warning(`Failed to configure cluster ${cluster}: ${error}`, NS);
            }
        }
    },
};

module.exports = definition;