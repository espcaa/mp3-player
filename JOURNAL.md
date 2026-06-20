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
 <img src="assets/12-05-26_01.png" alt="" width="400"/>
\
 _basic rp2350b design_
\
\
<img src="assets/12-05-26_02.png" alt="" width="400"/>
\
 _multiple sheets for different parts of the design_
\
\
 **Total time spent: 5 hours**

## 14th May 2026

I forgot to journal yesterday's progress so this is going to cover that as well.
After diving deeper in bluetooth audio, I realized that going with a nrf was maybe not that good of an idea as it only supports LE Audio. I then did some research and found the FSC-BT1114QI that support both classic bluetooth audio & LE Audio! That means I can make my player future proof & still work with most things!.\
The only problem is price, 10$ :(\
\
I then designed a basic schematic & footprint since it doesn't seem to be very popular, and added it to my design!\
\
<img src="assets/14-05-26_01.png" alt="" width="400"/>
<img src="assets/14-05-26_02.png" alt="" width="400"/>
\
 _schematic & footprint for the bluetooth module_\
\
I finished the day by wiring that new module :p\
\
<img src="assets/14-05-26_03.png" alt="" width="400"/>
\
\
**Total time spent: 9 hours**

## 15th May 2026

Today is power management day ::: \
I first started by trying to add a DW01A to my pcb to protect the battery, but after doing some research I found out that it's not really necessary since most batteries already come with a protection circuit.\
This is what I ended up with:\

```mermaid
graph LR
    USB[USB 5V]:::source
    BATT[LiPo]:::source
    FuelGauge[Battery SOC ic]:::ic
    BQ["BQ24075 <br /> (charging and switching)"]:::ic
    Regulator[Buck-Boost<br>Converter]:::converter
    SYS[System Load]:::load

    USB --> BQ
    BATT --> FuelGauge
    FuelGauge --> BQ

    BQ -->|SYSTEM_POWER| Regulator
    Regulator -->|3V3| SYS
```

_hopefully it works :-:_\
\
wip schematic:\
\
<img src="assets/15-05-26_01.png" alt="" width="400"/>
\
\
**Total time spent: 11 hours**

## 16th May 2026

Today's mission was to wire the 3.5mm jack part of the audio setup! (dac :fear:)\
I think I somehow made it and it should work? hopefully?\
\
<img src="assets/16-05-26_01.png" alt="" width="400"/>
\
_schematic for the audio output (used kicad buses for the first time \o/)_\
\
I also added a power button to power on/off the device with a long/short press!\
\
<img src="assets/16-05-26_02.png" alt="" width="400"/>
\
_schematic for the power button_\
\
& random fixes here and there\
i need to sleep noww\
\
**Total time spent: 15 hours**

## 23rd May 2026

I locked in on a transatlantic flight and made a lot of progress!

- new power latch circuit
- better hierarchical sheet structure
- more buses :D
- io expanders to fit everything
- screen wiring

\
<img src="assets/23-05-26_01.png" alt="" width="400"/>
\
_power latch circuit_
\
\
<img src="assets/23-05-26_02.png" alt="" width="400"/>
\
_hierarchical sheets_
\
\
<img src="assets/23-05-26_03.png" alt="" width="400"/>
\
_io expander thingies_
\
\
<img src="assets/23-05-26_04.png" alt="" height="400"/>
\
_screennn_
\
\
<img src="assets/23-05-26_05.jpeg" alt="" width="400"/>
\
_bonus plane picture_

**Total time spent: 21 hours**

## 28th May 2026

Today I worked on the component assignment so that I can start routing!\
I also cleaned up the schematic everywhere & added some missing stuff.\
\
<img src="assets/28-05-26_01.png" alt="" width="400"/>
\
_component assignment_

**Total time spent: 25 hours**

## 29th May 2026

Today I fixed footprint issues & finished component assignment!\
Every component now has a footprint & a lcsc part number!!! \
\
<img src="assets/29-05-26_01.png" alt="" width="400"/>
\
_new jack wiring_
\
\
<img src="assets/29-05-26_02.png" alt="" width="400"/>
\
_component assignment done!_
\
\
<img src="assets/29-05-26_03.png" alt="" width="400"/>
\
_ready for routing!_
\
**Total time spent: 30 hours**

## 30th May 2026

Today I started routing my pcb! Well actually the first thing I needed to do before that was to find the exact form factor I was going for.\
Paper is very useful for that!\
\
<img src="assets/30-05-26_01.jpg" alt="" width="400"/>
\
_paper prototype :p_
\
\
After that, (and a bunch of iterations, and headaches over where to place the different connectors & ports) I ended up with this:\
\
<img src="assets/30-05-26_02.png" alt="" width="400"/>
\
_device outline_
\
\
I then started routing! I first did the analog audio part (no idea why) and then moved on to the power supply.
\
<img src="assets/30-05-26_03.png" alt="" width="400"/>
\
_routing..._  
\
I also fixed some schematic issues that I found while routing (wrong caps & inverted buses, other random things i forgot)

**Total time spent: 35 hours**

## 31st May 2026

I basically spent my day routing!\
A lot of the components moved around the board while I was trying to find the best arrangement for everything :sob:, but i think this is good enough?\
\
<img src="assets/31-05-26_01.png" alt="" width="400"/>
\
_routing..._
\
\
**Total time spent: 40 hours**

## 1st June 2026

June already...\
Today I continued routing the board! Did a lot of the annoying stuff (screen, sd card, spi..., dpad layout) & it's almost finished!!!\
\
<img src="assets/01-06-26_01.png" alt="" width="400"/>
\
_routing..._
\
\
**Total time spent: 43 hours**

## 3nd June 2026

Routing is finished! It took a lot of time because i actually moved a lot of traces around to try to keep the planes cleaner, remove useless vias where I could & add the debug header. I'm quite proud of how it turned out!\
\
<img src="assets/03-06-26_01.png" alt="" width="400"/>
\
_final routing_
\
\
I also tried to get a quote at jlcpcb and uh i might be cooked.\
\
<img src="assets/03-06-26_02.png" alt="" width="400"/>
\
_jlcpcb quote..._
\
\
I'm thinking of maybe getting a hotplate + stencil & soldering it myself.\
That'd be pretty cool + would sove the 69$ of extended component fee... I just need to trust myself not to mess it up & find a cheap hotplate pf...\
Now time for the case & firmware!\
Oh also, jlcpcb wants to charge me 40$ more to get a white pcb :sob:. Since my whole vision for the design was based on a white pcb + transparent case, I might just go with that & pay but I'm not sure\
\
**Total time spent: 49 hours**

## 4th June 2026

Today I went on a polishing spree!

- rounded the corners of the pcb
- made the fpc slot a bit larger so the connector doesn't break
- fixed footprints where mounting holes were tht holes instead of npt
- removed duplicated vias
- removed broken traces
- broken footprint with way too much solder mask
- connecting something to gnd (oops i forgot)

<img src="assets/04-06-26_01.png" alt="" width="400"/>

_0 errors omg_
\
\
Now I need to actually lockin on the case :sob:
**Total time spent: 51 hours**

## 12th June 2026

I'm back \o/\
Today I:

- added resistors on uart lines for the bluetooth module according to the datasheet
- redesigned the whole bluetooth module part to add antenna clearance because i forgot about it :sob:

<img src="assets/12-06-26_01.png" alt="" width="400"/>
\
_redesign_

**Total time spent: 54 hours**

## 14th June 2026

Today I focused on making a case for it!\
I made a lot of research on how i could join parts & battled with onshape to get my pcb & other components in my file.\
I also decided to go with simple petg instead of resin because I need this quickly/cheaply and I don't have a resin printer.\
Battery form factor changed to 505050 because else it's going to be very thick :sob:\
\
<img src="assets/14-06-26_01.png" alt="" width="400"/>
\
_new mounting holes_
\
\
<img src="assets/14-06-26_02.png" alt="" width="400"/>
\
_onshaep thingies_
\
**Total time spent: 57 hours**

## 15th June 2026

Turns out sleeping a normal amount of time makes you more productive!\
Today I really locked in & finished the case design!\
It's not perfect yet, but I think I'm 90% done with it!\
I had to move stuff around on my pcb & reroute some traces to make it fit better in the case (dpad moved up, horizontal buttons moved sideways, ...).\
(my pcb left side wasn't straight for some reason??? :sob:)\
\
<img src="assets/15-06-26_01.png" alt="" width="400"/>
\
_case design_
\
<img src="assets/15-06-26_02.png" alt="" width="400"/>
\
_case design_
\
<img src="assets/15-06-26_03.png" alt="" width="400"/>
\
_pcb update_
\
\
I'm very bad at CAD so this took forever, but I'm really happy with how it turned out!\
I'm thinking of printing different parts in different colors to make it look cooler :p. I could also bring different cases to opensauce & disassemble it to show the insides.

**Total time spent: 62 hours**

## 17th June 2026

This is going to be a long journal entry & a long commit cause I did a lot today & forgot to journal yesterday :pensive:\
I started the firmware!\
I first started by setting up the project structure & designing a hal so that I can run the ui on my computer to test it!\
\
<img src="assets/17-06-26_01.png" alt="" width="400"/>
\
_first time the ui ran on my computer!_
\
\
I then started working on the ui itself!\
Did some figma design with a friend but didn't really end up using it\
\
<img src="assets/17-06-26_02.png" alt="" width="400"/>
\
_ui design_
\
\
I then started implementing some basic ui system (text, images, buttons, widget system...)!\
\
<img src="assets/17-06-26_03.png" alt="" width="400"/>
\
_totally what I wanted to write trust_
\
\
After a lot of trial & error, I got something to work with the virtual sdcard!\
\
<img src="assets/17-06-26_04.png" alt="" width="400"/>
<img src="assets/17-06-26_05.png" alt="" width="400"/>
\
_basic music player!!!_
\
\
I also made a bunch of tooling for fonts, upload to the sdcard & running the emulator.\
No working music or settings or anything yet but this is definitely going somewhere!!!

**Total time spent: 70 hours**

## 18th June 2026

Today I continued my work on the firmware...\
Tbh i'm getting quite bored of that but we need to ship this thing :sob:\
\
Things I did today:

- way better fonts
- fixed padding/margin things
- new now playing screen
- status bar!
- better dpad navigation

I didn't really worked on the pico sdk implementation of the hal though... so that's what I need to focus on tomorrow.\
\
<img src="assets/18-06-26_01.png" alt="" width="400"/>
\
_new now playing screen_
\
\
<img src="assets/18-06-26_02.png" alt="" width="400"/>
\
_new library_

**Total time spent: 75 hours**

## 19th June 2026

New firmware features:

- charging & battery icon
- volume control
- settings screen
- bluetooth pairing & connecting screen + status bar integration
- mp3 decoding & playback actually working
- pico hal!

Screenshots!:\
\
<img src="assets/19-06-26_01.png" alt="" width="400"/>
<img src="assets/19-06-26_02.png" alt="" width="400"/>

**Total time spent: 81 hours**
