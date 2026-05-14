## 12th May 2026

Today is the first day of this project :) \
I decided to work on this because I wanted a fun and "simple" project to work on! Okay, maybe not that simple if I keep overengineering it aaaaa \
\
Ideas:

- rp2350b to drive the display & buttons & audio output
- nrf54l1O for bluetooth
- full size sd card for storage (love them, i lost so many micro sd cards)
- spi display

Final components list: (might change a lot)

- rp2350b
- nrf54l1O
- 1.3 inch spi display
- 2.8 inch ips screen - [link](https://www.buydisplay.com/bar-type-2-8-inch-480x640-ips-tft-lcd-display-spi-rgb-interface)
- PCM5102A (i don't know anything about dacs, so this is likely to change)
- 5 buttons (dpad) + 1 for power
- 1 rotary encoder for volume control
- 1 neopixel

After that, I started working on the schematic of my pcb! So far, I followed raspberry pi's reference design to get the rp2350b working & added the sd card.\
\
Screenshots:\
\
 <img src="assets/12-05-26_01.png" alt="first picture" width="400"/>
\
 _basic rp2350b design_
\
<img src="assets/12-05-26_02.png" alt="second picture" width="400"/>
\
 _multiple sheets for different parts of the design_
\
\
 **Total time spent: 5 hours**

## 14th May 2026

I forgot to journal yesterday's progress so this is going to be for yesterday too...\
After diving deeper in bluetooth audio, I realized that going with a nrf was maybe not that good of an idea as it only supports LE Audio. I then did some research and found the FSC-BT1114QI that support both classic bluetooth audio & LE Audio! That means I can make my player future proof & still work with most things!.\
The only problem is price, 10$ :(\
\
I then designed a basic schematic & footprint since it doesn't seem to be very popular, and added it to my design!\
\
<img src="assets/14-05-26_01.png" alt="third picture" width="400"/>
<img src="assets/14-05-26_02.png" alt="fourth picture" width="400"/>
\
 _schematic & footprint for the bluetooth module_\
\
\
I finished the day by wiring that new module :p\
\
<img src="assets/14-05-26_03.png" alt="fifth picture" width="400"/>
\
\
**Total time spent: 4 hours**
