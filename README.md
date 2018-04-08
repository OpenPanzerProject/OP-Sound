![Open Panzer](http://www.openpanzer.org/images/github/soundcard_git_v2.jpg)

# Open Panzer Sound Card
The Open Panzer Sound Card is a work in progress with the goal of bringing inexpensive, high quality, and open source sound functionality to RC models but especially to tanks using the [Tank Control Board (TCB)](https://github.com/OpenPanzerProject/TCB). The board is actually made up of two components. First, an off-the-shelf [PJRC Teensy 3.2](https://www.pjrc.com/store/teensy32.html) is used as the onboard processor. The Teensy is then plugged into a socket on our custom carrier board that adds a Micro SD card slot (max 32 GB) a [Maxim 9768](https://datasheets.maximintegrated.com/en/ds/MAX9768.pdf) 10 watt mono amplifier, and headers for external connections. 

## Resources
  * A discussion thread on the development of this card can be found on the [Open Panzer Forum](http://openpanzer.org/forum/index.php?topic=17.0).
  * [Eagle Board and Schematic](http://openpanzer.org/downloads/soundcard/eagle/OP_Sound_v2_r3.zip) (zip)
  * [Printable Schematic](http://openpanzer.org/downloads/soundcard/eagle/OP_Sound_v2_r3_Schematic.pdf) (pdf)
  * Bill of Materials - [PDF](http://openpanzer.org/downloads/soundcard/bom/OP_Sound_BOM.pdf) - [Excel](http://openpanzer.org/downloads/soundcard/bom/OP_Sound_BOM.xls)
  * [Bare boards at OSH Park](https://oshpark.com/shared_projects/YumYMk9Z)

## Loading Firmware
If you don't need to make changes to the firmware, the easiest way to load the latest code onto your Teensy is to use the OP Config program [available here](http://openpanzer.org/downloads). After installation, go to the Firmware tab, select "Sound Card" from the firmware drop down, click the "Get Latest Release" button, and then click the Flash button.

## Compiling Firmware
If you want to modify the firmware yourself it can be compiled in the Arduino IDE using the [Teensyduino Add-On](https://www.pjrc.com/teensy/td_download.html). After installing the add-on, open the sketch in the Arduino IDE and under the Tools menu select Board - "Teensy 3.2/3.1". 

After installing Teensyduino it is also recommended you turn on the optimized SD code by editing the following file (replace "C:\Arduino\" with your Arduino install directory):  
`C:\Arduino\hardware\teensy\avr\libraries\SD\SD_t3.h`

Un-comment the line at the top of that file:  
`#define USE_TEENSY3_OPTIMIZED_CODE`


## Features
  * Supports 16 bit 44,100 Hz WAV files.
  * Up to 6 simultaneous sounds can be played at once from the memory card - 2 reserved for engine, 4 for effects. However if you are using track overlay sounds, note that these will reserve 2 slots, leaving only 2 remaining for simultaneous effects. Additional sounds can be played simultaneously from the onboard flash (not yet implemented in firmware). One thought is to add common sounds (mg, repair, perhaps cannon hits or destroyed sounds) to the onboard flash. 
  * Volume control - either with a physical potentiometer or, when used with the TCB, directly from a knob on your transmitter. In fact you can use both and the card will intelligently respond to whichever volume control is currently being adjusted. You can also adjust relative volumes of engine, track overlay and effects from within [OP Config](https://github.com/OpenPanzerProject/OP-Config). 
  * Three-part sounds for various effects such as barrel elevation, turret rotation, machine gun fire, and others - this permits a distinct sound to be played at the start of the effect, then a separate looping portion, then a closing sound. The firmware is intelligent enough to determine if the special start and ending sounds are present or not, if you don't have one or the other just leave them off the memory card and the looping portion will still play. 
  * Six custom user sounds in addition to all the usual model sound effects. 
  * Support for hot and cold start engine sounds.
  * Multiple versions of sounds can be used for various engine effects, for example, you can assign up to 5 distinct idle sounds and the card will choose a different one each time the vehicle returns to idle. But if you don't have 5 idle sounds don't worry, the firmware is smart enough to automatically use as few or as many as you put on the memory card. 
  * Designed to work seamlessly with the Open Panzer TCB, but can also be controlled directly via standard RC inputs for use in other models. Up to 5 RC channels can be read. However not all options are available in RC mode and firmware development efforts are presently focused on enhancing serial control via the TCB. 

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
For full functionality the sound card is designed to be paired with the [Open Panzer Tank Control Board (TCB)](http://openpanzer.org/wiki/doku.php?id=wiki:tcb:tcbinstall:boardlayout). Some minimal control can be accomplished with standard RC gear (see below), but when used with the TCB the RC channels are not used and should be left disconnected. 

## General RC Usage
For full functionality the sound card is designed to be paired with the [Open Panzer TCB](https://github.com/OpenPanzerProject/TCB). But some minimal sounds can optionally be accomplished without the TCB using standard RC signals. Even more sophisticated options could be possible with further firmware development, but since RC control is not our primary focus we will leave that endeavor to the open source community.
  * **Channel 1** - Throttle. If engine on, channel centered is idle, movement in either direction increases engine speed.
  * **Channel 2** - Engine on/off (2-position switch). Pulse width greater than 1500 uS turns engine on, less than 1500 uS turns engine off.
  * **Channel 3** - Cannon/MG (3-position switch). Channel center is no sound, switch low (1000 uS) plays machine gun sound, switch high (2000 uS) plays cannon fire sound.
  * **Channel 4** - User sounds (3-position switch). Channel center is no sound, switch low (1000 uS) plays user1.wav, switch high (2000 uS) plays user2.wav
  * **Channel 5** - Volume control (knob). Use to adjust sound card volume. If not needed, use a standard pot physically attached to board.

## Sound Files
The sound card requires basically no configuration when paired with the TCB, other than adding your desired sounds to the memory card. The sound card identifies the function of each sound by its file name, so you must name your files exactly as shown in the table below. Note we are limited to the 8n3 format, meaning file names cannot exceed 8 characters. Every sound is not required, if any are omitted the card will simply ignore the sound for that function.

Sound files must be saved as **16 bit 44,100 Hz WAV files**. It doesn't matter if the files are in mono or stereo format, but since the card can only drive a single speaker any stereo files will be output as mono. 

Turret rotation, barrel elevation and machine gun sounds have a repeating portion that will loop continuously so long as the effect is active. But you can also specify lead-in and lead-out sounds that will play once at the beginning or end of the sound effect. Again these are optional and can be omitted if desired.

Squeaks are played at random intervals only when the vehicle is moving. The interval span for each squeak must be defined by the device controlling the sound card - in our case this will be the TCB. Therefore squeak intervals are adjusted using OP Config and saved to the TCB, which will communicate them to the sound card.

<html>
	<table width="700px">
		<tr><td colspan="4" align="left"><br/><b>Sound Effects</b></td></tr>
		<tr>
			<th width="20%" align="left">Effect</th>
			<th width="20%" align="left">File Name</th>
			<th width="18%" align="left">Optional</th>
			<th width="42%" align="left">Notes</th>
		</tr>
		<tr>
			<td valign="top">Turret Rotation</td>
			<td valign="top">turret.wav</td>
			<td valign="top">tr_start.wav<br/>tr_stop.wav</td>
                        <td>The optional start and stop sounds will play at the beginning or end of the repeating portion, if present.</td>			
		</tr>
		<tr>
			<td valign="top">Turret Rotation<br/>w/ Engine Off</td>
			<td valign="top">turret_m.wav</td>
                        <td valign="top">tr_strtm.wav<br/>tr_stopm.wav</td>
                        <td valign="top">An optional version of turret rotation sounds that will play only when the vehicle is not in the running state. If omitted, the regular turret sounds above will play regardless of the engine state. The optional start (tr_strtm) and stop (tr_stopm) sounds will play at the beginning or end of the repeating portion, if present.</td>
		</tr>
                 <tr>
			<td valign="top">Barrel Elevation</td>
			<td valign="top">barrel.wav</td>
			<td valign="top">br_start.wav<br/>br_stop.wav</td>
                        <td>The optional start and stop sounds will play at the beginning or end of the repeating portion, if present.</td>
		</tr>
		<tr>
			<td valign="top">Machine Gun Fire</td>
			<td valign="top">mg.wav</td>
			<td valign="top">mg_start.wav<br/>mg_stop.wav</td>
                        <td>The optional start and stop sounds will play at the beginning or end of the repeating portion, if present.</td>	
		</tr>
		<tr>
			<td valign="top">Second Machine Gun Fire</td>
			<td valign="top">mg2.wav</td>
			<td valign="top">mg2start.wav<br/>mg2stop.wav</td>
                        <td>Use these sounds for a second machine gun on your model. The optional start and stop sounds will play at the beginning or end of the repeating portion, if present.</td>	
		</tr>
		<tr>
			<td>Machine Gun Hit</td>
			<td>mghit.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td valign="top">Cannon Fire</td>
			<td valign="top">cannonf.wav</td>
                        <td valign="top">cannonf2.wav<br/>cannonf3.wav<br/>cannonf4.wav<br/>cannonf5.wav<br/></td>
			<td valign="top">If multiple cannon fire sounds are specified, a different one will be played each time.</td>
		</tr>
		<tr>
			<td valign="top">Cannon Ready</td>
			<td valign="top">reloaded.wav</td>
                        <td valign="top"></td>
			<td valign="top">Notification sound to let the user know they can fire the cannon again. Actual reload time is determined by the vehicle's weight class, defined in <a href="http://openpanzer.org/wiki/doku.php?id=wiki:opconfig:tabs:battle">OP Config on the Battle tab</a>.</td>
		</tr>
<tr>
			<td>Cannon Hit</td>
			<td>cannonh.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td>Vehicle Destroyed</td>
			<td>destroy.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td>Vehicle Repair</td>
			<td>repair.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td valign="top">Light Switch</td>
			<td valign="top">light.wav</td>
			<td valign="top"></td>
			<td valign="top">Will play when the headlight is turned on or off (L1 port on TCB).</td>
		</tr>
		<tr>
			<td valign="top">Light 2 Switch</td>
			<td valign="top">light2.wav</td>
			<td></td>
			<td valign="top">Will play when Light 2 is turned on or off (L2 port on TCB).</td>
		</tr>
                <tr>
			<td valign="top">Braking Sound</td>
			<td valign="top">brake.wav</td>
			<td></td>
			<td valign="top">Will play automatically when vehicle is braked. To disable, simply omit the sound file.</td>
		</tr>		
		<tr>
			<td valign="top">Transmission</td>
			<td valign="top">txengage.wav<br/>txdegage.wav</td>
			<td valign="top"></td>
                        <td valign="top">Sounds to play when the transmission is manually engaged or disengaged by the user.</td>
		</tr>		
		<tr>
			<td valign="top">Squeaks</td>
			<td valign="top">squeak1.wav<br/> squeak2.wav<br/> squeak3.wav<br/> squeak4.wav<br/> squeak5.wav<br/> squeak6.wav</td>
			<td></td>
			<td valign="top">Squeak frequency is defined by the controlling device, ie the TCB (see the Sounds tab of OP Config)</td>
		</tr>
		<tr>
			<td valign="top">Track<br/>Overlay<br/>Sounds</td>			<td valign="top">tracks1.wav<br/>tracks2.wav<br/>tracks3.wav<br/>tracks4.wav<br/>tracks5.wav<br/>tracks6.wav<br/>tracks7.wav<br/>tracks8.wav<br/>tracks9.wav<br/>tracks10.wav</td>
			<td valign="top">trkstart.wav<br/>trkstop.wav</td>
                        <td valign="top">Track/tread overlay sounds when vehicle moving, with tracks1.wav played at the slowest speed and higher numbers being faster speed. Not all sounds are required, as many as are included will automatically be evenly distributed across the entire speed range. Optional start and stop sounds can be added that will play when the vehicle first begins to move or just comes to a stop.</td>
		</tr>	
		<tr>
			<td valign="top">User Sounds</td>
			<td valign="top">user1.wav<br/>user2.wav<br/>user3.wav<br/>user4.wav<br/>user5.wav<br/>user6.wav <br/>user7.wav<br/>user8.wav<br/>user9.wav<br/>user10.wav<br/>user11.wav<br/>user12.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td>Confirmation Beep</td>
			<td>beep.wav</td>
			<td></td>
			<td></td>
		</tr>		
	</table>
</html>
<html>
	<table width="700px">
		<tr><td colspan="3" align="left"><br/><b>Engine Sounds</b></td></tr>
		<tr>
			<th width="20%" align="left">Effect</th>
			<th width="20%" align="left">File Name</th>
			<th width="60%" align="left">Notes</th>
		</tr>
		<tr>
			<td valign="top">Engine Start</td>
			<td valign="top">enstart1.wav</td>
			<td valign="top">Default/Cold start</td>
		</tr>
		<tr>
			<td valign="top">Engine Hot Start</td>
			<td valign="top">enstart2.wav</td>
			<td valign="top">Optional, if present will use on second and subsequent starts</td>
		</tr>
		<tr>
			<td valign="top">Engine Idle</td>
			<td valign="top">enidle1.wav<br/>enidle2.wav<br/>enidle3.wav<br/>enidle4.wav<br/>enidle5.wav</td>
			<td valign="top">If multiple idle sounds specified, a different one will be played each time vehicle returns to idle.</td>
		</tr>
		<tr>
			<td valign="top">Engine Idle<br/>- Damaged State</td>
			<td valign="top">enidle_d.wav</td>
			<td valign="top">An optional alternate idle sound that will be played when the vehicle has sustained damage in IR battle (in the manner of Tamiya).</td>
		</tr>
                <tr>
			<td valign="top">Engine Accelerate</td>
			<td valign="top">enaccl1.wav<br/>enaccl2.wav<br/>enaccl3.wav<br/>enaccl4.wav<br/>enaccl5.wav</td>
			<td valign="top">The acceleration sound is played as a transition from idle to moving.
                            If multiple sounds specified, a different one will be played each time vehicle begins moving.</td>
		</tr>
		<tr>
			<td valign="top">Engine Decelerate</td>
			<td valign="top">endecl1.wav<br/>endecl2.wav<br/>endecl3.wav<br/>endecl4.wav<br/>endecl5.wav</td>
			<td valign="top">The deceleration sound is played as a transition from moving
                            to stopped (idle). If multiple sounds specified, a different one will be
                            played each time vehicle stops moving.</td>
		</tr>
		<tr>
			<td valign="top">Vehicle Moving</td>
			<td valign="top">enrun1.wav<br/>enrun2.wav<br/>enrun3.wav<br/>enrun4.wav<br/>enrun5.wav<br/>                       enrun6.wav<br/>enrun7.wav<br/>enrun8.wav<br/>enrun9.wav<br/>enrun10.wav</td>
			<td valign="top">Engine sound when vehicle moving, with enrun1.wav being the slowest speed and higher numbers being faster speed. Not all sounds are required, as many are specified will automatically be evenly distributed across the entire speed range.</td>
		</tr>
		<tr>
			<td valign="top">Engine Shutdown</td>
			<td valign="top">enstop.wav</td>
			<td valign="top"></td>
		</tr>		
	</table>
</html>
