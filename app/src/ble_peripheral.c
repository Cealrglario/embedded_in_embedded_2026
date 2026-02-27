/**
 * @file ble_peripheral.c
 */

#include "ble_peripheral.h"

/**
 * Local variables
 */

static const struct bt_uuid_128 ble_hardware_monitor_service_uuid = BT_UUID_INIT_128(BLE_HARDWARE_MONITOR_SERVICE_UUID);

static const struct bt_uuid_128 ble_cpu_gpu_scalar_metrics_characteristic_uuid =
    BT_UUID_INIT_128(BLE_CPU_GPU_SCALAR_METRICS_CHARACTERISTIC);

static const struct bt_uuid_128 ble_network_scalar_metrics_characteristic_uuid =
    BT_UUID_INIT_128(BLE_NETWORK_SCALAR_METRICS_CHARACTERISTIC);

static const struct bt_uuid_128 ble_cpu_gpu_ram_percentage_metrics_characteristic_uuid =
    BT_UUID_INIT_128(BLE_CPU_GPU_RAM_PERCENTAGE_METRICS_CHARACTERISTIC);

// Data actively advertised for GATT clients to see
const struct bt_data ble_advertising_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BLE_HARDWARE_MONITOR_SERVICE_UUID),
};

// Data returned to a GATT client when our peripheral is scanned
const struct bt_data ble_scan_response_data[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

// Define arrays that store the data written to our characteristics
static uint8_t ble_cpu_gpu_scalar_metrics_characteristic_data[BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH] = {};
static uint8_t ble_network_scalar_metrics_characteristic_data[BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH] = {};
static uint8_t ble_cpu_gpu_ram_percentage_metrics_characteristic_data[BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH] = {};

/**
 * Prototypes
 */

// We only need a callback for when we're written to, as we don't ever return anything back to a connected GATT client
static ssize_t ble_cpu_gpu_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags);
                                        
static ssize_t ble_network_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags);

static ssize_t ble_cpu_gpu_ram_percentage_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags);

/**
 * BLE service setup
 */

BT_GATT_SERVICE_DEFINE(
    ble_hardware_monitor_service, // Name of the struct that will store the config for this service
    BT_GATT_PRIMARY_SERVICE(&ble_hardware_monitor_service_uuid), // Setting the service UUID

    // Now to define the characteristics:

    // FOR CPU AND GPU SCALAR METRICS
    BT_GATT_CHARACTERISTIC(
        &ble_cpu_gpu_scalar_metrics_characteristic_uuid.uuid, // Setting the characteristic UUID
        BT_GATT_CHRC_WRITE_WITHOUT_RESP, // A connected GATT client can write to this characteristic, and we don't need to reply with an ack
        BT_GATT_PERM_WRITE, // Permissions that connecting devices have
        ble_cpu_gpu_scalar_metrics_write_cb, // Callback for when this characteristic is written to
        ble_cpu_gpu_scalar_metrics_characteristic_data // Initial data stored in this characteristic
        ),

    // FOR NETWORK SCALAR METRICS
    BT_GATT_CHARACTERISTIC(
        &ble_network_scalar_metrics_characteristic_uuid.uuid, // Setting the characteristic UUID
        BT_GATT_CHRC_WRITE_WITHOUT_RESP, // A connected GATT client can write to this characteristic, and we don't need to reply with an ack
        BT_GATT_PERM_WRITE, // Permissions that connecting devices have
        ble_network_scalar_metrics_write_cb, // Callback for when this characteristic is written to
        ble_network_scalar_metrics_characteristic_data // Initial data stored in this characteristic
        ),

    // FOR CPU, GPU AND RAM PERCENTAGE METRICS
    BT_GATT_CHARACTERISTIC(
        &ble_cpu_gpu_ram_percentage_metrics_characteristic_uuid.uuid, // Setting the characteristic UUID
        BT_GATT_CHRC_WRITE_WITHOUT_RESP, // A connected GATT client can write to this characteristic, and we don't need to reply with an ack
        BT_GATT_PERM_WRITE, // Permissions that connecting devices have
        ble_cpu_gpu_ram_percentage_metrics_write_cb, // Callback for when this characteristic is written to
        ble_cpu_gpu_ram_percentage_metrics_characteristic_data // Initial data stored in this characteristic
        ),
    // End of service definition
);

/**
 * Write callback definitions
 */

static ssize_t ble_cpu_gpu_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    // TODO    
};
                                        
static ssize_t ble_network_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    // TODO    
};

static ssize_t ble_cpu_gpu_ram_percentage_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    // TODO    
};

