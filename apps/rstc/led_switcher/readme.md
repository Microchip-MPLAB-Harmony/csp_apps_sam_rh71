[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# RSTC interrupt

This example shows how to use the RSTC peripheral to generate an interrupt instead of generating a reset, when the Reset switch is pressed on board.

## Description

The RSTC peripheral samples the Reset input (NRST pin) at slow clock speed. When the line is detected low, it resets the processor and the peripherals. However, the RSTC peripheral can be programmed to not trigger a reset when an assertion of NRST occurs. The Reset pin state can be read at any time in software or it can also be programmed to generate an interrupt
instead of generating a reset.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_rh71) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/rstc/led_switcher/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_rh71_ek.X | MPLABX project for SAM RH71 Evaluation Kit |
|||

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_rh71_ek.X | SAM RH71 Evaluation Kit
|||

### Setting up SAM RH71 Evaluation Kit

- Connect the debugger probe to J33

## Running the Application

1. Build and Program the application project using its IDE
2. Disconnect and reconnect the Debug USB port to power cycle the board
3. Press the "RESET" switch and observe the LED toggling

The following table provides the LED name

| Board      | LED Name Name
| ----------------- | ---------- |
| SAM RH71 Evaluation Kit      | LED0 |
|||
