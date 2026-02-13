# EiE 2025-2026 Final Project Planning
## Goals
We want to TRY To use all functionalities
- Bluetooth
- LCD/LVGL
- Touchscreen

We want to develop considering best practices
- Zephyr state machines
- Zephyr logic and HAL

## Ideas
### 1. Thermal/IR camera (extra costs) -> NOT CHOSEN DUE TO COSTS
~~Description:~~
~~- Connect a thermal/IR sensor to the nRF52840, maybe using a breadboard, that displays a heatmap onto the LCD.~~

~~LCD/LVGL functionality:~~
~~- Display an "upscaled" and zoomed out heatmap to the LCD.~~

~~Bluetooth:~~
~~- Transmit heatmap over bluetooth to a web server or mobile application to view heatmap online/on a bigger screen.~~

~~Touchscreen:~~
~~- Buttons to toggle UI elements like temperature, distance to center, etc.~~

### 2. Desktop/laptop "sidecar" -> CHOSEN IDEA
Description:
- Run a script on a desktop/laptop that, over bluetooth, sends PC metrics and usage details to the nRF52840 that displays details and potential interaction options onto the LCD.

LCD/LVGL functionality:
- Display PC metrics, usage info, and provide user interaction options to view different metrics or even interact with music playing on the computer.

Bluetooth:
- Transmit PC information to the nRF52840 over BLE.

Touchscreen:
- Provide toggles to view different PC metrics and information, and provide options to modify music being played on the PC, if applicable.

## Initial Design Plan
The BLE *central* should be the PC running a (likely Python) script sending metrics to the nRF52840.
The BLE *peripheral* should be the nRF52840.

In this case, the peripheral acts as a "bulletin board", a GATT server, that can have data posted to it or read from it. The central acts as the poster, a GATT client, that actually "sends" data to the bulletin board. We want the PC to send metrics to the nRF52840, hence the role assignment.

In terms of implementing media play/pause (where the nRF52840 can act like a remote control for media playing on a PC running a Python script), the GATT client can *subscribe* to notifications from the GATT server. In this case, "notifications" will actually be instructions from the nRF52840 *to the PC* to perform media instructions.

### 1. Develop a working "debugging" interface
Before we even begin to think about receiving data from the GATT client (the PC), we need a way to visualize it so, down the line, we can confirm that data is actually being received properly.

Therefore, we should first implement the touchscreen and LCD functionality so that data, even mock data, can be displayed on the screen and manipulated which can verify proper communication between the GATT client and server down the line.





