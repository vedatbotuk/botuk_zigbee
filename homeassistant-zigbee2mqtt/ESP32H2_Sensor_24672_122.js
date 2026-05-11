const {binary, deviceAddCustomCluster, temperature, humidity} = require("zigbee-herdsman-converters/lib/modernExtend");

const {logger} = require("zigbee-herdsman-converters/lib/logger");

const NS = "zhc:botuk";

/**
 * Custom Clusters Definition
 */
const addCustomClusters = () => [
    deviceAddCustomCluster("irrigation", {
        name: "irrigation",
        ID: 0xfc0c,
        attributes: {onOff: {name: "onOff", ID: 0x0000, type: 0x10}},
        commands: {},
        commandsResponse: {},
    }),
];

const definition = {
    zigbeeModel: ["24672_122"],
    model: "24672_122",
    vendor: "Botuk",
    description: "ESP32H2 Irrigation and Light Controller",

    extend: [
        ...addCustomClusters(),
        temperature(),
        humidity(),

        binary({
            name: "state_light",
            cluster: "genOnOff",
            attribute: "onOff",
            valueOn: ["ON", 1],
            valueOff: ["OFF", 0],
            description: "Light on/off",
            // reporting: { min: 1, max: 3600, change: 1 },
            access: "ALL", // This enables GET, SET, and STATE (reporting)
        }),

        // Using name: 'state_red' here creates the 'state_red' expose automatically
        binary({
            name: "state_irrigation",
            cluster: "irrigation",
            attribute: "onOff",
            valueOn: ["ON", 1],
            valueOff: ["OFF", 0],
            description: "Irrigation on/off",
            // reporting: { min: 1, max: 3600, change: 1 },
            access: "ALL", // This enables GET, SET, and STATE (reporting)
        }),
    ],

    fromZigbee: [
        {
            cluster: /.*/,
            type: ["attributeReport", "readResponse"],
            convert: (model, msg, publish, options, meta) => {
                const state = msg.data["onOff"] !== undefined ? (msg.data["onOff"] ? "ON" : "OFF") : null;
                if (state) {
                    // Map cluster ID back to our state name
                    const clusterMap = {64524: "state_irrigation", 6: "state_light"};
                    return {[clusterMap[msg.cluster]]: state};
                }
            },
        },
    ],

    toZigbee: [
        {
            key: ["state_irrigation", "state_light"],
            convertSet: async (entity, key, value, meta) => {
                if (key === "state_light") {
                    const command = value.toLowerCase() === "on" ? "on" : "off";
                    await entity.command(
                        "genOnOff",
                        command,
                        {},
                        {
                            timeout: 30000,
                            disableDefaultResponse: true,
                        },
                    );
                } else {
                    const clusterMap = {
                        state_irrigation: 0xfc0c,
                    };

                    const clusterId = clusterMap[key];
                    const on = value.toLowerCase() === "on" ? 1 : 0;

                    await entity.write(
                        clusterId,
                        {0: {value: on, type: 0x10}},
                        {
                            timeout: 30000,
                            disableDefaultResponse: true,
                        },
                    );
                }

                return {state: {[key]: value.toUpperCase()}};
            },

            convertGet: async (entity, key, meta) => {
                const clusterMap = {
                    state_irrigation: 0xfc0c,
                    state_light: 0x0006,
                };

                await entity.read(clusterMap[key], ["onOff"], {timeout: 30000});
            },
        },
    ],

    configure: async (device, coordinatorEndpoint) => {
        const endpoint = device.getEndpoint(10);

        try {
            await endpoint.bind("genOnOff", coordinatorEndpoint);
            await endpoint.configureReporting("genOnOff", [
                {
                    attribute: "onOff",
                    minimumReportInterval: 0,
                    maximumReportInterval: 3600,
                    reportableChange: 0,
                },
            ]);
        } catch (error) {
            logger.warning(`Failed to configure reporting for genOnOff cluster: ${error}`, NS);
        }

        try {
            await endpoint.bind("irrigation", coordinatorEndpoint);
            await endpoint.configureReporting("irrigation", [
                {
                    attribute: "onOff",
                    minimumReportInterval: 0,
                    maximumReportInterval: 3600,
                    reportableChange: 0,
                },
            ]);
        } catch (error) {
            logger.warning(`Failed to configure reporting for irrigation cluster: ${error}`, NS);
        }

        try {
            await endpoint.bind("msTemperatureMeasurement", coordinatorEndpoint);
            await endpoint.configureReporting("msTemperatureMeasurement", [
                {
                    attribute: "measuredValue",
                    minimumReportInterval: 60,
                    maximumReportInterval: 3600,
                    reportableChange: 10,
                },
            ]);
        } catch (error) {
            logger.warning(`Failed to configure reporting for msTemperatureMeasurement: ${error}`, NS);
        }

        try {
            await endpoint.bind("msRelativeHumidity", coordinatorEndpoint);
            await endpoint.configureReporting("msRelativeHumidity", [
                {
                    attribute: "measuredValue",
                    minimumReportInterval: 60,
                    maximumReportInterval: 3600,
                    reportableChange: 50,
                },
            ]);
        } catch (error) {
            logger.warning(`Failed to configure reporting for msRelativeHumidity: ${error}`, NS);
        }
    },

    ota: true,
};

module.exports = definition;
