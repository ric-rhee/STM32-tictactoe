# Introduction
This is my final project for ECE 362: Microprocessor Systems and Interfacing\
I attempted to write a complete tictactoe game with different game states and a scorekeeper with a reset button.

## Dev board & environment
- STM32F091 development board
- Eclipse IDE with the "System Workbench for STM32" plugin
- STLink Driver to interface with the STM32
- OpenSTM32 Standard Peripheral Library

## How to run
Import the files into a project in System Workbench\
If project configuration is needed,
1. At the "Select a wizard" dialog, select "C Project"
2. At the "C Project" dialog, specify "Ac6 STM32 MCU Project" and specify a project name (do not use spaces)
3. At the "Select Configurations" dialog, press next
4. At the "Target Configuration" dialog, first choose the "Mcu" tab, then "STM32F0", then choose "STM32F091RCTx"
5. At the "Project Firmware Configuration" dialog, choose "Standard Peripheral Library (StdPeriph)"
6. Select the Run >> Debug configurations menu item. Double-click on the "Ac6 STM32 Debugging" item on the left side of the screen to create a New_configuration entry. Press the "Search Project..." button and select the "elf" entry that appears.
7. While still in the Run >> Debug configurations..., select the "Debugger" tab. Change the button that normally says "Show generator options..." to "Hide generator options..." and, in the information shown below it, change the option that says "Connect under reset" to "Software system reset". Then press Apply and Close.
8. Finally, press the debug icon to build and run the code