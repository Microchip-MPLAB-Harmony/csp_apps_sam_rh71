[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# HEFC flash read write

This example application demonstrates how to use the HEFC to erase and program the internal Flash memory.

## Description

This example uses the HEFC peripheral library to erase a row and write an array of values to the internal Flash memory. It verifies the value written by reading the values back and comparing it to the value written.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_rh71) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/hefc/flash_read_write/firmware** .

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

1. Build and Program the application using its IDE
2. LED indicates success or failure:
    - The LED is turned ON when the value read from the flash matched with the written value
    - The LED is turned OFF when the value read from the flash did not match with the written value

| Board      | LED name |
| ----------------- | ---------------------------------------------- |
| SAM RH71 Evaluation Kit | LED0 |
|||
