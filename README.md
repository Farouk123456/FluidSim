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

| ID | Name                         | Quantity | Supplier | Supplier Part | Price | link                                               |
|----|------------------------------|----------|----------|---------------|-------|----------------------------------------------------|
| 1  | 10uF                         | 20       | LCSC     | C22387953     | 0.02  | https://www.lcsc.com/product-detail/C22387953.html |
| 2  | 100nF                        | 20       | LCSC     | C2832506      | 0.022 | https://www.lcsc.com/product-detail/C2832506.html  |
| 3  | 1uF                          | 10       | LCSC     | C5632429      | 0.053 | https://www.lcsc.com/product-detail/C5632429.html  |
| 4  | 1N5819HW-7-F                 | 10       | LCSC     | C82544        | 0.059 | https://www.lcsc.com/product-detail/C82544.html    |
| 5  | HDR-M-2.54_1x2               | 20       | LCSC     | C124375       | 0.028 | https://www.lcsc.com/product-detail/C124375.html   |
| 6  | 15-21SURC/S530-A3/3T         | 5        | LCSC     | C2984265      | 0.023 | https://www.lcsc.com/product-detail/C2984265.html  |
| 7  | L8050QLT1G                   | 50       | LCSC     | C49581        | 0.02  | https://www.lcsc.com/product-detail/C49581.html    |
| 8  | 10kΩ                         | 10       | LCSC     | C714145       | 0.041 | https://www.lcsc.com/product-detail/C714145.html   |
| 9  | 50Ω                          | 10       | LCSC     | C47116135     | 0.033 | https://www.lcsc.com/product-detail/C47116135.html |
| 10 | 47.5kΩ                       | 20       | LCSC     | C385390       | 0.027 | https://www.lcsc.com/product-detail/C385390.html   |
| 11 | LXW-TS-6X6X5-SMD             | 20       | LCSC     | C53447020     | 0.022 | https://www.lcsc.com/product-detail/C53447020.html |
| 12 | SGM2212-3.3XKC3G/TR_C3294699 | 1        | LCSC     | C3294699      | 0.574 | https://www.lcsc.com/product-detail/C3294699.html  |
| 13 | LESD5D5.0CT1G-ES             | 20       | LCSC     | C5199159      | 0.015 | https://www.lcsc.com/product-detail/C5199159.html  |
| 14 | BMI270                       | 1        | LCSC     | C2836813      | 2.618 | https://www.lcsc.com/product-detail/C2836813.html  |
| 15 | ESP32-S3-WROOM-1(N16R8)      | 1        | LCSC     | C2913202      | 5.403 | https://www.lcsc.com/product-detail/C2913202.html  |
| 16 | 240x320_2.8_TFT_TOUCH_SD     | 1        | Amazon   | B0CQ7VQB3P    | 16.99 | https://www.amazon.de/-/en/gp/product/B0CQ7VQB3P   |
| 17 | 22.1kΩ                       | 50       | LCSC     | C2848551      | 0.007 | https://www.lcsc.com/product-detail/C2848551.html  |
| 18 | CP2102N-A02-GQFN28R          | 1        | LCSC     | C964632       | 1.875 | https://www.lcsc.com/product-detail/C964632.html   |
| 19 | MINI 5PTP                    | 10       | LCSC     | C2681563      | 0.07  | https://www.lcsc.com/product-detail/C2681563.html  |
