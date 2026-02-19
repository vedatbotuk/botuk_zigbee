const {
    temperature,
    humidity,
    co2,
    pressure,
    numeric,
    deviceAddCustomCluster
} = require('zigbee-herdsman-converters/lib/modernExtend');

const { logger } = require('zigbee-herdsman-converters/lib/logger');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const utils = require('zigbee-herdsman-converters/lib/utils');
const e = exposes.presets;

const NS = 'zhc:botuk';

/**
 * Custom Clusters Definition
 */
const addCustomClusters = () => [
    deviceAddCustomCluster('botukIaqMeas', {
        ID: 0xFC04,
        attributes: { measuredValue: { ID: 0x0000, type: 0x39 } }, // SINGLE_PREC
        commands: {}, commandsResponse: {},
    }),
    deviceAddCustomCluster('botukVocMeas', {
        ID: 0xFC05,
        attributes: { measuredValue: { ID: 0x0000, type: 0x39 } },
        commands: {}, commandsResponse: {},
    }),
    deviceAddCustomCluster('botukGasMeas', {
        ID: 0xFC06,
        attributes: { measuredValue: { ID: 0x0000, type: 0x39 } },
        commands: {}, commandsResponse: {},
    })
];

const definition = {
    zigbeeModel: ['37888'],
    model: '37888',
    vendor: 'Botuk',
    description: 'ESP32H2 Sensor Device Air Quality Sensor',

    extend: [
        temperature({
            reporting: { min: 10, max: 3600, change: 10 }
        }),
        humidity({
            reporting: { min: 10, max: 3600, change: 10 }
        }),
        co2({
            reporting: { min: 10, max: 3600, change: 10 }
        }),
        pressure({
            reporting: { min: 10, max: 3600, change: 10 }
        }),
        ...addCustomClusters(),

        numeric({
            name: 'iaq_index',
            cluster: 'botukIaqMeas',
            attribute: 'measuredValue',
            unit: 'index',
            access: 'STATE_GET',
            precision: 1,
            reporting: { min: 1, max: 3600, change: 1 },
            description: 'Measured IAQ index value'
        }),
        numeric({
            name: 'voc_index',
            cluster: 'botukVocMeas',
            attribute: 'measuredValue',
            unit: 'ppm',
            access: 'STATE_GET',
            precision: 2,
            reporting: { min: 1, max: 3600, change: 0.1 },
            description: 'Measured VOC index value'
        }),
        numeric({
            name: 'gas_resistance',
            cluster: 'botukGasMeas',
            attribute: 'measuredValue',
            unit: 'Ohm',
            access: 'STATE_GET',
            precision: 0,
            reporting: { min: 1, max: 3600, change: 10 },
            description: 'Gas resistance value'
        }),
    ],

    fromZigbee: [{
        cluster: 'botukIaqMeas',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            if (msg.data.measuredValue !== undefined) {
                const iaq = utils.toNumber(msg.data.measuredValue);

                // Classification Logic
                let classification = "Error";
                if (iaq <= 50) classification = "Excellent";
                else if (iaq <= 100) classification = "Good";
                else if (iaq <= 150) classification = "Lightly polluted";
                else if (iaq <= 200) classification = "Moderately polluted";
                else if (iaq <= 250) classification = "Heavily polluted";
                else if (iaq <= 350) classification = "Severely polluted";
                else if (iaq <= 500) classification = "Extremely polluted";

                return {
                    air_quality: classification,
                    eco2: Math.round(iaq * 10 + 500)
                };
            }
        },
    }],

    exposes: [
        e.numeric('eco2', exposes.access.STATE)
            .withUnit('ppm')
            .withValueMin(500)
            .withValueMax(5500)
            .withDescription('TVOC-derived CO₂-equivalent'),
        e.text('air_quality', exposes.access.STATE)
            .withDescription('IAQ Classification'),
    ],

    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(1);
        const clusters = [0xFC04, 0xFC05, 0xFC06];
        for (const cluster of clusters) {
            try {
                await endpoint.bind(cluster, coordinatorEndpoint);
                await endpoint.configureReporting(cluster, [{
                    attribute: 'measuredValue',
                    minimumReportInterval: 1,
                    maximumReportInterval: 3600,
                    reportableChange: 0.1
                }]);
                logger.info(`Configured cluster ${cluster} for ${device.ieeeAddress}`, NS);
            } catch (error) {
                logger.warning(`Failed to configure cluster ${cluster}: ${error}`, NS);
            }
        }
    },
};

module.exports = definition;