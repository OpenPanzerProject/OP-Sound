![Open Panzer](http://www.openpanzer.org/images/github/soundcard_git_v3.jpg)

# Open Panzer Sound Card
The Open Panzer Sound Card is a work in progress with the goal of bringing inexpensive, high quality, and open source sound functionality to RC models but especially to tanks using the [Tank Control Board (TCB)](https://github.com/OpenPanzerProject/TCB). The board is actually made up of two components. First, an off-the-shelf [PJRC Teensy 3.2](https://www.pjrc.com/store/teensy32.html) is used as the onboard processor. The Teensy is then plugged into a socket on our custom carrier board that adds a Micro SD card slot (max 32 GB) a [Maxim 9768](https://datasheets.maximintegrated.com/en/ds/MAX9768.pdf) 10 watt mono amplifier, and headers for external connections. 

## Resources
  * A discussion thread on the development of this card can be found on the [Open Panzer Forum](http://openpanzer.org/forum/index.php?topic=17.0).
  * [Eagle Board and Schematic (v3)](http://openpanzer.org/downloads/soundcard/eagle/OP_Sound_v3_r1.zip) (zip)
  * [Printable Schematic](http://openpanzer.org/downloads/soundcard/eagle/OP_Sound_v3_r1_Schematic.pdf) (pdf)
  * Bill of Materials - [PDF](http://openpanzer.org/downloads/soundcard/bom/OP_Sound_BOM_v3.pdf) - [Excel](http://openpanzer.org/downloads/soundcard/bom/OP_Sound_BOM_v3.xls)
  * [Bare boards at OSH Park](https://oshpark.com/shared_projects/6f5U9GD4)

## Loading Firmware
If you don't need to make changes to the firmware, the easiest way to load the latest code onto your Teensy is to use the OP Config program [available here](http://openpanzer.org/downloads). After installation, go to the Firmware tab, select "Sound Card" from the firmware drop down, click the "Get Latest Release" button, and then click the Flash button.

## Compiling Firmware
If you want to modify the firmware yourself it can be compiled in the Arduino IDE using the [Teensyduino Add-On](https://www.pjrc.com/teensy/td_download.html). After installing the add-on, open the sketch in the Arduino IDE and under the Tools menu select Board - "Teensy 3.2/3.1". 

After installing Teensyduino it is also recommended you turn on the optimized SD code by editing the following file (replace "C:\Arduino\" with your Arduino install directory):  
`C:\Arduino\hardware\teensy\avr\libraries\SD\SD_t3.h`

Un-comment the line at the top of that file:  
`#define USE_TEENSY3_OPTIMIZED_CODE`

## Micro SD Card Notes
Not all SD cards are created equal. To get reliable simultaneous sound performance, we recommend using SanDisk Ultra SD cards [such as these](https://www.amazon.com/gp/product/B010Q57T02). The maximum size support is 32 GB. Format should be FAT32.

![SanDisk Ultra](http://www.openpanzer.org/images/github/sandiskultra_32gb.jpg)

## LED Key
The sound card has two status LEDs, one blue and one red. On startup, the red LED will blink rapidly if unable to read the memory card, otherwise it will blink slowly until an input signal is received, either from the serial port or an RC channel. Whichever type is detected first is the mode the sound card will use until the next reboot. Once a signal is detected the red LED will turn off.

If a memory card error is indicated, turn off power to the device. Check to make sure your memory card is present and inserted all the way, and that sound files on the card are in the correct format and named correctly (see the table below for file names).

So long as the input is active the blue LED will remain solid. If in RC mode and connection is lost on all 5 channels, the blue LED will blink rapidly. If in Serial mode the blue LED may blink slowly if no command has been given for a length of time - this is not an error, it simply indicates idle status.    

![SanDisk Ultra](http://www.openpanzer.org/images/github/opsound_ledpatterns.jpg)

## Test Mode
When the sound card is powered up with a jumper attached to RC input 1 such that the signal pin is held to ground, it will enter a test routine. During this mode the device will sequentially play in full each sound that it finds on the memory card, or at least each sound that it knows to look for (see the full list below). Debugging information will be printed out the USB port while this occurs. When every sound has been played it will start over from the beginning and repeat this process continuously until the jumper is removed. This can be useful in identifying which sounds the device thinks it can find, and whether they sound as you expect, etc... 

## Use with TCB
For full functionality the sound card is designed to be paired with the [Open Panzer Tank Control Board (TCB)](http://openpanzer.org/wiki/doku.php?id=wiki:tcb:tcbinstall:boardlayout). When used with the TCB the RC channels are not used and should be left disconnected. 

## General RC Usage

