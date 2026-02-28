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

State Machines:
State machines will be used for navigating menus. Each state represents a different UI layout, and states change per button press (if the button being pressed is a navigational button). 

States needed so far:
- Main menu
- Performance metrics
- Media controls

## Initial Design Plan
The BLE *central* should be the PC running a (likely Python) script sending metrics to the nRF52840.
The BLE *peripheral* should be the nRF52840.

In this case, the peripheral acts as a "bulletin board", a GATT server, that can have data posted to it or read from it. The central acts as the poster, a GATT client, that actually "sends" data to the bulletin board. We want the PC to send metrics to the nRF52840, hence the role assignment.

In terms of implementing media play/pause (where the nRF52840 can act like a remote control for media playing on a PC running a Python script), the GATT client can *subscribe* to notifications from the GATT server. In this case, "notifications" will actually be instructions from the nRF52840 *to the PC* to perform media instructions.

### Stage 1. Develop a working, barebones visual interface (COMPLETED)
Before we even begin to think about receiving data from the GATT client (the PC), we need a way to visualize it so, down the line, we can confirm that data is actually being received properly.

Therefore, we should first implement the touchscreen and LCD functionality so that data, even mock data, can be displayed on the screen and manipulated which can verify proper communication between the GATT client and server down the line.

#### Performance Metrics UI Layout

The screen is divided into two primary containers to organize the data density effectively.

**1. Top Container (40% Height, Flex-Wrap Row)**
*Used for scalar values that do not require progress bars.*
* [Label] CPU Clock: x.x GHz
* [Label] CPU Power: xx W
* [Label] CPU Temp: xx °C
* [Label] GPU Temp: xx °C
* [Label] Net Upload: xx Mbps
* [Label] Net Download: xx Mbps

**2. Bottom Container (60% Height, Column)**
*Used for percentage-based metrics requiring visual bars.*
* **CPU Usage**
    * [Label] Title: "CPU Usage"
    * [Bar] Range: 0-100%
* **GPU Usage**
    * [Label] Title: "GPU Usage"
    * [Bar] Range: 0-100%
* **RAM Usage**
    * [Label] Title: "RAM Usage"
    * [Bar] Range: 0-100%

#### Media Controls Page Layout:

??? (W.I.P. until performance metrics functionality is fully implemented)

### Stage 2. Develop the bluetooth peripheral (our nRF52840, GATT server) architecture (IN-PROGRESS)

### Stage 3. Develop the bluetooth central (PC running a Python script, GATT client) architecture (TO DO)

### Stage 4. Unify the bluetooth GATT server and client and achieve communication (TO DO)

### Stage 5. Display performance metrics on the nRF52840 (for now, just a set number of metrics) (TO DO)

### Stage 6. Implement media controls on the nRF52840 that modifies media playing on a PC (TO DO)

### Stage 7. Implement toggleable metrics (can show different metrics on screen) (TO DO)

### Stage 8. Refinements, bug fixes, small extra functionalities??? (TO DO)


