![Open Panzer](http://www.openpanzer.org/images/github/soundcard_git_beta.jpg)

# Open Panzer Sound Card
The Open Panzer Sound Card is a work in progress with the goal of bringing inexpensive, high quality, and open source sound functionality to RC models but especially to tanks using the [Tank Control Board (TCB)](https://github.com/OpenPanzerProject/TCB). The board is actually made up of two components. First, an off-the-shelf [PJRC Teensy 3.2](https://www.pjrc.com/store/teensy32.html) is used as the onboard processor. The Teensy is then plugged into a socket on our custom carrier board that adds an SD card slot (max 32 GB), an additional 16 MB of flash memory, an [LM48310](https://www.digikey.com/product-detail/en/texas-instruments/LM48310SD-NOPB/LM48310SD-NOPBCT-ND/1765468) 2.6 watt audio amplifier, and headers for external connections. 

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
![SanDisk Ultra](http://www.openpanzer.org/images/github/sandiskultra_32gb.jpg)

## Sound Files
The sound card requires basically no configuration when paired with the TCB, other than adding your desired sounds to the micro SD card. The sound card identifies the function of each sound by its file name, so you must name your files exactly as shown in the table below. Note we are limited to the 8n3 format, meaning file names cannot exceed 8 characters. Every sound is not required, if any are omitted the card will simply ignore the sound for that function.

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
			<td>Machine Gun Hit</td>
			<td>mghit.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td>Cannon Fire</td>
			<td>cannonf.wav</td>
			<td></td>
			<td></td>
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
			<td>Light Switch</td>
			<td>light.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td>Confirmation Beep</td>
			<td>beep.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td valign="top">Squeaks</td>
			<td valign="top">squeak1.wav<br/> squeak2.wav<br/> squeak3.wav<br/> squeak4.wav<br/> squeak5.wav<br/> squeak6.wav</td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td valign="top">User Sounds</td>
			<td valign="top">user1.wav<br/>user2.wav<br/>user3.wav<br/>user4.wav<br/>user5.wav<br/>user6.wav</td>
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
			<td valign="top">If multiple idle sounds specified, a random one will be played each time vehicle returns to idle.</td>
		</tr>
		<tr>
			<td valign="top">Engine Accelerate</td>
			<td valign="top">enaccl1.wav<br/>enaccl2.wav<br/>enaccl3.wav<br/>enaccl4.wav<br/>enaccl5.wav</td>
			<td valign="top">The acceleration sound is played as a transition from idle to moving.
                            If multiple sounds specified, a random one will be played each time vehicle begins moving.</td>
		</tr>
		<tr>
			<td valign="top">Engine Decelerate</td>
			<td valign="top">endecl1.wav<br/>endecl2.wav<br/>endecl3.wav<br/>endecl4.wav<br/>endecl5.wav</td>
			<td valign="top">The deceleration sound is played as a transition from moving
                            to stopped (idle). If multiple sounds specified, a random one will be
                            played each time vehicle stops moving.</td>
		</tr>
		<tr>
			<td valign="top">Vehicle Moving</td>
			<td valign="top">enrun1.wav<br/>enrun2.wav<br/>enrun3.wav<br/>enrun4.wav<br/>enrun5.wav<br/>                       enrun6.wav<br/>enrun7.wav<br/>enrun8.wav<br/>enrun9.wav<br/>enrun10.wav</td>
			<td valign="top">Engine sound when vehicle moving, with enrun1.wav being the slowest speed and higher numbers being faster speed. Not all sounds are required, as many are specified will automatically be evenly distributed across theentire speed range.</td>
		</tr>
	</table>
</html>
