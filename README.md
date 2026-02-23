# Main Repository
This repository is currently being used for the development of a final project to complete the EiE program for its 2025-2026 run. The project aims to turn the nRF52840 development board into a hardware monitor "sidecar" which displays real-time hardware performance metrics (such as CPU usage, CPU core clocks, temperatures, etc.) from a PC connected over bluetooth onto an LCD attached to the nRF52840. Down the line, functionality will also be added to the "sidecar" that allows it to act as a media control device for the connected PC which allows a user to play, pause, skip and change the volume of a song or video currently playing on the PC.

# EiE Program Details
## Embedded In Embedded w/ nRF52840

EiE is a mentoring program for university engineering students that was started in 2002 at the University of Calgary. The program connects industry engineers with students interested in embedded systems. Together they explore the practical, hands-on technical side of embedded development while building great relationships.
https://embeddedinembedded.com/

## Hardware

Starting with the 2025/2026 school year EIE is using the nRF52840 development kit from Nordic Semiconductors which uses the nRF52840 system on chip

### Schematic and Resources

- [Datasheet](https://docs.nordicsemi.com/bundle/ps_nrf52840/page/keyfeatures_html5.html)
- [Board Schematic, Layout drawings, and Altium files](https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/dev-kits/nrf52840-dk/nrf52840-development-kit---hardware-files-3_0_3.zip)

## Firmware

The firmware used for EIE is based on the Zephyr RTOS and is contained within this repository

## Lessons

1. [Getting Started](doc/1_Getting_Started/README.MD)
2. [Version Control](doc/2_Version_Control/README.MD)
3. [Embedded Primer](doc/3_Embedded_Primer/embedded_primer.adoc)
4. [Debug and Buttons](doc/4_Debug_and_Buttons/README.md)
5. [State Machines](doc/5_State_Machines/state_machines.adoc)
6. [Pulse Width Modulation](doc/6_Pulse_Width_Modulation/README.md)
7. [BLE Intro](doc/7_BLE_Intro/README.md)
8. [BLE Peripheral](doc/8_BLE_Peripheral/README.md)
9. [BLE Central](doc/9_BLE_Central/README.md)
10. [Serial Protocols](doc/10_Serial_Protocols/serial_protocols.adoc)
11. [Zephyr Modules](doc/11_Zephyr_Modules/zephyr_modules.adoc)
