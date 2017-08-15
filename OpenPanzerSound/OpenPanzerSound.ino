/* Open Panzer Sound    Open Panzer sound board based on the PJRC Teensy 3.2
 * Source:              openpanzer.org              
 * Authors:             Luke Middleton
 *                      
 * Copyright 2017 Open Panzer
 *   
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "src/EEPROMex/EEPROMex.h"
#include "src/LedHandler/LedHandler.h"
#include "src/SimpleTimer/SimpleTimer.h"
#include "SoundCard.h"
#include "Version.h"


// DEFINES AND GLOBAL VARIABLES
// ------------------------------------------------------------------------------------------------------------------------------------------------------>

    // Debug
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        boolean DEBUG = true;                                   // Use for testing, set to false for production
    
    // Serial Ports
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        #define USB_BAUD_RATE          115200                   // We use a fixed baud rate for the USB port
        #define COMM_BAUD_RATE_DEFAULT  38400                   // This is the default baud rate for communicating with the TCB. The user can change it via serial command, but this way they always know what the device is initialized to. 
        #define DebugSerial            Serial                   // Which serial port to print debug messages to. On the Teensy, Serial is the USB port
        #define CommSerial            Serial1                   // Which serial port to monitor for command messages from external controller such as the TCB

    // Simple Timer
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        OPS_SimpleTimer timer;

    // Device mode
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        typedef char _INPUT_MODE;                               // We accept two types of input - hobby RC PWM commands, or serial
        #define INPUT_UNKNOWN               0                   // This is the input mode on startup
        #define INPUT_RC                    1           
        #define INPUT_SERIAL                2
        #define INPUT_ERROR                 3           
        _INPUT_MODE InputMode =             INPUT_UNKNOWN;      // Global variable for input mode

    // Serial Comms
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        #define SerialBlinkTimeout_mS       1000                // We will use this length of time with no serial commands to blink an LED
        #define SENTENCE_BYTES              5                   // How many bytes in a valid sentence. Note we use 5 bytes, not the 4 you are used to seeing with the Scout or Sabertooth! 
        struct DataSentence {                                   // Serial commands should have four bytes (plus termination). 
            uint8_t    Address =            0;                  // We use a struct for convenience
            uint8_t    Command =            0;
            uint8_t    Value =              0;
            uint8_t    Modifier =           0;                  // Unlike the Sabertooth and Scout protocols, the sentence for the sound card includes an extra byte we call Modifier. 
            uint8_t    Checksum =           0;
        };
        uint32_t     TimeLastSerial =       0;                  // Time of last received serial data

    // RC defines
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        #define NUM_RC_CHANNELS             5                   // Number of RC channels we can read
        #define PULSE_WIDTH_ABS_MIN         800                 // Absolute minimum pulse width considered valid
        #define PULSE_WIDTH_ABS_MAX         2200                // Absolute maximum pulse width considered valid
        #define PULSE_WIDTH_TYP_MIN         1000                // Typical minimum pulse width
        #define PULSE_WIDTH_TYP_MAX         2000                // Typical maximum pulse width        
        #define PULSE_WIDTH_CENTER          1500                // Stick centered pulse width (motor off)

        #define RC_PULSECOUNT_TO_ACQUIRE    5                   // How many pulses on each channel to read before considering that channel SIGNAL_SYNCHED
        #define RC_TIMEOUT_US               100000UL            // How many micro-seconds without a signal from any channel before we go to SIGNAL_LOST. Note a typical RC pulse would arrive once every 20,000 uS
        #define RC_TIMEOUT_MS               100                 // In milliseconds, so RC_TIMEOUT_US / 1000

        // RC state machine
        typedef char _RC_STATE;
        #define RC_SIGNAL_ACQUIRE           0
        #define RC_SIGNAL_SYNCHED           1
        #define RC_SIGNAL_LOST              2
        _RC_STATE RC_State =                RC_SIGNAL_ACQUIRE;
        _RC_STATE Last_RC_State =           RC_SIGNAL_ACQUIRE;        

        struct _rc_channel {
            uint8_t pin;                                        // Pin number of channel
            _RC_STATE state;                                    // State of this individual channel (acquiring, synched, lost)
            uint16_t pulseWidth;                                // Actual pulse-width in uS, typically in the range of 1000-2000
            uint8_t value;                                      // Can be used to carry a mapped version of pulseWidth to some other meaningful value
            uint8_t switchPos;                                  // If converted to a multi-position switch, what "position" is the RC switch presently in
            uint32_t lastEdgeTime;                              // Timing variable for measuring pulse width
            uint32_t lastGoodPulseTime;                         // Time last signal was received for this channel
            uint8_t acquireCount;                               // How many pulses have been acquired during acquire state
        }; 
        _rc_channel RC_Channel[NUM_RC_CHANNELS];
        
        #define TEST_CHANNEL 0                                  // Channel 1
        boolean TestRoutine                 = false;            // If RC input 1 is jumpered to ground on startup, the sound card will run a test routine
        boolean CancelTestRoutine           = false;            // If we get a rising edge on the channel we will cancel the test routine

        #define RC_MULTISWITCH_POSITIONS  3
        const int16_t MultiSwitch_MatchArray[RC_MULTISWITCH_POSITIONS] = {
            1500,  // Center - off position        
            1000,  // 1
            2000   // 2
        };

/*      // Here's an example of 13 positions (6 either side of center)
        // This is similar to the Benedini "12-key coder", but most radios would find it difficult to create this many positions
        // without custom hardware. 
        #define RC_MULTISWITCH_POSITIONS  13
        const int16_t MultiSwitch_MatchArray[RC_MULTISWITCH_POSITIONS] = {
            1500,  // Center - off position        
            1000,  // 1
            1083,  // 2
            1165,  // 3
            1248,  // 4
            1331,  // 5
            1413,  // 6
            1587,  // 7
            1669,  // 8
            1752,  // 9
            1835,  // 10
            1917,  // 11
            2000   // 12
        };        
*/

    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
    // Sound Files
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        struct _soundfile{                                       
            char fileName[13];                                  // Filename can have up to 8 characters (no spaces), plus a 3 character extension, plus the dot makes 12 - and we need 1 more for null terminal character
            boolean exists;                                     // This flag will get set to true if we find this file on the SD card
            uint32_t length;                                    // File length in milliseconds, uint32_t from library source
            uint8_t priority;                                   // Higher numbers equal higher priority
        };
        
        // Sometimes we want to know if a sound is playing, but in the first few milliseconds after it starts the library will return false even though it has begun. 
        #define TIME_TO_PLAY            4                       // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying(). We set this to 4mS to be safe. 

        // General Sound Effects
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
        #define NUM_SOUND_FX           27
        #define SND_TURRET_START        0
        #define SND_TURRET              1
        #define SND_TURRET_STOP         2
        #define SND_BARREL_START        3
        #define SND_BARREL              4        
        #define SND_BARREL_STOP         5
        #define SND_MG_START            6
        #define SND_MG                  7
        #define SND_MG_STOP             8
        #define SND_MG2_START           9
        #define SND_MG2                10
        #define SND_MG2_STOP           11        
        #define SND_HIT_CANNON         12
        #define SND_HIT_MG             13
        #define SND_HIT_DESTROY        14
        #define SND_LIGHT_SWITCH       15
        #define SND_REPAIR             16
        #define SND_BEEP               17
        #define SND_BRAKE              18
        #define SND_TRANS_ENGAGE       19
        #define SND_TRANS_DISENGAGE    20
        //--------------------------------
        #define SND_SQUEAK_OFFSET      21   // The position in the array where squeaks begin
        #define NUM_SQUEAKS             6   // Number of squeaks
        //-------------------------------
        _soundfile Effect[NUM_SOUND_FX] = {
            {"tr_start.wav",false, 0, 1},   // Turret start (optional)
            {"turret.wav",  false, 0, 1},   // Turret rotation
            {"tr_stop.wav", false, 0, 1},   // Turret stop  (optional)
            {"br_start.wav",false, 0, 1},   // Barrel start (optional)
            {"barrel.wav",  false, 0, 1},   // Barrel elevation
            {"br_stop.wav", false, 0, 1},   // Barrel stop  (optional
            {"mg_start.wav",false, 0, 3},   
            {"mg.wav"      ,false, 0, 3},   // As a repeating sound we give machine gun higher priority so things like squeaks don't interrupt it
            {"mg_stop.wav", false, 0, 3},
            {"mg2start.wav",false, 0, 3},   
            {"mg2.wav"     ,false, 0, 3},   // As a repeating sound we give machine gun higher priority so things like squeaks don't interrupt it
            {"mg2stop.wav", false, 0, 3},            
            {"cannonh.wav", false, 0, 1},
            {"mghit.wav",   false, 0, 1},
            {"destroy.wav", false, 0, 1},
            {"light.wav",   false, 0, 1},
            {"repair.wav",  false, 0, 1},
            {"beep.wav",    false, 0, 9},   // We give beep a very high priority
            {"brake.wav",   false, 0, 1},
            {"txengage.wav",false, 0, 1},   // Transmission engage sound
            {"txdegage.wav",false, 0, 1},   // Transmission dis-engage sound
            {"squeak1.wav", false, 0, 1},
            {"squeak2.wav", false, 0, 1},
            {"squeak3.wav", false, 0, 1},
            {"squeak4.wav", false, 0, 1},
            {"squeak5.wav", false, 0, 1},
            {"squeak6.wav", false, 0, 1}
        };

       
        // Cannon fire sounds - permit multiple
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
//        #define SND_FIRE_CANNON         6
        #define NUM_SOUNDS_CANNON       5
        _soundfile CannonFireSound[NUM_SOUNDS_CANNON] = {
            {"cannonf.wav", false, 0, 1},
            {"cannonf2.wav", false, 0, 1},
            {"cannonf3.wav", false, 0, 1},
            {"cannonf4.wav", false, 0, 1},
            {"cannonf5.wav", false, 0, 1}
        };

       
        // Squeaks
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
        boolean AllSqueaks_Active = false;                      // Indicates whether squeaking generally is going on or not, but doesn't indicate the currently active status of any particular squeak
        typedef void(*void_FunctionPointer)(void);              // We will use an array of function pointers to the squeak functions for ease of coding in some places. These are assigned in InitializeSounds()
        void_FunctionPointer CallSqueakFunction[NUM_SQUEAKS];   // An array of function pointers we will use for squeaks 
        struct _squeak_info{                                    // Information about each squeak. We could have also put the function pointers inside here but it complicates our EEPROM copy of this struct. 
            uint16_t intervalMin;
            uint16_t intervalMax;
            boolean enabled;                                    // Has the user enabled this squeak generally
            boolean active;                                     // Active indicates if squeaking is currently ongoing (vehicle moving or other conditions met), or not (ie vehicle stopped)
            uint32_t lastSqueak;                                // Time when this squeak last squeaked
            uint16_t squeakAfter;                               // Amount of time after which the squeak should squeak again
        };
        int SqueakTimerID = 0;
        boolean AnySqueakEnabled = 0;                           // Are any squeaks even enabled? If not, we can skip some messages

        // User Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->        
        #define NUM_USER_SOUNDS        6                        // If you change this you must also update the initialize of the active flags to false in PlayUserSound() on the Sounds tab
        _soundfile UserSound[NUM_USER_SOUNDS] = {
            {"user1.wav",   false, 0, 1},
            {"user2.wav",   false, 0, 1},
            {"user3.wav",   false, 0, 1},
            {"user4.wav",   false, 0, 1},
            {"user5.wav",   false, 0, 1},
            {"user6.wav",   false, 0, 1}
        };       

        // Engine - Idle Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
        #define NUM_SOUNDS_IDLE         5
        _soundfile IdleSound[NUM_SOUNDS_IDLE] = {
            {"enidle1.wav", false, 0, 1},
            {"enidle2.wav", false, 0, 1},
            {"enidle3.wav", false, 0, 1},
            {"enidle4.wav", false, 0, 1},
            {"enidle5.wav", false, 0, 1}
        };
        
        // Engine - Acceleration Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
        #define NUM_SOUNDS_ACCEL        5
        _soundfile AccelSound[NUM_SOUNDS_ACCEL] = {
            {"enaccl1.wav", false, 0, 1},
            {"enaccl2.wav", false, 0, 1},
            {"enaccl3.wav", false, 0, 1},
            {"enaccl4.wav", false, 0, 1},
            {"enaccl5.wav", false, 0, 1}
        };

        // Engine - Deceleration Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->
        #define NUM_SOUNDS_DECEL        5
        _soundfile DecelSound[NUM_SOUNDS_DECEL] = {
            {"endecl1.wav", false, 0, 1},
            {"endecl2.wav", false, 0, 1},
            {"endecl3.wav", false, 0, 1},
            {"endecl4.wav", false, 0, 1},
            {"endecl5.wav", false, 0, 1}            
        };

        // Engine - Running Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->        
        uint8_t EngineSpeed = 0;                                        // Global variable for engine speed
        uint8_t NumRunSounds = 0;                                       // This will later be set to the number of detected run sounds (can be less than NUM_SOUNDS_RUN)
        #define NUM_SOUNDS_RUN           10
        _soundfile RunSound[NUM_SOUNDS_RUN] = {                         // An array of all possible run sounds
            {"enrun1.wav", false, 0, 1},
            {"enrun2.wav", false, 0, 1},
            {"enrun3.wav", false, 0, 1},
            {"enrun4.wav", false, 0, 1},
            {"enrun5.wav", false, 0, 1},
            {"enrun6.wav", false, 0, 1},
            {"enrun7.wav", false, 0, 1},
            {"enrun8.wav", false, 0, 1},
            {"enrun9.wav", false, 0, 1},
            {"enrun10.wav", false, 0, 1}
        };
        _soundfile EngineColdStart = {"enstart1.wav", false, 0, 1};     // Will use this start sound first, and continue to use it if enstart2 doesn't exists
        _soundfile EngineHotStart  = {"enstart2.wav", false, 0, 1};     // Start sound on 2nd and subsequent starts if exists, otherwise will use enstart1
        _soundfile EngineShutoff   = {"enstop.wav",   false, 0, 1};
        #define ENGINE_FADE_TIME_MS  400                                // Time in milliseconds to cross-fade between engine sounds. 

        // Track Overlay Sounds
        // ---------------------------------------------------------------------------------------------------------------------------------------------->        
        uint8_t VehicleSpeed = 0;                                       // Global variable for vehicle speed
        boolean VehicleMoving = false;                                  // Are we moving or not
        uint8_t NumOverlaySounds = 0;                                   // This will later be set to the number of detected track overlay sounds (can be less than NUM_SOUNDS_TRACK_OVERLAY)
        #define NUM_SOUNDS_TRACK_OVERLAY    10
        _soundfile TrackOverlaySound[NUM_SOUNDS_TRACK_OVERLAY] = {      // An array of all possible track overlay sounds
            {"tracks1.wav", false, 0, 1},
            {"tracks2.wav", false, 0, 1},
            {"tracks3.wav", false, 0, 1},
            {"tracks4.wav", false, 0, 1},
            {"tracks5.wav", false, 0, 1},
            {"tracks6.wav", false, 0, 1},
            {"tracks7.wav", false, 0, 1},
            {"tracks8.wav", false, 0, 1},
            {"tracks9.wav", false, 0, 1},
            {"tracks10.wav", false, 0, 1}
        };
        _soundfile TrackOverlayStart = {"trkstart.wav", false, 0, 2};   // If present, this sound will play before tracks1
        _soundfile TrackOverlayStop = {"trkstop.wav", false, 0, 2};     // If present, this sound will play when the vehicle comes to a stop
        #define TRACK_OVERLAY_FADE_TIME_MS  400                         // Time in milliseconds to cross-fade between track overlay sounds

    // Total Number of Sounds
    // ---------------------------------------------------------------------------------------------------------------------------------------------->        
        const uint8_t COUNT_TOTAL_SOUNDFILES = NUM_SOUND_FX + NUM_USER_SOUNDS + NUM_SOUNDS_TRACK_OVERLAY + NUM_SOUNDS_IDLE + NUM_SOUNDS_ACCEL + NUM_SOUNDS_DECEL + NUM_SOUNDS_RUN + 5; // Plus 5 - 3 for engine cold start, hot start, and shudown, and 2 for track overlay start/stop
        _soundfile *allSoundFiles[COUNT_TOTAL_SOUNDFILES];
        

    // Audio
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        typedef char _sound_special_case;       // Some sound effects might involve multiple sounds played in order, such as turret rotation. 
        #define FX_SC_NONE          0           // FX_SC = Sound Effect Special Case
        #define FX_SC_TURRET        1           // Special case turret - if this flag is set, we need to play the repeating portion of the turret rotation sound after having played a special turret start sound
        #define FX_SC_BARREL        2           // Special case barrel - if this flag is set, we need to play the repeating portion of the barrel elevation sound after having played a special barrel start sound
        #define FX_SC_MG            3           // Special case MG - if this flag is set, we need the play the repeating portion of the machine gun sound after having played a special machine gun start sound
        #define FX_SC_MG2           4           // Special case second MG 

        typedef char _engine_state;             // These are engine states
        #define ES_OFF              0
        #define ES_START            1
        #define ES_IDLE             2
        #define ES_ACCEL            3
        #define ES_ACCEL_WAIT       4
        #define ES_RUN              5
        #define ES_START_IDLE       6           // When we want to go directly to idle rather than through decel first
        #define ES_DECEL            7
        #define ES_SHUTDOWN         8           
        _engine_state EngineState = ES_OFF;     // Create a global variable with the current state
        _engine_state EngineState_Prior = ES_OFF;   // This will let us keep track of where we were
        boolean EngineRunning = false;          // Global engine running flag

        typedef char _engine_transition;        // These are the equivalent of the effect special cases above - we use these flags to transition from one engine sound to another
        #define EN_TR_NONE          0           // 
        #define EN_TR_IDLE          1           // Transition to idle from startup or decel 
        #define EN_TR_ACCEL_RUN     2           // Transition to run from acceleration sound 
        #define EN_TR_RUN           3           // Engine running is a sort of continual transition, eiher to a new speed or else repeating the existing speed.
        #define EN_TR_TURN_OFF      4           // Engine is now completely off (shutdown sound has completed)

        typedef char _track_transition;         // As above but for track overlay transitions
        #define TO_TR_NONE          0           // 
        #define TO_TR_START_MOVING  1           // Transition from the start sound to moving sound
        #define TO_TR_KEEP_MOVING   2           // Vehicle moving is a sort of continual transition, eiher to a new speed or else repeating the existing speed.
        #define TO_TR_STOP_MOVING   3           // Stop sound has finished playing, now shut down the overlay stuff
        
        // We need several pieces of information to go along with our sound effects 
        struct _sound_id{                                                    
            uint16_t Num;                                                   // This sub-struct stores the ID of the currently-playing sound (unique number that increments as time goes on)
            uint8_t Slot;                                                   // as well as the Slot number, which identifies the position in our FX array
        };
        struct _sound{                                                      // For each sound effect we keep track of several items:
            AudioPlaySdWav SDWav;                                           // A WAV file SD card player audio object
            _soundfile soundFile;                                           // The currently-playing soundfile (this is a struct that holds the file name, whether it exists, its length, priority, etc. 
            boolean isActive;                                               // If active it indicates there is a file playing, otherwise sound effect slot is free to be used for something else
            boolean repeat;                                                 // If true, the sound will be re-started when it is done playing. 
            uint8_t repeatTimes;                                            // If zero, the sound will be repeated indefinitely. If >0 the sound will be repeated by the specified number of times.
            uint8_t timesRepeated;                                          // How many times has this sound been repeated
            uint32_t timeStarted;                                           // Timestamp when the file last began playing
            uint32_t timeWillFadeOut;                                       // If we are fading out this sound, at what time will the sound equal zero - use this to stop the sound at that point, rather than having it play static
            boolean fadingOut;                                              // Is this sound in the process of being faded-out
            _sound_id ID;                                                   // See the struct above, this includes a unique ID for each instance of a played sound
            _sound_special_case specialCase;
        };
        #define NUM_FX_SLOTS     4                                          // We reserve 4 slots for the sound effects.
        _sound                   FX[NUM_FX_SLOTS];                          //     sounds from the SD card, which is probably about the limit with a good SD card (SanDisk Ultra)
        uint8_t AvailableFXSlots = NUM_FX_SLOTS;                            // How many FX slots are available for sound effects - if we are implementing track overlay, this number will have 2 subtracted from it
        #define NUM_ENGINE_SLOTS 2
        _sound                   Engine[NUM_ENGINE_SLOTS];                  // Engine WAV files (two for mixing as we fade from one to the next)
        boolean                  EngineEnabled = false;                     // The engine will be enabled only if a minimum number of sound files have been found to exist on the SD card
        uint8_t                  EnNext = 0;                                // A flag to indicate which of the two engine slots we should use for the next sound to be queued.
        uint8_t                  EnCurrent = 0;                             // Which of the two engine slots is the active one now (both can be playing at once but one will be fading out and the other will be fading in, we set the latter to Current)
        #define NUM_TRACK_OVERLAY_SLOTS 2
        #define FIRST_OVERLAY_SLOT (NUM_FX_SLOTS - NUM_TRACK_OVERLAY_SLOTS) 
        boolean                  TrackOverlayEnabled = false;               // The track overlay sounds will be enabled only if certain sound files have been found to exist on the SD card
        uint8_t                  TONext = FIRST_OVERLAY_SLOT;               // A flag to indicate which of the two FX slots we should use for the next track overlay sound to be queued, if enabled
        uint8_t                  TOCurrent = FIRST_OVERLAY_SLOT;            // Which of the two FX slots is the active one now for purpose of track overlay (both can be playing at once but one will be fading out and the other will be fading in, we set the latter to Current)
        AudioPlaySerialflashRaw  FlashRaw;                                  // We also reserve a slot for sounds stored on serial flash in RAW format. This will not count to the SD card limit. 
        AudioEffectFade          EngineFader[NUM_ENGINE_SLOTS];             // Fader for Engine sounds
        AudioEffectFade          OverlayFader[NUM_TRACK_OVERLAY_SLOTS];     // Fader for track overlay
        AudioMixer4              Mixer1;                                    // Mixer
        AudioMixer4              Mixer2;                                    // Mixer
        AudioMixer4              Mixer3;                                    // Mixer
        AudioMixer4              MixerFinal;                                // Mixer
        AudioOutputAnalog        DAC;                                       // Output to DAC
      // Connections
        // Engines into faders
        AudioConnection          patchCord1(Engine[0].SDWav, 0, EngineFader[0], 0); // Feed both engine sources into faders, so we can cross-fade between them
        AudioConnection          patchCord2(Engine[0].SDWav, 1, EngineFader[0], 0);
        AudioConnection          patchCord3(Engine[1].SDWav, 0, EngineFader[1], 0);
        AudioConnection          patchCord4(Engine[1].SDWav, 1, EngineFader[1], 0);
        // Engine faders into Mixer 1
        AudioConnection          patchCord5(EngineFader[0],  0, Mixer1, 0); // Fade 0 (from Engine sound 0) into Mixer 1 input 0
        AudioConnection          patchCord6(EngineFader[1],  0, Mixer1, 1); // Fade 1 (from Engine sound 1) into Mixer 1 input 1
        // FX[0], FX[1] into Mixer 2
        AudioConnection          patchCord7(FX[0].SDWav, 0, Mixer2, 0);     // FX1 into Mixer 2 input 0 & 1
        AudioConnection          patchCord8(FX[0].SDWav, 1, Mixer2, 1);     // 
        AudioConnection          patchCord9(FX[1].SDWav, 0, Mixer2, 2);    // FX2 into Mixer 2 input 2 & 3
        AudioConnection          patchCord10(FX[1].SDWav, 1, Mixer2, 3);    // 
        // FX[2], FX[3] into faders
        AudioConnection          patchCord11(FX[2].SDWav, 0, OverlayFader[0], 0); // Feed both track overlay FX sources into faders, so we can cross-fade between them
        AudioConnection          patchCord12(FX[2].SDWav, 1, OverlayFader[0], 0);
        AudioConnection          patchCord13(FX[3].SDWav, 0, OverlayFader[1], 0);
        AudioConnection          patchCord14(FX[3].SDWav, 1, OverlayFader[1], 0);
        // Track Overlay faders into Mixer 3
        AudioConnection          patchCord15(OverlayFader[0],  0, Mixer3, 0); // Fade 0 (from FX 2) into Mixer 3 input 0
        AudioConnection          patchCord16(OverlayFader[1],  0, Mixer3, 1); // Fade 1 (from FX 3) into Mixer 3 input 1
        // Mixer1, Mixer2, and Mixer3 into MixerFinal
        AudioConnection          patchCord17(Mixer1, 0, MixerFinal, 0);     // Mixer 1 into MixerFinal input 0
        AudioConnection          patchCord18(Mixer2, 0, MixerFinal, 1);     // Mixer 2 into MixerFinal input 1
        AudioConnection          patchCord19(Mixer3, 0, MixerFinal, 2);     // Mixer 3 into MixerFinal input 2 (3 unused)
        // Flash Raw into MixerFinal
        AudioConnection          patchCord20(FlashRaw, 0, MixerFinal, 3);   // Flash memory RAW file into Mixer 1 input 2 (RAW are always mono)
        // Mixer out to DAC
        AudioConnection          patchCord21(MixerFinal, DAC);       

    // Volume
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        float Volume =                  0.0;                                // Global variable for volume. Can be modified by a physical knob or through external source (eg serial)
        typedef char VOLUME_CATEGORY;
        #define VC_ENGINE                 0                                 // The four types of volume that can be adjusted relative to each other on the Open Panzer sound card
        #define VC_EFFECTS                1
        #define VC_TRACK_OVERLAY          2
        #define VC_FLASH                  3
        float fVols[4] = {1.0, 1.0, 1.0, 0.0};                              // Relative volumes for different categories. Default to equal, but will ultimately be set by user through OP Config 
        typedef char _volume_source;
        _volume_source  vsKnob =        0;                                  // Volume Source (vs) control - knob
        _volume_source  vsSerial =      1;                                  // Volume Source (vs) control - serial
        _volume_source  vsRC =          2;                                  // Volume Source (vs) control - RC
        _volume_source volumeSource =   vsKnob;                             // What is the current control source for volume?


    // EEPROM
    // -------------------------------------------------------------------------------------------------------------------------------------------------->       
        #define EEPROM_INIT             0xABCD
        #define EEPROM_START_ADDRESS    0
        struct _eeprom_data {                                  
            _squeak_info squeakInfo[NUM_SQUEAKS];                           // Squeak settings
            uint32_t InitStamp;                                             // Marker used to indicate if EEPROM has ever been initalized
        };
        _eeprom_data ramcopy;                                               // Put a copy of this struct in RAM. Another identical copy will be created in EEPROM.


// PINS                       
// ------------------------------------------------------------------------------------------------------------------------------------------------------>
    // SPI 
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                        
        // We use 11, 12, and 13 for MOSI, MISO, SCK respectively, which is the default for Arduino anyway. 
        const byte SPI_MOSI =          11;                                  
        const byte SPI_MISO =          12;
        const byte SPI_SCK =           13;

    // SD card
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        const byte SD_CS =            14;                                   // SD card chip select pin

    // SPI Flash
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        const byte FLASH_CS =          6;                                   // Select for the SPI flash chip

    // LM48310 amplifier and volume control
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                    
        const byte Amp_Enable =        5;                                   // Active low shutdown for amplifier. Set to high to enable. 
        const byte Volume_Knob =      23;                                   // Physical volume knob control. User can also control volume via serial.

    // RC Inputs
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                    
        const byte RC_1 =             19;
        const byte RC_2 =             18;
        const byte RC_3 =             17;
        const byte RC_4 =             16;
        const byte RC_5 =             15;                                

    // LEDs
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        const byte pin_BlueLED =      20;                                   // Blue LED - used to indicate status
        const byte pin_RedLED =       21;                                   // Red LED - used to indicate errors
        OPS_LedHandler            RedLed;
        OPS_LedHandler           BlueLed;

    // DEBUGGING STUFF
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        elapsedMillis PrintMemUsage;



void setup() 
{

    // Serial
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        Serial.begin(USB_BAUD_RATE);                            // USB serial port
        DebugSerial.begin(USB_BAUD_RATE);                       // USB serial port
        CommSerial.begin(COMM_BAUD_RATE_DEFAULT);               // Comm port to the TCB

    
    // LOAD VALUES FROM EEPROM    
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        InitializeEEPROM();                                                     // If EEPROM has never been used before (InitStamp in EEPROM does not equal EEPROM_INIT defined in sketch), initialize all values to default        
        // boolean did_we_init = InitializeEEPROM();                            // Use for testing
        // if (did_we_init) { DebugSerial.println(F("EEPROM Initalized")); }    

        
    // Audio Objects
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        // With Teensy 3.2, the Teensy Audio Library can be used to create sounds. Nearly all the audio library examples are designed for the Audio Shield. To adapt them for the Prop Shield, 
        // replace "i2s1" with "dac1" in the Design Tool, or AudioOutputI2S with AudioOutputAnalog in the Arduino code. Then delete the SGTL5000 object and any code which uses it.
        // Of course, we have gotten a little more complicated than that... but the general suggestion is correct. 
        AudioMemory(12);                // Allocate the memory for all audio connections. The numberBlocks input specifies how much memory to reserve for audio data. Each block holds 128 audio samples, or approx 2.9 ms of sound. 
                                        // Usually an initial guess is made for numberBlocks and the actual usage is checked with AudioMemoryUsageMax(). See https://www.pjrc.com/teensy/td_libs_AudioConnection.html
                                        // In testing with 2 engine slots and 4 FX slots we have not exceeded 8 slots. Set it somewhat higher to be safe. 
        SetVolume();

    
    // RC Inputs
    // -------------------------------------------------------------------------------------------------------------------------------------------------->        
        // Assign pins to RC inputs
        RC_Channel[0].pin = RC_1;       // Engine on/off
        RC_Channel[1].pin = RC_2;       // Engine speed
        RC_Channel[2].pin = RC_3;       // 3-position sound switch
        RC_Channel[3].pin = RC_4;       // 3-position sound switch
        RC_Channel[4].pin = RC_5;       // Volume control
        InitializeRCChannels();         // Initialize RC channels
        EnableRCInterrupts();           // Start checking the RC pins for a signal
        
        
    // LEDs and Other Pins
    // -------------------------------------------------------------------------------------------------------------------------------------------------->        
        // LEDs
        RedLed.begin(pin_RedLED, false);                        
        BlueLed.begin(pin_BlueLED, false);

        // Volume knob
        pinMode(Volume_Knob, INPUT_PULLUP);


    // SPI - Used for both SD card and Flash memory access
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        SPI.setMOSI(SPI_MOSI);    
        SPI.setMISO(SPI_MISO);
        SPI.setSCK(SPI_SCK);

    // SD Card 
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        // Standard Arduino SD Library:                      https://www.arduino.cc/en/Reference/SD
        // BUT WE ARE USING THE TEENSY OPTIMIZED SD LIBRARY! 
        // See the comments here: 
        // C:\Arduino\hardware\teensy\avr\libraries\SD\SD_t3.h      (Replace C:\Arduino\ with your Arduino install directory - assumes also you have Teensyduino installed)
        // And uncomment the line at the top of that file: 
        //  #define USE_TEENSY3_OPTIMIZED_CODE
        // We use the "optimized" version of the SD library designed for the Teensy 3.x which replaces entirely the Arduino SD library. 
        // It is much faster for reading more than 1 file at a time (which we need). It comes at the cost that we can no longer write to the SD card, which we do not need. 
        // Also from: https://www.pjrc.com/store/teensy3_audio.html
        // PJRC recommends SanDisk Ultra for projects where multiple WAV files will be played at the same time. SanDisk Ultra is more expensive, but its non-sequential speed is much faster.
        // The Arduino SD library supports up to 32 GB size. Do not use 64 & 128 GB cards.
        // The audio library includes a simple benchmark to test SD cards. Open it from File > Examples > Audio > HardwareTesting > SdCardTest
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);      // De-select the SD Card at the beginning
        if (!(SD.begin(SD_CS)))         // The pin number passed is chip select
        {
            RedLed.startBlinking(40,40);    // This is a very fast blink we reserve for indicating SD card errors
            elapsedMillis wait; 
            while (1) 
            {
                RedLed.update();
                if (wait > 1000)
                {
                    Serial.println("Unable to access the SD card");
                    wait = 0;
                }
            }
        }
        

    // Initialize Files
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        // Do this before turning on the amp to avoid an obnoxious sound
        delay(500);                                 // Get ready
        InitializeSounds();                         // Discover which sound files are present on the card    
        if (DEBUG) PrintVersion();
        if (DEBUG) DumpSoundFileInfo();             // Dump all discovered file information to the USB port
        DetermineEnginePresent();                   // Set the EngineEnabled global variable
        DetermineTrackOverlayPresent();             // Set the TrackOverlayEnabled global variable


    // Initialize Static Mixers
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
        // Mixers 1, 2, and 3 are fixed with equal gain for all channels. We manipulate gain on MixerFinal to adjust volume. 
        Mixer1.gain(0,1);
        Mixer1.gain(1,1);
        Mixer1.gain(2,1);
        Mixer1.gain(3,1);
        Mixer2.gain(0,1);
        Mixer2.gain(1,1);
        Mixer2.gain(2,1);
        Mixer2.gain(3,1);
        Mixer3.gain(0,1);
        Mixer3.gain(1,1);
        Mixer3.gain(2,1);
        Mixer3.gain(3,1);        
        
    
    // Amplifier
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        // By default, the audio library will create a 1.2Vp-p signal at the DAC pin, which is a comfortable volume level for listening in a quiet room, but not nearly the amplifier's full power. 
        // To get louder output, set the dac1 object to use EXTERNAL reference. Doing this before powering up the amp will avoid a loud click.
        DAC.analogReference(EXTERNAL);  // much louder!
        delay(50);                      // time for DAC voltage stable
        // Now power up the amp
        pinMode(Amp_Enable, OUTPUT);    // Set the amplifier pin to output
        digitalWrite(Amp_Enable, HIGH); // Turn on the amplifier
        delay(10);                      // Allow time to wake up  

        
    // Random Seed
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
        randomSeed(analogRead(A0));     // Initialize the random number generator by reading an unused (floating) analog input pin

    // Start LEDs
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
        RedLed.blinkHeartBeat();

    // Test Routine
    // -------------------------------------------------------------------------------------------------------------------------------------------------->    
        DetectJumper();                 // This will check if a jumper has been attached to the specified RC input (signal held to ground). If it has, we will 
                                        // run through all sounds on the SD card
}


void loop()
{
static boolean testRoutineStarted = false; 

    // Determine the input mode for this session, and once discovered, poll it routinely
    switch (InputMode)
    {
        case INPUT_UNKNOWN:
            // In this case we have yet to identify a serial or RC command, so check both
            CheckSerial();                                      // Serial requires polling
            if (RC_State == RC_SIGNAL_SYNCHED)
            {
                InputMode = INPUT_RC;                           // Set mode to RC
                if (DEBUG) DebugSerial.println(F("Device mode: RC input"));
            }
            else if (TimeLastSerial > 0) 
            {
                InputMode = INPUT_SERIAL;                       // Set mode to Serial
                DisableRCInterrupts();                          // Don't check the RC inputs anymore
                if (DEBUG) DebugSerial.println(F("Device mode: Serial input"));
            }
            
            // While in unknown mode the Red LED is blinking slowly as started in Setup(). Once we exit unknown mode, turn it off. 
            // We are only in unknown mode until the first signal is received - after that we will never return to unknown mode until the device is re-started. 
            if (InputMode != INPUT_UNKNOWN) 
            {
                RedLed.stopBlinking();
                BlueLed.on();                                   // Blue LED comes on solid when signal present
            }
            break;
                        
        case INPUT_SERIAL:
            if (!TestRoutine) CheckSerial();                    // Poll the serial port, but only if we are not within the test routine
            if (!BlueLed.isBlinking() && (millis() - TimeLastSerial) > SerialBlinkTimeout_mS)
            {
                BlueLed.blinkHeartBeat();                       // Blink the status LED slowly if we haven't received data in a while
            }
            break;

        
        case INPUT_RC:
            // RC pretty much takes care of itself through the ISRs
            // The RC pin change ISRs will try to determine the status of each channel, but of course if a channel becomes disconnected its ISR won't even trigger. 
            // So we also force a check from the main loop
            UpdateRCStatus();
            break;
    }


    // Per loop updates that have to happen regardless of input mode. 
        timer.run();                            // SimpleTimer object, used for various timing tasks. Must be polled. 
        RedLed.update();                        // Led handlers must be polled
        BlueLed.update();                       // Led handlers must be polled
        SetVolume();                            // Volume can be set by a physical knob wired to the board, or via Serial or RC input. 

    // There are other updates that also need to happen, but it depends on whether we are in the test routine or not
        if (TestRoutine)                        // We are in the test routine, main loop works a litle different
        {
            if (!testRoutineStarted)
            {   // Start test routine
                testRoutineStarted = true; 
                RedLed.stopBlinking();
                BlueLed.stopBlinking();
                StopAllSounds();
                PrintStartTestRoutine();
            }

            RunTestRoutine();                   // During the test routine keep calling this function
            
        }
        else                                    // Not in test routine, proceed normally
        {
            if (testRoutineStarted)
            {
                // Here we are first leaving the test routine
                testRoutineStarted = false;     // Reset so we only come here once
                FX[0].SDWav.stop();             // Stop playing
                PrintEndTestRoutine();          // We're done
                if (InputMode == INPUT_UNKNOWN) RedLed.blinkHeartBeat();    // Restore the waiting
            }

            // Normal operation updates
            UpdateEngine();                     // Updates the engine status
            UpdateIndividualEngineSounds();     // Takes care of repeating/transitioning engine sounds
            UpdateVehicle();                    // Updates the vehicle moving status
            UpdateIndividualOverlaySounds();    // Takes care of repeating/transitioning track overlay sounds
            UpdateEffects();                    // Plays sound effect files
            UpdateSqueaks();                    // Play squeaks at appropriate intervals
        }


    
    // We used this to figure out how many memory slots to allocate. 
    /*
    if (DEBUG && PrintMemUsage > 1000)
    {
        Serial.print(F("Max mem used: ")); 
        Serial.println(AudioMemoryUsageMax());
        PrintMemUsage = 0;
    }
    */
}

void DetermineEnginePresent(void)
{
    // Decide if we should enable engine sound functionality
    // We need at least one start sound, one idle sound, and one run sound
    if ((EngineColdStart.exists || EngineHotStart.exists) && IdleSound[0].exists && RunSound[0].exists) 
    {
        EngineEnabled = true;
    }     
    else 
    {
        EngineEnabled = false;
        if (DEBUG)
        {
            DebugSerial.println();
            DebugSerial.print(F("Engine sounds disabled - files missing"));
            DebugSerial.println();      
        }
    }    
}

void DetermineTrackOverlayPresent(void)
{
    // Decide if we should enable track overlay sound functionality. 
    // If so, we reserve two FX slots to these sounds exclusively. 
    // We need at least one track overlay sound
    if (TrackOverlaySound[0].exists) 
    {
        TrackOverlayEnabled = true;
        if (DEBUG) DebugSerial.println(F("Track overlay sounds enabled"));
    }     
    else 
    {
        TrackOverlayEnabled = false;
        if (DEBUG)
        {
            DebugSerial.println();
            DebugSerial.print(F("Track Overlay sounds disabled - no files found"));
            DebugSerial.println();      
        }
    }        
}





