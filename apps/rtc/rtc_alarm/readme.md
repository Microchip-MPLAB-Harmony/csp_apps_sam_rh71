[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# RTC alarm interrupt

This example shows how to use the RTC to configure the time and generate the alarm.

## Description

This example application shows how to setup system time and configure alarm using the RTC Peripheral Library. The application sets up an alarm to be generated after 20 seconds of the configured system time. The application displays a message on the alarm trigger.

## Downloading and building the application

To download or clone this application from Github, go to the [top level of the repository](https://github.com/Microchip-MPLAB-Harmony/csp_apps_sam_rh71) and click

![clone](../../../docs/images/clone.png)

Path of the application within the repository is **apps/rtc/rtc_alarm/firmware** .

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

1. Open the Terminal application (Ex.:Tera term) on the computer
2. Connect to the EDBG Virtual COM port and configure the serial settings as follows:
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and Program the application project using its IDE
4. The console displays the current time and the alarm time (System time starts at 12:00:00 and gets updated every second)

    ![output](images/output_rtc_alarm_1.png)

5. Once the alarm time is reached, it print the alarm message and turns on the LED (System time keeps updating)

    ![output](images/output_rtc_alarm_2.png)

The following table provides the LED and switch names:

| Board | LED  |
| ----- | --------- |
| SAM RH71 Evaluation Kit  |LED0 |
|||