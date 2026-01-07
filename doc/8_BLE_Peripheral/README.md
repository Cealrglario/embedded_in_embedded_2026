# BLE Peripheral

[TOC]

## Introduction

As an extension of the last lesson,
we're going to now go over building a custom BLE peripheral from scratch.
We'll be keeping the peripheral as a GATT server,
meaning that it will be the device that hosts the services and characteristics
presented to connecting devices.

## Lesson

For this lesson,
we'll be trying to replicate and expand on the BLE peripheral example from the last lesson.

### Includes

In order to link everything we need to call, add the following includes to your `main.c`:

```c
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>
```

### Kconfig Project Configuration

In order to enable the BLE peripheral section of the Zephyr RTOS,
we need to set a few configuration values in the `prj.conf` file:

1. `CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE` -
    This allows for the operating system's internal event handler to allocate more memory for its stack.
    Because we are requesting the OS to control more complicated behaviour than normal, we need to increase this value.
    Set this to `2048` (measured in bytes).
2. `CONFIG_BT` -
    This config simply controls whether or not BLE in general is enabled.
    Set this to `y` (enabled).
3. `CONFIG_BT_PERIPHERAL`
    This config enables the peripheral mode of the BLE subsystem.
    With this enabled, we get the ability to tell the device to advertise, and thus, have other devices connect to our board.
    Set this to `y` (enabled).
4. `CONFIG_BT_DEVICE_NAME` -
    Finally, this is the name that the device will advertise with.
    Set this to something unique to you, such as `"#### EiE BLE Peripheral"`, where `####` is the last 4 digits of your UCID.

Making these changes in `prj.conf` should still allow your project to compile,
even before you make any code changes.
To make sure you've done the modifications correctly, make sure your project still compiles! 

### Source Code Changes

#### UUIDs

Every custom BLE service and characteristic requires a "Universally Unique Identifier" (UUID).
These are 16B numbers that are ideally completely unique in a given context.
For our purposes, we can set these to whatever we want,
but you'll want to make sure you never use the same UUID twice for two different items.

You can use the `BT_UUID_128_ENCODE` macro to define two UUIDs,
one for a service, and one for a characteristic.
For ease of use, use `#define`s for this.

```c
BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
```

#### Advertising Data

We have to define what data the radio sends out in advertising packets.
We can generally put whatever information we want in the advertising data as long as the whole packet is at most 31 bytes,
however specific things like connection flags or available service IDs aid in discovery and connection forming.
There are some features to increase this size to up to 1650 bytes, but we won't be using them,
as the focus of our advertising is only going to be for connection establishment.
We're going to be using a data flags field (3B) and our device name (variable size).

We can generate an encoding for the flags field with `BT_DATA_BYTES(BT_DATA_FLAGS, flags)`,
where `flags` is a series of bitwise-OR'd values that set specific bits.
For our use, we need to set the `BT_LE_AD_GENERAL` and `BT_LE_AD_NO_BREDR` flags.
`BT_LE_AD_GENERAL` enables "general discoverability mode",
allowing any device to receive our advertising packets and try to connect,
and `BT_LE_AD_NO_BREDR` simply tells the connecting devices that we only support BLE,
not the "classic" BLE (a.k.a. "BREDR").

Next we can include our device name with `BT_DATA(BT_DATA_NAME_COMPLETE, name, sizeof(name))`,
where `name` is an ASCII array (e.g. string).
Since we've defined our device name with Kconfig,
we can use `CONFIG_BT_DEVICE_NAME` for this.

All we have to do now is define an `bt_data` array to use as our advertising packets:

```c
static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(...),
  BT_DATA(...),
};
```

#### Characteristic Data Buffers

For each of our characteristics,
we need to define an array that stores the data written to or read from it.

We should set the maximum size for this array to be 20B in order to work with the default parameters available to most devices.
As with advertising data, there are mechanisms to expand this, however we won't be using them.


```c
static uint8_t ble_custom_characteristic_user_data[20] = {};
```

#### Read Callback

We need to define a callback for when the characteristic is read from.
In its simplest form, we simply relay the read event to the GATT server,
which then will send the current stored value of the characteristic to the connected device.

```c
static ssize_t ble_custom_characteristic_read_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                                 void* buf, uint16_t len, uint16_t offset) {
  const char* value = attr->user_data;
  return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}
```

#### Write Callback

Similarly to the read callback,
we need to define a callback for when our characteristic is written to.
In its simplest form, all we do is take the value written, ensure that it isn't too long,
and then save it into the characteristic's data buffer.

```c
static ssize_t ble_custom_characteristic_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                                  const void* buf, uint16_t len, uint16_t offset,
                                                  uint8_t flags) {
  uint8_t* value_ptr = attr->user_data;

  if (offset + len > BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  memcpy(value_ptr + offset, buf, len);
  value_ptr[offset + len] = 0;

  return len;
}
```

#### Service and Characteristic Description

Now we can tie all of our pieces together with the service definition macro, `BT_GATT_SERVICE_DEFINE`.
This is what actually creates the service and its included characteristics.
Inside of this macro,
we first specify a name for the created object that stores all of the config for our service,
the the UUID for the service,
and finally the characteristics.

First, let's define `bt_uuid_128` objects for each of out UUIDs,
where `BLE_CUSTOM_SERVICE_UUID` and `BLE_CUSTOM_CHARACTERISTIC_UUID` are the `#define`s from earlier:

```c
static const struct bt_uuid_128 ble_custom_service_uuid = BT_UUID_INIT_128(BLE_CUSTOM_SERVICE_UUID);
static const struct bt_uuid_128 ble_custom_characteristic_uuid = BT_UUID_INIT_128(BLE_CUSTOM_CHARACTERISTIC_UUID);
```

Then we'll start the service definition macro:

```c
BT_GATT_SERVICE_DEFINE(
    ble_custom_service,  // Name of the struct that will store the config for this service
    BT_GATT_PRIMARY_SERVICE(&ble_custom_service_uuid),  // Setting the service UUID
    ...
);
```

Finally, we can define our characteristic using the `BT_GATT_CHARACTERISTIC` macro.
In order, we need to pass the characteristic UUID,
permitted operations,
default permissions,
callbacks,
and a location for it to store its data.
Most of these we've already defined,
but the permitted operations and the default permissions haven't been touched yet.
These all use either pre-defined `BT_GATT_CHRC_*` (for permitted operations) or `BT_GATT_PERM_*` (for permissions)
values all bitwise-OR'd together.
This sets specific flag bits that the BLE stack uses to configure some lower-level behaviour.
For both of these, we want the `READ` and `WRITE` values.
Put this macro inside the list of arguments to `BT_GATT_SERVICE_DEFINE` after `BT_GATT_PRIMARY_SERVICE(...)`.

```c
    ...
    BT_GATT_CHARACTERISTIC(
        &ble_custom_characteristic_uuid.uuid,  // Setting the characteristic UUID
        <BT_GATT_CHRC_* values ORd together>,  // Possible operations
        <BT_GATT_PERM_* values ORd together>,  // Permissions that connecting devices have
        ble_custom_characteristic_read_cb,     // Callback for when this characteristic is read from
        ble_custom_characteristic_write_cb,    // Callback for when this characteristic is written to
        ble_custom_characteristic_user_data    // Initial data stored in this characteristic
        ),
    ...
```

#### Main

The last thing we need to do is initialize the BLE module and start advertising in the `main` function.
First, we can call `bt_enable(NULL)` to initialize BLE.
Be sure to check the return value, as **any value other than 0 means the init failed**.
Pay attention here, because *this is backwards from what you're used to with booleans*!
This is because lots of Zephyr APIs use "Error Numbers" to communicate *what* failed,
rather than just the fact that *something* failed.
In this scheme, 0 typically means "OK" and everything else has a related error message.
Zephyr defines these here if you're interested:
<https://docs.zephyrproject.org/3.7.0/doxygen/html/group__system__errno.html>
If you get anything else, print the error value and return from the `main(...)` function.

The argument to `bt_enable(...)` being `NULL` means that we want the BLE module to init *synchronously*,
meaning that this call is a *blocking* call,
and only passes to the next instruction once the BLE module is ready to be used.
We could, in theory, pass a callback here instead and do other things while the module initializes,
then start using BLE once the callback is called, but that isn't necessary for us at the moment.

Next we want to start *advertising*,
or sending out small "advertising packets" over the radio for other devices to find.
Note that we only want to be advertising when we want other devices to be able to see us,
and most of the time, connect to us.
This is done with the `bt_le_adv_start(...)` function.
For it's arguments, we need to pass in the advertising config
(use the prebuilt `BT_LE_ADV_CONN_FAST_1` for this),
a pointer to our advertising data array (`ble_advertising_data`),
the size of our advertising data (`ARRAY_SIZE(ble_advertising_data)`),
and then a pointer to our "scan response" data array and its size as well
(we aren't using these, so just use `NULL` and `0` for these respectively).

Just like with `bt_enable(...)`, make sure the return value from `bt_le_adv_start(...)` is 0 before continuing.
Otherwise, print the error value and return from the `main(...)` function.

**Make sure that you do all of this** ***before*** **the main loop!**
We only want this to happen once!

### Run It!

Once compiled and flashed,
your board should now be functioning very similarly to the example code from the previous lesson,
so let's test it!
Open up nRF Connect on your phone and ensure you can find and connect to your board,
see your custom service and characteristic,
and read and write data to said characteristic.
If anything isn't working,
be sure to add some `printk(...)` statements to your code and look at what's happening in the serial monitor.
If you get stuck, ask a leader!

## Challenge

If you're finished early and have your board all working as expected,
try to complete the following to advance your knowledge and capabilities:

1. Currently, your custom service only has a single characteristic.
   Many custom and pre-defined BLE services actually have *multiple* characteristics for organization and permission management.
   Try adding *another* characteristic to your existing service that operates completely independently.
   For example, writing to one characteristic should not impact the other,
   and each should be able to store their own value that can be read from/written to.

2. Expanding from challenge 1, let's start messing with permissions.
   Now that you have 2 characteristics, we can make them act as a getter and a setter, respectively.
   Modify the definition of your *first characteristic* such that it can *only be read from*.
   Next, modify the definition of your *second characteristic* such that it can *only be written to*.
   Finally, find a way to set the behaviour such that these two services act like they store the same value.
   For example, if you write to the second characteristic, then read from the first,
   the connected device should receive whatever value was just written to the second characteristic!

3. While we went over reading and writing in this lesson,
   the one feature missing from the example code that we didn't go over was notifications.
   Referencing the example code from the last lesson (on the `lesson/ble_intro` branch),
   try and find a way to allow for a connected device to subscribe to and receive notifications from one of your characteristics
   (let's choose the one that can be read from if you've done challenge 1 first).
   
   a. What's been added to the arguments of `BT_GATT_SERVICE_DEFINE(...)`?

   b. What's changed in the use of `BT_GATT_CHARACTERISTIC(...)`?

   c. What's being called in the main loop? What does this function do?

      i. Note: Depending on how you've modified your service with the first two challenges,
         there might be a tiny change needed to a call in this function to do with the index used.
         Try and figure it out, but if you get stuck, ask for help!
