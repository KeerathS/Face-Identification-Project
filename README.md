# Face-Identification-Project

This system is a prototype proof of concept for a smart security system that integrates
machine learning–based face recognition with real-time feedback. An ESP32-Wrover board
with a camera runs a TensorFlow Lite model trained to distinguish between two individual
faces. Once a face is recognized, the ESP32 sends the result via UART to an
STM32F429I-Discovery board, which displays the message on its onboard LCD. The message
is automatically cleared after a short delay, allowing the system to refresh for the next detection.
An onboard LED provides visual confirmation when specific identities such as “Keerath (0.xyz)”
are received. The system ensures reliable communication, live inference, and efficient
embedded feedback. The overall goal is to demonstrate how edge AI and microcontroller
coordination can be used to build intelligent, low-power security or identity verification systems.



FSM DIAGRAM:
![FSM](https://github.com/user-attachments/assets/768216f8-08b3-4d66-b600-d9bf6a21f321)
