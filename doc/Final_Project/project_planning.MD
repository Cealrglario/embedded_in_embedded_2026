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
~~### 1. Thermal/IR camera (extra costs)~~ -> NOT CHOSEN DUE TO COSTS
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


