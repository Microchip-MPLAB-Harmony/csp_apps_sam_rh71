[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# RTT periodic interrupt

This example shows how to use the RTT to generate periodic interrupts.

## Description

This example application shows how to configure the RTT Peripheral to generate periodic ticks. It toggles an LED every time the callback is triggered.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_rh71) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/rtt/rtt_periodic_timeout/firmware** .

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

1. Build and Program the application project using its respective IDE
2. The LED blinks continuously

The following table provides the LED name

| Board      | LED |
| ----------------- | ---------- |
| SAM RH71 Evaluation Kit | LED0 |
|||
