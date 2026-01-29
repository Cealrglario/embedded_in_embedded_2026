/*
 * main.c
 */

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>

static struct bt_uuid_128 BLE_CUSTOM_SERVICE_UUID = 
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000001));

static struct bt_uuid_128 BLE_CUSTOM_CHARACTERISTIC_UUID = 
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000002));

static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {

}

static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t err) {

}

static struct bt_conn* my_connection; // points to the current bluetooth connection

int main(void) {

	return 0;
}
