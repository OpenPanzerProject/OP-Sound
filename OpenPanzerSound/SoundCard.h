/* OP_SoundCard.h       Defines for use in communicating with Open Panzer sound cards
 *                      This should match the defines in OP_Sound.h used in the TCB project. 
 *
 */ 


#ifndef OP_SOUNDCARD_H
#define OP_SOUNDCARD_H

// Open Panzer Sound Card address byte
#define OPSC_ADDRESS                         0xDA   // 218  Unique identifier for Open Panzer sound cards

// Commands                                         
#define OPSC_CMD_SERIAL_WATCHDOG             0x0E   // 14   Same as used for Scout - may not be implemented on sound card, probably don't need
#define OPSC_CMD_BAUD_RATE                   0x0F   // 15   Same as used for Scout - "   "

#define OPSC_CMD_ENGINE_START                0x2C   // 44   Other Sound Card commands begin at 44
#define OPSC_CMD_ENGINE_STOP                 0x2D   // 45   
#define OPSC_CMD_ENGINE_SET_SPEED            0x2E   // 46
#define OPSC_CMD_ENGINE_SET_IDLE             0x2F   // 47
#define OPSC_CMD_REPAIR_START                0x30   // 48
#define OPSC_CMD_REPAIR_STOP                 0x31   // 49
#define OPSC_CMD_CANNON                      0x32   // 50
#define OPSC_CMD_CANNON_HIT                  0x33   // 51
#define OPSC_CMD_TANK_DESTROYED              0x34   // 52
#define OPSC_CMD_MG_START                    0x35   // 53
#define OPSC_CMD_MG_STOP                     0x36   // 54
#define OPSC_CMD_MG_HIT                      0x37   // 55
#define OPSC_CMD_TURRET_START                0x38   // 56
#define OPSC_CMD_TURRET_STOP                 0x39   // 57
#define OPSC_CMD_BARREL_START                0x3A   // 58
#define OPSC_CMD_BARREL_STOP                 0x3B   // 59
#define OPSC_CMD_HEADLIGHT                   0x3C   // 60
#define OPSC_CMD_USER_SOUND_PLAY             0x3D   // 61   -- Use Modifier field to indicate which number to play.
#define OPSC_CMD_USER_SOUND_REPEAT           0x3E   // 62   -- Use Modifier field to indicate which number to play.
#define OPSC_CMD_USER_SOUND_STOP             0x3F   // 63   -- Use Modifier field to indicate which number to play.
#define OPSC_CMD_SQUEAKS_START               0x40   // 64
#define OPSC_CMD_SQUEAKS_STOP                0x41   // 65
#define OPSC_CMD_SQUEAK_SET_MIN              0x42   // 66   -- Use Modifier field to indicate which squeak to set. Min goes in Value field.
#define OPSC_CMD_SQUEAK_SET_MAX              0x43   // 67   -- Use Modifier field to indicate which squeak to set. Max goes in Value field.
#define OPSC_CMD_SQUEAK_ENABLE               0x44   // 68
#define OPSC_CMD_BEEP_ONCE                   0x45   // 69
#define OPSC_CMD_BEEP_X                      0x46   // 70
#define OPSC_CMD_SET_VOLUME                  0x47   // 71
#define OPSC_CMD_BRAKE_SOUND                 0x48   // 72
#define OPSC_CMD_2NDMG_START                 0x49   // 73
#define OPSC_CMD_2NDMG_STOP                  0x4A   // 74
#define OPSC_CMD_VEHICLE_SET_SPEED           0x4B   // 75
#define OPSC_CMD_SET_RELATIVE_VOLUME         0x4C   // 76  -- Use Modifier to indicate which volume 0=Engine, 1=Track Overlay, 2=Effects, 3=Flash
#define OPSC_CMD_ENGAGE_TRANSMISSION         0X4D   // 77  -- Pass in value: true (1) means engaged, false (0) means disengaged
#define OPSC_CMD_CANNON_READY                0x4E   // 78

// Modifiers
#define OPSC_MAX_NUM_SQUEAKS                  6     // How many squeaks can this device implement
#define OPSC_MAX_NUM_USER_SOUNDS              4     // How many user sounds does this device implement

// Codes
#define OPSC_BAUD_CODE_2400                   1     // Codes for changing baud rates, same numbers as used for Scout
#define OPSC_BAUD_CODE_9600                   2     // These are the same codes used by certain Dimension Engineering Sabertooth controllers
#define OPSC_BAUD_CODE_19200                  3     //
#define OPSC_BAUD_CODE_38400                  4     //
#define OPSC_BAUD_CODE_115200                 5     //
#define OPSC_BAUD_CODE_57600                  6     // The preceding codes are numbered identically to the codes used for Sabertooth controllers, which do not include 57600. 
                                                    // That is why 57600 is number 6 and not number 5. On some boards 57600 doesn't work very well so it is not recommended to use it. 

#endif // OP_SOUNDCARD_H


