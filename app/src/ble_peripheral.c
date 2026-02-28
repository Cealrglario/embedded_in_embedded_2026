/**
 * @file ble_peripheral.c
 */

#include "ble_peripheral.h"

/**
 * Typedefs
 */

typedef struct {
    uint32_t cpu_clock_mhz; // LSB (start write)
    uint32_t cpu_power_watts;
    uint32_t cpu_temp_celsius;
    uint32_t gpu_temp_celsius; // MSB (end write)
} cpu_gpu_scalar_metrics_t;

typedef struct {
    uint32_t network_down_bits; // LSB (start write)
    uint32_t network_up_bits; // MSB (end write)
} network_scalar_metrics_t;

typedef struct {
    uint32_t cpu_usage_percent; // LSB (start write)
    uint32_t gpu_usage_percent;
    uint32_t ram_usage_gb; // MSB (end write)
} cpu_gpu_ram_percentage_metrics_t;

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

const size_t advertising_data_array_size = ARRAY_SIZE(ble_advertising_data);
const size_t scan_response_data_array_size = ARRAY_SIZE(ble_scan_response_data);

// Define structs that store the data written to our characteristics (we dont want them static as we need access to them in state machine)
cpu_gpu_scalar_metrics_t ble_cpu_gpu_scalar_metrics_characteristic_data;
network_scalar_metrics_t ble_network_scalar_metrics_characteristic_data;
cpu_gpu_ram_percentage_metrics_t ble_cpu_gpu_ram_percentage_metrics_characteristic_data;

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
        NULL, // We don't need a callback for reading as a client doesn't read our characteristics
        ble_cpu_gpu_scalar_metrics_write_cb, // Callback for when this characteristic is written to
        &ble_cpu_gpu_scalar_metrics_characteristic_data // Address where we want data stored for this characteristic
        ),

    // FOR NETWORK SCALAR METRICS
    BT_GATT_CHARACTERISTIC(
        &ble_network_scalar_metrics_characteristic_uuid.uuid, // Setting the characteristic UUID
        BT_GATT_CHRC_WRITE_WITHOUT_RESP, // A connected GATT client can write to this characteristic, and we don't need to reply with an ack
        BT_GATT_PERM_WRITE, // Permissions that connecting devices have
        NULL, // We don't need a callback for reading as a client doesn't read our characteristics
        ble_network_scalar_metrics_write_cb, // Callback for when this characteristic is written to
        &ble_network_scalar_metrics_characteristic_data // Address where we want data stored for this characteristic
        ),

    // FOR CPU, GPU AND RAM PERCENTAGE METRICS
    BT_GATT_CHARACTERISTIC(
        &ble_cpu_gpu_ram_percentage_metrics_characteristic_uuid.uuid, // Setting the characteristic UUID
        BT_GATT_CHRC_WRITE_WITHOUT_RESP, // A connected GATT client can write to this characteristic, and we don't need to reply with an ack
        BT_GATT_PERM_WRITE, // Permissions that connecting devices have
        NULL, // We don't need a callback for reading as a client doesn't read our characteristics
        ble_cpu_gpu_ram_percentage_metrics_write_cb, // Callback for when this characteristic is written to
        &ble_cpu_gpu_ram_percentage_metrics_characteristic_data // Address where we want data stored for this characteristic
        ),
    // End of service definition
);

/**
 * Write callback definitions
 */

static ssize_t ble_cpu_gpu_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    /**
     * conn: pointer representing the BLE connection to the GATT client
     * attr: points to the characteristic being written to defined in BT_GATT_SERVICE_DEFINE, attr->user_data POINTS to the struct holding the actual data
     * buf: raw bytestream coming from GATT client
     * len: length of the bytestream coming from the GATT client
     * offset: only matters if incoming bytestream is greater than maximum per write (20 bytes), which we won't allow so we can ignore this
     * flags: indicates type of BLE write (in this case, Write Without Response), not important
     */

    // If data received is over the maximum we expect
    if (len != sizeof(cpu_gpu_scalar_metrics_t) || offset != 0) {
    printk("[BLE] ble_cpu_gpu_scalar_metrics_write_cb: Received oversized data.\n");
    return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
    }

    // Since each incoming metric is packed in its own uint32_t with no need to consider padding, we can simply write all incoming bytes
    // directly into attr->user_data with one line of code which will store each metric into each struct attribute in Little-Endian order (LSB first)
    memcpy(attr->user_data, buf, len);
    printk("Received CPU and GPU scalar metrics from GATT client.\n");

    return len;
};
                                        
static ssize_t ble_network_scalar_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    /**
     * conn: pointer representing the BLE connection to the GATT client
     * attr: points to the characteristic being written to defined in BT_GATT_SERVICE_DEFINE, attr->user_data POINTS to the struct holding the actual data
     * buf: raw bytestream coming from GATT client
     * len: length of the bytestream coming from the GATT client
     * offset: only matters if incoming bytestream is greater than maximum per write (20 bytes), which we won't allow so we can ignore this
     * flags: indicates type of BLE write (in this case, Write Without Response), not important
     */

    // If data received is over the maximum we expect
    if (len != sizeof(network_scalar_metrics_t) || offset != 0) {
    printk("[BLE] ble_network_scalar_metrics_write_cb: Received oversized data.\n");
    return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
    }

    // Since each incoming metric is packed in its own uint32_t with no need to consider padding, we can simply write all incoming bytes
    // directly into attr->user_data with one line of code which will store each metric into each struct attribute in Little-Endian order (LSB first)
    memcpy(attr->user_data, buf, len);
    printk("Received network scalar metrics from GATT client.\n");

    return len;
};

static ssize_t ble_cpu_gpu_ram_percentage_metrics_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        const void* buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {
    /**
     * conn: pointer representing the BLE connection to the GATT client
     * attr: points to the characteristic being written to defined in BT_GATT_SERVICE_DEFINE, attr->user_data POINTS to the struct holding the actual data
     * buf: raw bytestream coming from GATT client
     * len: length of the bytestream coming from the GATT client
     * offset: only matters if incoming bytestream is greater than maximum per write (20 bytes), which we won't allow so we can ignore this
     * flags: indicates type of BLE write (in this case, Write Without Response), not important
     */ 

    // If data received is over the maximum we expect
    if (len != sizeof(cpu_gpu_ram_percentage_metrics_t) || offset != 0) {
    printk("[BLE] ble_cpu_gpu_ram_percentage_metrics_write_cb: Received oversized data.\n");
    return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
    }

    // Since each incoming metric is packed in its own uint32_t with no need to consider padding, we can simply write all incoming bytes
    // directly into attr->user_data with one line of code which will store each metric into each struct attribute in Little-Endian order (LSB first)
    memcpy(attr->user_data, buf, len);
    printk("Received CPU, GPU and RAM percentage metrics from GATT client.\n");
     
    return len;
};

