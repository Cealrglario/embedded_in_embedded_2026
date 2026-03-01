/**
 * @file ble_peripheral.h
 */

#ifndef BLE_PERIPHERAL_H
#define BLE_PERIPHERAL_H

/**
 * Includes
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

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
    uint32_t ram_usage_percent; // MSB (end write)
} cpu_gpu_ram_percentage_metrics_t;

/**
 * Service and Characteristic Setup
 */

#define BLE_HARDWARE_MONITOR_SERVICE_UUID \
    BT_UUID_128_ENCODE(0x01928374, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BLE_CPU_GPU_SCALAR_METRICS_CHARACTERISTIC \
    BT_UUID_128_ENCODE(0x01928374, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)

#define BLE_NETWORK_SCALAR_METRICS_CHARACTERISTIC \
    BT_UUID_128_ENCODE(0x01928374, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)

#define BLE_CPU_GPU_RAM_PERCENTAGE_METRICS_CHARACTERISTIC \
    BT_UUID_128_ENCODE(0x01928374, 0x1234, 0x5678, 0x1234, 0x56789abcdef3)

#define BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH 20

#endif
