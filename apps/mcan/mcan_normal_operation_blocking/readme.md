[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# MCAN blocking

This example shows how to use the MCAN module to transmit and receive CAN messages in polling mode.

## Description

This application transmits and receives CAN messages on the CAN bus. To run this application, two evaluation boards of same type are required. These boards acts as different nodes on the CAN bus. Same application is programmed onto both the boards. Boards are connected to PC via UART. While running the application, user can send and receive CAN FD messages between the boards using UART console applications running on the PC.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_rh71) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/mcan/mcan_normal_operation_blocking/firmware** .

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

- Connect SAM RH71 Evaluation Kit to anotherSAM RH71 Evaluation Kit as per the pin connections shown below

| SAMRH71 EK - 1    | SAMRH71 EK - 2     |
| ----------------- | ------------------ |
| CANH0, J13        | CANH0, J13         |
| CANL0, J13        | CANL0, J13         |
| GND, J5           | GND, J5            |
|||

- Connect the debugger probe to J33 for each board

## Running the Application

1. Open the Terminal application (Ex.:Tera term) on the computer
2. Connect to the EDBG Virtual COM port of both the boards and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application on both the boards using their respective IDEs
4. In the console associated with board 2, press "2" to receive a CAN message
5. In the console associated with board 1, press "1" to transmit a CAN  message
6. Transmitted message description and status will be displayed in the console window of board 1
7. Received message will be displayed in the console window of board 2
8. If the steps are executed in this sequence, the final output in the consoles will be as below (console on the left is the transmitter (board 1) and the one on the right is receiver (board 2)):
![output](images/output_mcan_normal_operation_blocking.png)
