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
