const { binary, deviceAddCustomCluster, temperature, humidity, co2, pressure, numeric } = require("zigbee-herdsman-converters/lib/modernExtend");

const { logger } = require("zigbee-herdsman-converters/lib/logger");
const exposes = require("zigbee-herdsman-converters/lib/exposes");
const utils = require("zigbee-herdsman-converters/lib/utils");
const e = exposes.presets;

const NS = "zhc:botuk";

// =============================================================================
// LED HELPER FUNCTIONS
// =============================================================================
/**
 * Create custom clusters for the specified LEDs
 */
const createCustomClustersLED = (ledColors) => {
    const clusters = {
        red: { name: "redLight", ID: 0xfc07 },
        yellow: { name: "yellowLight", ID: 0xfc08 },
        green: { name: "greenLight", ID: 0xfc09 },
        white: { name: "whiteLight", ID: 0xfc0a },
    };

    return ledColors.map((color) =>
        deviceAddCustomCluster(clusters[color].name, {
            name: clusters[color].name,
            ID: clusters[color].ID,
            attributes: { onOff: { name: "onOff", ID: 0x0000, type: 0x10 } },
            commands: {},
            commandsResponse: {},
        }),
    );
};

/**
 * Create binary exposes for the specified LEDs
 */
const createBinaryExposes = (ledColors) => {
    const ledInfo = {
        red: { cluster: "redLight", description: "Red LED on/off state" },
        yellow: { cluster: "yellowLight", description: "Yellow LED on/off state" },
        green: { cluster: "greenLight", description: "Green LED on/off state" },
        white: { cluster: "whiteLight", description: "White LED on/off state" },
    };

    return ledColors.map((color) =>
        binary({
            name: `state_${color}`,
            cluster: ledInfo[color].cluster,
            attribute: "onOff",
            valueOn: ["ON", 1],
            valueOff: ["OFF", 0],
            description: ledInfo[color].description,
            access: "ALL",
        }),
    );
};

/**
 * Create fromZigbee converter
 */
const createFromZigbee = (clusterIds) => [
    {
        cluster: clusterIds,
        type: ["attributeReport", "readResponse"],
        convert: (model, msg, publish, options, meta) => {
            const state = msg.data["onOff"] !== undefined ? (msg.data["onOff"] ? "ON" : "OFF") : null;
            if (state) {
                const clusterMap = { 64519: "state_red", 64520: "state_yellow", 64521: "state_green", 64522: "state_white" };
                return { [clusterMap[msg.cluster]]: state };
            }
        },
    },
];

/**
 * Create toZigbee converter
 */
const createToZigbee = (ledColors) => [
    {
        key: ledColors.map((color) => `state_${color}`),
        convertSet: async (entity, key, value, meta) => {
            const clusterMap = {
                state_red: 0xfc07,
                state_yellow: 0xfc08,
                state_green: 0xfc09,
                state_white: 0xfc0a,
            };

            const clusterId = clusterMap[key];
            const on = value.toLowerCase() === "on" ? 1 : 0;

            await entity.write(
                clusterId,
                { 0: { value: on, type: 0x10 } },
                {
                    timeout: 30000,
                    disableDefaultResponse: true,
                },
            );

            return { state: { [key]: value.toUpperCase() } };
        },

        convertGet: async (entity, key, meta) => {
            const clusterMap = {
                state_red: 0xfc07,
                state_yellow: 0xfc08,
                state_green: 0xfc09,
                state_white: 0xfc0a,
            };

            await entity.read(clusterMap[key], ["onOff"], { timeout: 30000 });
        },
    },
];

/**
 * Create configure function
 */
const createConfigure = (clusterIds) => async (device, coordinatorEndpoint) => {
    const endpoint = device.getEndpoint(10);
    for (const cluster of clusterIds) {
        try {
            await endpoint.bind(cluster, coordinatorEndpoint);
            await endpoint.configureReporting(cluster, [
                {
                    attribute: "onOff",
                    minimumReportInterval: 1,
                    maximumReportInterval: 3600,
                    reportableChange: 0,
                },
            ]);
            logger.info(`Configured cluster ${cluster} for ${device.ieeeAddress}`, NS);
        } catch (error) {
            logger.warning(`Failed to configure cluster ${cluster}: ${error}`, NS);
        }
    }
};

// =============================================================================
// AIR QUALITY HELPER FUNCTIONS
// =============================================================================
/**
 * Custom Clusters Definition for Air Quality Sensor
 */
const createCustomClustersAirQuality = () => [
    deviceAddCustomCluster("botukIaqMeas", {
        name: "botukIaqMeas",
        ID: 0xfc04,
        attributes: { measuredValue: { name: "measuredValue", ID: 0x0000, type: 0x21 } },
        commands: {},
        commandsResponse: {},
    }),
    deviceAddCustomCluster("botukVocMeas", {
        name: "botukVocMeas",
        ID: 0xfc05,
        attributes: { measuredValue: { name: "measuredValue", ID: 0x0000, type: 0x39 } },
        commands: {},
        commandsResponse: {},
    }),
    deviceAddCustomCluster("botukGasMeas", {
        name: "botukGasMeas",
        ID: 0xfc06,
        attributes: { measuredValue: { name: "measuredValue", ID: 0x0000, type: 0x39 } },
        commands: {},
        commandsResponse: {},
    }),
    deviceAddCustomCluster("botukIaqAccuracy", {
        name: "botukIaqAccuracy",
        ID: 0xfc0b,
        attributes: { measuredValue: { name: "measuredValue", ID: 0x0000, type: 0x20 } },
        commands: {},
        commandsResponse: {},
    }),
];

// =============================================================================
// DEVICE DEFINITIONS
// =============================================================================
/**
 * Definitions for Botuk devices
 */

// Definition for 16512_125 (2 LEDs: Red, Yellow)
const definition_16512_125 = {
    zigbeeModel: ["16512_125"],
    model: "16512_125",
    vendor: "Botuk",
    description: "ESP32H2 LED status indicator",

    extend: [...createCustomClustersLED(["red", "yellow"]), ...createBinaryExposes(["red", "yellow"])],

    fromZigbee: createFromZigbee([0xfc07, 0xfc08]),
    toZigbee: createToZigbee(["red", "yellow"]),
    configure: createConfigure([0xfc07, 0xfc08]),

    ota: true,
};

// Definition for 16512_127 (4 LEDs: Red, Yellow, Green, White)
const definition_16512_127 = {
    zigbeeModel: ["16512_127"],
    model: "16512_127",
    vendor: "Botuk",
    description: "Router ESP32H2 LED status indicator",

    extend: [...createCustomClustersLED(["red", "yellow", "green", "white"]), ...createBinaryExposes(["red", "yellow", "green", "white"])],

    fromZigbee: createFromZigbee([0xfc07, 0xfc08, 0xfc09, 0xfc0a]),
    toZigbee: createToZigbee(["red", "yellow", "green", "white"]),
    configure: createConfigure([0xfc07, 0xfc08, 0xfc09, 0xfc0a]),

    ota: true,
};

// Definition for 50304_126 (1 LED: Red)
const definition_50304_126 = {
    zigbeeModel: ["50304_126"],
    model: "50304_126",
    vendor: "Botuk",
    description: "ESP32H2 LED status indicator",

    extend: [...createCustomClustersLED(["red"]), ...createBinaryExposes(["red"])],

    fromZigbee: createFromZigbee([0xfc07]),
    toZigbee: createToZigbee(["red"]),
    configure: createConfigure([0xfc07]),

    ota: true,
};

// Definition for 54272_127 (Air Quality Sensor)
const definition_54272_127 = {
    zigbeeModel: ["54272_127"],
    model: "54272_127",
    vendor: "Botuk",
    description: "ESP32H2 Sensor Device Air Quality Sensor",

    extend: [
        temperature(),
        humidity(),
        co2(),
        pressure(),
        ...createCustomClustersAirQuality(),

        numeric({
            name: "iaq_index",
            cluster: "botukIaqMeas",
            attribute: "measuredValue",
            unit: "index",
            access: "STATE_GET",
            precision: 1,
            description: "Measured IAQ index value",
        }),
        numeric({
            name: "voc_index",
            cluster: "botukVocMeas",
            attribute: "measuredValue",
            unit: "ppm",
            access: "STATE_GET",
            precision: 2,
            description: "Measured VOC index value",
        }),
        numeric({
            name: "gas_resistance",
            cluster: "botukGasMeas",
            attribute: "measuredValue",
            unit: "Ohm",
            access: "STATE_GET",
            precision: 0,
            description: "Gas resistance value",
        }),
        numeric({
            name: "iaq_accuracy",
            cluster: "botukIaqAccuracy",
            attribute: "measuredValue",
            unit: "accuracy",
            access: "STATE_GET",
            precision: 0,
            description: "IAQ accuracy value",
        }),
    ],

    fromZigbee: [
        {
            cluster: "botukIaqMeas",
            type: ["attributeReport", "readResponse"],
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
                        eco2: Math.round(iaq * 10 + 500),
                    };
                }
            },
        },
    ],

    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(10);
        const configs = [
            { cluster: 'msTemperatureMeasurement', attr: 'measuredValue', min: 60, max: 3600, change: 10 },
            { cluster: 'msRelativeHumidity', attr: 'measuredValue', min: 60, max: 3600, change: 50 },
            { cluster: 'msCO2', attr: 'measuredValue', min: 60, max: 3600, change: 10 },
            { cluster: 'msPressureMeasurement', attr: 'measuredValue', min: 60, max: 3600, change: 1 },
            { cluster: 'botukIaqMeas', attr: 'measuredValue', min: 60, max: 3600, change: 1 },
            { cluster: 'botukVocMeas', attr: 'measuredValue', min: 60, max: 3600, change: 0.1 },
            { cluster: 'botukGasMeas', attr: 'measuredValue', min: 60, max: 3600, change: 100 },
            { cluster: 'botukIaqAccuracy', attr: 'measuredValue', min: 60, max: 3600, change: 0 },
        ];

        for (const cfg of configs) {
            try {
                await endpoint.bind(cfg.cluster, coordinatorEndpoint);
                // Adding a 100ms pause to let the device catch its breath
                await new Promise(resolve => setTimeout(resolve, 100));

                await endpoint.configureReporting(cfg.cluster, [{
                    attribute: cfg.attr,
                    minimumReportInterval: cfg.min,
                    maximumReportInterval: cfg.max,
                    reportableChange: cfg.change,
                }]);
                logger.info(`Successfully configured ${cfg.cluster}`, NS);
            } catch (error) {
                logger.warning(`Failed to configure ${cfg.cluster}: ${error}`, NS);
            }
        }
    },

    exposes: [
        e.numeric("eco2", exposes.access.STATE).withUnit("ppm").withValueMin(500).withValueMax(5500).withDescription("TVOC-derived CO₂-equivalent"),
        e.text("air_quality", exposes.access.STATE).withDescription("IAQ Classification"),
    ],
    ota: true,
};

//=============================================================================
// EXPORTING DEFINITIONS
// ============================================================================
/**
 * Exporting all definitions
 */
module.exports = [definition_16512_125, definition_16512_127, definition_50304_126, definition_54272_127];
