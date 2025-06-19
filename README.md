# Balance Robot Firmware
The firmware design of the balance robot, written in C language, bare metal, Keil uVision 5
![image](https://github.com/user-attachments/assets/2baac360-61ce-486a-84f7-f0306064dccb)

# Project: Balance Robot 
1.Two wheels self balance robot

2.Support BLE for remote control robot’s motion

3.Powered by rechargeable batteries and support on-board charging 

4.PCB Design, designed by Altium Designer

5.Firmware Design (C language, bare-metal) **(This project page)**

# Demonstration
https://www.youtube.com/watch?v=u6oU_3oZxdM

# Hardware Structure
![image](https://github.com/user-attachments/assets/56e582bc-2032-436c-bdb0-fe9698ca58f3)

# Balance Control Structure
![image](https://github.com/user-attachments/assets/0bfd8ef7-d2d6-4a48-9aba-190ac4e9e1c8)

# Program flow
The program is written in C, using the STM32 LL (Low-Level) library in a bare-metal style.

The robot balance control algorithm is implemented within a 1 kHz timer interrupt routine, which includes sensor updates (MPU6050 DMP sensor, motor encoder) and seven PID control functions.

Additionally, the main loop handles UART communication with BLE modules, robot fall detection, and battery management.

The firmware also includes address-based data logging functions placed inside the 1 kHz control routine, allowing the data logging module to record any variables in the program for tuning control parameters.

![image](https://github.com/user-attachments/assets/dcf99269-94b8-4c73-a8e3-a3e1bdf43d41)
![image](https://github.com/user-attachments/assets/c8375687-95ea-42cf-b0bc-080d8705f1b4)
![image](https://github.com/user-attachments/assets/b0cdf2f5-23a0-4d7e-8c23-7b1d5d78bbf4)

# Hardware 
The hardware: [https://github.com/hkcys9990/Balance-Robot-Hardware/edit/main/README.md](https://github.com/hkcys9990/Balance-Robot-Hardware/tree/main)
