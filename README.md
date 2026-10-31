# FluidSim

## An Interactive Fluid Simulation on the ESP32-S3

![PCB Model Image](imgs/pcbmodel.png)

![PCB Image](imgs/pcb.png)

### What is it ?

This is a LCD TFT Display Based Fluid Simulation Computer running on an ESP32-S3 an IMU, as well as the TFT Touchscreen input act as Influences on the Simulation for example accelerating or and rotating the board will apply a force on the whole fluid, touching the board will have a similar effect as stirring the fluid with a spoon

### Why does it exist ?

This is an exploration of the esp32-s3's capabilities and its goal is to help learn about the navier stokes equations which govern fluid simulations and to challenge me to optimize the programm for it to hopefully run at an acceptable framerate.

### How does it Work ?

On each Frame the esp32 gets the influences from the board (touch, acceleration, ratation). It then solves the navier stokes equation with the influence terms added and for each pixel and assigns a color based on the velocity and pressure of the fluid inside that pixel 

### How would I use it ?

You'd first assemble the board then use the built in USB to UART converter to flash the firmware to the ESP32 and connect a 5V supply the the Power header
then it runs automatically.

## Bill of Materials

