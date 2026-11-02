Day 1:

First Up component selection:
There are many esp323 modules since a fluidsim benefits from high compute i want to find the fastest clock
Esp32-s3 seems good enough

I need to add USB to UART for programming and whatever components the esp32 needs to operate
I found the devkit datasheet i will be mostly following it

Added Indicator LEDs but i should be finished with the devkit part now i need to hook up the imu and and tft touchscreen

Im a bit confused as to how to connect the tft display to the esp32 seems there multiple ways
Ok i hooked it all up the imu IC is almost a uC it has so much in such little space

Finished Schematic time for PCB
PCB was harder than expected but it turned out pretty compact

Day 2:

I think ill use TFT_eSPI with lvgl but the setup seems complicated 

15 -> TFT_CS
8 -> IMU SDA
9 -> IMU SCL
12 -> MISO TFT and Touch
13 -> MOSI TFT and Touch
14 -> SCK TFT and Touch
21 -> TFT Backlight LED
39 -> Touch_CS
40 -> Touch_IRQ
41 -> BMI_INT1
42 -> BMI_INT2
2 -> TFT_DC

At compile time tons of warnings because of deprecation -> Fixed
I hate how i cant put the config files in src

Weird i cant even put placeholders in src they have to be copies

Damn the code is too big i feel like lvgl is too much for this application i just need a shader not a whole gui
so ill remove it and just use TFT_eSPI but now i have to go clean up our halloween party

Ok im back and i wrote a skeleton of the code whith a simulated fragment shader

No i added the i2c interface with the IMU IC

Ok since this seems REAALLY complicated i'll try to implement it in python first then acctually write it in my esp32 code
Well since i dont have much time left i used AI to make something thats goodenough so that im insured incased i cant write it with the rest of the time i have

I might have to pivot to metaballs and a particle system because of esp32 ram limitations with whai have now i can only simulate 128X96 pixels then upscale it

The Plan:

Ill try to implement the metaball idea using the 2010 presentation about it
if it doesnt work ill pivot the ideo completely to a bliard like scene with hard balls the respond to external stimuli
and keep AI slop as backup

Ok apparently i dont need to add centripetall force because the imu feels it as its linear composition and returns it to me as acceleration values

I think im done now i had to resort to billiard ball model but i still have the fluid sim so ill keep the theme
