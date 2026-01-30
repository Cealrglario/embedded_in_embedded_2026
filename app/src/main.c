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

static struct bt_conn* my_connection; // points to the current bluetooth connection

static struct bt_uuid_128 BLE_CUSTOM_SERVICE_UUID = 
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000001));

static struct bt_uuid_128 BLE_CUSTOM_CHARACTERISTIC_UUID = 
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111, 0x2222, 0x3333, 0x4444, 0x000000000002));

static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {
  if (0 != err) { //
    bt_conn_unref(my_connection); // we use my_connection since we want conn and my_connection to be the same connection
    my_connection = NULL;
  } else {
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr)); // get the connected device's MAC address as a string
    printk("Connected to: %s\n", addr);
  }
}

static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t err) {
  if (0 != err) {
    printk("Device disconnected due to an error: %s\n", bt_hci_err_to_str(err));
  } else {
    printk("Device disconnected: %s\n", bt_hci_err_to_str(reason));
  }

  bt_conn_unref(my_connection);
  my_connection = NULL;
}

static void ble_on_advertisement_received(const bt_addr_le_t* addr, int8_t rssi, uint8_t type,
                                          struct net_buf_simple* ad) {

}

// Tell the OS to use the connect/disconnect callback funcs during connection events
BT_CONN_CB_DEFINE(conn_callbacks) = {
  .connected = ble_on_device_connected,
  .disconnected = ble_on_device_disconnected,
};

int main(void) {
  // Start PASSIVELY scanning for devices
  bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_on_advertisement_received);
	return 0;
}
