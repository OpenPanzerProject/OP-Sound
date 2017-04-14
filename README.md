![Open Panzer](http://www.openpanzer.org/images/github/soundcard_git_beta.jpg)

# Open Panzer Sound Card
The Open Panzer Sound Card is a work in progress with the goal of bringing inexpensive, high quality, and open source sound functionality to RC models but especially to tanks. The board is actually made up of two components. First, a [PJRC Teensy 3.2](https://www.pjrc.com/store/teensy32.html) is used as the onboard processor. Secondly, a carrier board that adds an SD card slot (max 32 GB), an additional 16 MB of flash memory, an [LM48310](https://www.digikey.com/product-detail/en/texas-instruments/LM48310SD-NOPB/LM48310SD-NOPBCT-ND/1765468) 2.6 watt audio amplifier, and headers for exernal connections. 

A discussion thread on the development of this card can be found on the [Open Panzer Forums](http://openpanzer.org/forum/index.php?topic=17.0). 

## Features
  * Supports 16 bit 44,100 Hz WAV files.
  * Up to 5 simultaneous sounds can be played at once from the SD card - 2 reserved for engine, 3 for effects. Additional sounds can be played simultaneously from the onboard flash. One thought is to add common sounds (mg, repair, perhaps cannon hits or destroyed sounds) to the onboard flash. 
  * Volume control - either with a physical potentiometer or, when used with the TCB, directly from a knob on your transmitter. In fact you can use both and the card will intelligently respond to whichever volume control is currently being adjusted. 
  * Three-part sounds for various effects such as barrel elevation, turret rotation, machine gun fire, and others - this permits a distinct sound to be played at the start of the effect, then a separate looping portion, then a closing sound. The firmware is intelligent enough to determine if the special start and ending sounds are present or not, if you don't have one or the other just leave them off the SD card and the looping portion will still play. 
  * Six custom user sounds in addition to all the usual model sound effects. 
  * Support for hot and cold start engine sounds.
  * Multiple versions of sounds can be used for various engine effects, for example, you can assign up to 5 distinct idle sounds and the card will randomly choose one each time the vehicle returns to idle. But if you don't have 5 idle sounds don't worry, the firmware is smart enough to automatically use as few or as many as you put on the SD card. 
  * Designed to work seamlessly with the Open Panzer TCB, but can also be controlled directly via standard RC inputs for use in other models. Up to 5 RC channels can be read. However not all options are available in RC mode and firmware development efforts are presently focused on enhancing serial control via the TCB. 

## SD Card Notes
Not all SD cards are created equal. To get reliable simultaneous sound performance, we recommend using SanDisk Ultra SD cards [such as these](https://www.amazon.com/gp/product/B010Q57T02). Further testing may identify other brands that work.  
