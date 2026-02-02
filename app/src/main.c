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
  if (err) { //
    bt_conn_unref(my_connection); // we use my_connection since we want conn and my_connection to be the same connection
    my_connection = NULL;
  } else {
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr)); // get the connected device's MAC address as a string
    printk("Connected to device with MAC address: %s\n", addr);
  }
}

static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t err) {
  if (err) {
    printk("Device disconnected due to an error: %s\n", bt_hci_err_to_str(err));
  } else {
    printk("Device disconnected.\n");
  }

  bt_conn_unref(my_connection);
  my_connection = NULL;
}

static bool ble_get_adv_device_name_cb(struct bt_data* data, void* user_data) {
  char* name = user_data; // points to a region of memory that will store any user data

  if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {  // if the data received contains device name
    memcpy(name, data->data, data->data_len); // copy device name into appropriate memory space
    name[data->data_len] = 0; // null terminate the name
    return false; // stop parsing the name
  }

  return true;  // continue looking through the received advertising packet for the device name
}

static void ble_on_advertisement_received(const bt_addr_le_t* addr, int8_t rssi, uint8_t type,
                                          struct net_buf_simple* ad) {
  if (NULL != my_connection) {  // we already have a connection
    return;
  }
  else if (BT_GAP_ADV_TYPE_ADV_IND != type || BT_GAP_ADV_TYPE_ADV_DIRECT_IND != type) { // if the advertisement is "non-connectable"
    return;
  }
  else {  // process the incoming advertisement
    // Get the advertising device's name
    char name[32] = {0};
    bt_data_parse(ad, ble_get_adv_device_name_cb, name);

    // Get the advertising device's MAC address
    char str_addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, str_addr, sizeof(str_addr));

    printk("Advertisement received from device with name: %s and MAC address: %s\n", name, str_addr);

    // return if connection isn't strong
    if (rssi < -60) {
      return;
    }
    else {  // if name matches
      bt_le_scan_stop();  // stop scanning for more advertisers
      int err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &my_connection);  // connect to the device
      
      if (err) {
        // error occurred during connection establishment
        printk("Error occurred while trying to establish connection to device with name %s\n", name);
        printk("Details: %s\n", bt_hci_err_to_str(err));
        return;
      }
    }
  }
}

// Tell the OS to use the connect/disconnect callback funcs during connection events
BT_CONN_CB_DEFINE(conn_callbacks) = {
  .connected = ble_on_device_connected,
  .disconnected = ble_on_device_disconnected,
};

int main(void) {
  // Enable bluetooth
  int err = bt_enable(NULL);

  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return 0;
  }
  else {
    printk("Bluetooth initialized!\n");
  }

  // Start PASSIVELY scanning for devices
  bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_on_advertisement_received);
	return 0;
}
