![Open Panzer](http://www.openpanzer.org/images/github/soundcard_git_v3.jpg)

# Open Panzer Sound Card
The Open Panzer Sound Card is a work in progress with the goal of bringing inexpensive, high quality, and open source sound functionality to RC models - either in combination with the [Tank Control Board (TCB)](https://github.com/OpenPanzerProject/TCB) or as a standalone device controlled by standard hobby RC. The board is actually made up of two components. First, an off-the-shelf [PJRC Teensy 3.2](https://www.pjrc.com/store/teensy32.html) is used as the onboard processor. The Teensy is then plugged into a socket on our custom carrier board that adds a Micro SD card slot (max 32 GB) a [Maxim 9768](https://datasheets.maximintegrated.com/en/ds/MAX9768.pdf) 10 watt mono amplifier, and headers for external connections. 

## Resources
  * A discussion thread on the development of this card can be found on the [Open Panzer Forum](https://openpanzer.org/forum/index.php?topic=17.0).
  * [Eagle Board and Schematic (v3)](https://openpanzer.org/secure_downloads/soundcard/eagle/OP_Sound_v3_r1.zip) (zip)
  * [Printable Schematic](https://openpanzer.org/secure_downloads/soundcard/eagle/OP_Sound_v3_r1_Schematic.pdf) (pdf)
  * Bill of Materials - [PDF](https://openpanzer.org/secure_downloads/soundcard/bom/OP_Sound_BOM_v3.pdf) - [Excel](https://openpanzer.org/secure_downloads/soundcard/bom/OP_Sound_BOM_v3.xls)
  * [Bare boards at OSH Park](https://oshpark.com/shared_projects/6f5U9GD4)
  * [Solder paste stencils at OSH Stencils](https://www.oshstencils.com/#projects/e5703b7c66119b959bbf3e3a7f4f89331f41fd27)
  * More: [Open Panzer Sound Card Complete Resource List](https://www.openpanzer.org/downloads#sound)

## Loading Firmware
If you don't need to make changes to the firmware, the easiest way to load the latest code onto your Teensy is to use the [OP Config program](https://openpanzer.org/downloads) (for those using the TCB), or the [INI Creator Utility](https://openpanzer.org/downloads#sound) (for those using the sound card in RC mode). In either case, after installation of one of those Windows programs connect your sound card to the computer via USB, go to the Firmware tab, click the "Get Latest Release" button, and then Flash the firmware to the sound card.

## Compiling Firmware
If you want to modify the firmware yourself it can be compiled in the Arduino IDE using the [Teensyduino Add-On](https://www.pjrc.com/teensy/td_download.html). After installing the add-on, open the sketch in the Arduino IDE and under the Tools menu select Board - "Teensy 3.2/3.1". 

After installing Teensyduino it is also recommended you turn on the optimized SD code by editing the following file (replace "C:\Arduino\" with your Arduino install directory):  
`C:\Arduino\hardware\teensy\avr\libraries\SD\SD_t3.h`

Un-comment the line at the top of that file:  
`#define USE_TEENSY3_OPTIMIZED_CODE`

## Instructions for Use
Instructions are different depending on how the sound card is being used, either in combination with the TCB or controlled directly through RC. In either case, the general principle is to load your sound files on a Micro SD memory stick which is inserted into the sound card. Files must be 16 bit 44,100 Hz WAV files. It doesn't matter if they are stereo or mono but since the sound card only drives a single speaker, any stereo files you do have will be played in mono. Sound filenames define their function. For those using the sound card with the TCB, configuration of options is accomplished via the TCB's configuration program called [OP Config](https://openpanzer.org/downloads). For those using the sound card in generic RC models, settings must be read from an INI file that you place on the Micro SD memory card along with your sound files. Never fear, we have a convenient Windows utility that will create the INI file for you, called [INI Creator](https://openpanzer.org/downloads#sound) 

Go to these pages in the Open Panzer Wiki to read more about using the sound card: 
  * [Sound Card with TCB](https://openpanzer.org/wiki/doku.php?id=wiki:tcb:tcbinstall:sound_op)
  * [Sound Card in RC Mode](https://openpanzer.org/wiki/doku.php?id=wiki:sound:start)

## Micro SD Card Notes
Not all SD cards are created equal. To get reliable simultaneous sound performance, we recommend using SanDisk Ultra Micro SD cards [such as these](https://www.amazon.com/gp/product/B010Q57T02). The maximum size support is 32 GB. Format should be FAT32.

![SanDisk Ultra](https://www.openpanzer.org/images/github/sandiskultra_32gb.jpg)


## License
Firmware for the Open Panzer Sound Card is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

For more specific details see [http://www.gnu.org/licenses](http://www.gnu.org/licenses), the [Quick Guide to GPLv3.](http://www.gnu.org/licenses/quick-guide-gplv3.html) and the [copying.txt](https://github.com/OpenPanzerProject/TCB/blob/master/COPYING.txt) file in the codebase.

The GNU operating system which is under the same license has an informative [FAQ here](http://www.gnu.org/licenses/gpl-faq.html).
