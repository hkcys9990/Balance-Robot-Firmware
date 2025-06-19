# Balance Robot Firmware
The firmware design of the balance robot, written in C language, bare metal, Keil uVision 5
![image](https://github.com/user-attachments/assets/2baac360-61ce-486a-84f7-f0306064dccb)

# Project: Balance Robot 
1.Two wheels self balance robot

2.Support BLE for remote control robot’s motion

3.Powered by rechargeable batteries and support on-board charging 

4.PCB Design, designed by Altium Designer

5.Firmware Design (C language, bare-metal) **(This project page)**

# Hardware Structure
![image](https://github.com/user-attachments/assets/56e582bc-2032-436c-bdb0-fe9698ca58f3)

# Balance Control Structure
![image](https://github.com/user-attachments/assets/0bfd8ef7-d2d6-4a48-9aba-190ac4e9e1c8)

# Program flow
The program is written in C language, using STM32 LL (Low-level) library, bare-mental style.

The robot balance control algorithm is placed in a 1kHz timer interrupt routine, including sensor update (MPU6050 DMP sensor, motor encoder), 7x PID control functions.

Besides, the main loop is handling the uart communciation with BLE modules, robot falling detection and robot battery management.

The firmware include address based data log functions, it's placed inside the 1Khz control routine so that the data log modules can log down any variabels in the program for tunning control parameters.

![image](https://github.com/user-attachments/assets/dcf99269-94b8-4c73-a8e3-a3e1bdf43d41)
![image](https://github.com/user-attachments/assets/c8375687-95ea-42cf-b0bc-080d8705f1b4)
![image](https://github.com/user-attachments/assets/b0cdf2f5-23a0-4d7e-8c23-7b1d5d78bbf4)

# Hardware 
The hardware: 
