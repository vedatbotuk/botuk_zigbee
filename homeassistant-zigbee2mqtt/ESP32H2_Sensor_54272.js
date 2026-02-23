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
        attributes: { measuredValue: { ID: 0x0000, type: 0x21 } },
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
    }),
    deviceAddCustomCluster('botukIaqAccuracy', {
        ID: 0xFC0B,
        attributes: { measuredValue: { ID: 0x0000, type: 0x20 } },
        commands: {}, commandsResponse: {},
    })
];

const definition = {
    zigbeeModel: ['54272'],
    model: '54272',
    vendor: 'Botuk',
    description: 'ESP32H2 Sensor Device Air Quality Sensor',

    extend: [
        temperature({
            // reporting: { min: 60, max: 3600, change: 10 }
        }),
        humidity({
            // reporting: { min: 60, max: 3600, change: 10 }
        }),
        co2({
            // reporting: { min: 60, max: 3600, change: 10 }
        }),
        pressure({
            // reporting: { min: 60, max: 3600, change: 1 }
        }),
        ...addCustomClusters(),

        numeric({
            name: 'iaq_index',
            cluster: 'botukIaqMeas',
            attribute: 'measuredValue',
            unit: 'index',
            access: 'STATE_GET',
            precision: 1,
            // reporting: { min: 60, max: 3600, change: 1 },
            description: 'Measured IAQ index value'
        }),
        numeric({
            name: 'voc_index',
            cluster: 'botukVocMeas',
            attribute: 'measuredValue',
            unit: 'ppm',
            access: 'STATE_GET',
            precision: 2,
            // reporting: { min: 60, max: 3600, change: 0.1 },
            description: 'Measured VOC index value'
        }),
        numeric({
            name: 'gas_resistance',
            cluster: 'botukGasMeas',
            attribute: 'measuredValue',
            unit: 'Ohm',
            access: 'STATE_GET',
            precision: 0,
            // reporting: { min: 60, max: 3600, change: 100 },
            description: 'Gas resistance value'
        }),
        numeric({
            name: 'iaq_accuracy',
            cluster: 'botukIaqAccuracy',
            attribute: 'measuredValue',
            unit: 'accuracy',
            access: 'STATE_GET',
            precision: 0,
            // reporting: { min: 60, max: 3600, change: 1 },
            description: 'IAQ accuracy value'
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

    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(10);

        // await reporting.bind(endpoint, coordinatorEndpoint, [
        //     'msTemperatureMeasurement',
        //     'msRelativeHumidity',
        //     'msCO2',
        //     'msPressureMeasurement',
        //     'botukIaqMeas',
        //     'botukVocMeas',
        //     'botukGasMeas',
        //     'botukIaqAccuracy'
        // ]);

        await endpoint.configureReporting('msTemperatureMeasurement', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 10,
        }]);

        await endpoint.configureReporting('msRelativeHumidity', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 50,
        }]);

        await endpoint.configureReporting('msCO2', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 10,
        }]);

        await endpoint.configureReporting('msPressureMeasurement', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 1,
        }]);

        await endpoint.configureReporting('msPressureMeasurement', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 1,
        }]);

        await endpoint.configureReporting('botukIaqMeas', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 1,
        }]);

        await endpoint.configureReporting('botukVocMeas', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 0.1,
        }]);

        await endpoint.configureReporting('botukGasMeas', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 100,
        }]);

        await endpoint.configureReporting('botukIaqAccuracy', [{
            attribute: 'measuredValue',
            minimumReportInterval: 60,
            maximumReportInterval: 3600,
            reportableChange: 0,
        }]);
    },

    exposes: [
        e.numeric('eco2', exposes.access.STATE)
            .withUnit('ppm')
            .withValueMin(500)
            .withValueMax(5500)
            .withDescription('TVOC-derived CO₂-equivalent'),
        e.text('air_quality', exposes.access.STATE)
            .withDescription('IAQ Classification'),
    ],
    ota: true,
};

module.exports = definition;