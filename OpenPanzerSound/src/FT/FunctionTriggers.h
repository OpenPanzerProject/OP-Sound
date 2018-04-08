#ifndef OP_FUNCTIONTRIGGERS_H
#define OP_FUNCTIONTRIGGERS_H


//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Functions are actions the user may want to control. Functions are initiated by triggers which on the sound card are RC channel inputs.

// Function IDs *below* 1000 are user sounds. 
// User Sound Function IDs are constructed by (user sound number * 10) + (1,2,3) to represent play (1), repeat (2), or stop (3)
// Example: sound  2 repeat = Function ID is 22
// Example: sound 14 play   = Function ID is 141
// Example: sound 22 stop   = Function ID is 223
#define MAX_NUM_USER_SOUNDS 22
#define function_id_usersound_multiplier    10
#define function_id_usersound_min_range     10         // (sound 1  * multiplier)
#define function_id_usersound_max_range     999        // (sound 99 *  multiplier)

// User sounds can be played, repeated, or stopped
enum sound_action : uint8_t {
    SOUND_PLAY = 1,
    SOUND_REPEAT = 2,
    SOUND_STOP = 3
};


// Function IDs *above* 1000 are specific actions.
// They are constructed by adding 1000 to the switch_function enum
#define function_id_other_function_start_range  1000        // start of regular (non-user sound) function IDs

// Each function has a number and an enum name. 
// We don't want Arduino turning these into ints, so use " : byte" to keep the enum to bytes (chars)
// This also means we can't have more than 256 special functions
const byte COUNT_SPECFUNCTIONS  = 9;        // Count of special functions. 
enum switch_function : uint8_t {
    SF_NULL_FUNCTION = 0,
    SF_ENGINE_START,
    SF_ENGINE_STOP,
    SF_ENGINE_TOGGLE,
    SF_CANNON_FIRE,
    SF_MG_FIRE,
    SF_MG_STOP,
    SF_MG2_FIRE,
    SF_MG2_STOP
};

// Friendly names for each function, stored in PROGMEM. Used for printint out the serial port. 
#define FUNCNAME_CHARS  41
const char _FunctionNames_[COUNT_SPECFUNCTIONS][FUNCNAME_CHARS] = 
{   "NULL FUNCTION",                             // 0
    "Engine - Turn On",                          // 1
    "Engine - Turn Off",                         // 2
    "Engine - Toggle",                           // 3
    "Cannon Fire",                               // 4
    "Machine Gun - Fire",                        // 5
    "2nd Machine Gun - Fire"                     // 6
};



//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// TRIGGERS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// Each channel switch trigger has a unique Trigger ID. This is the channel number * 10 + switch position. Example channel 3 position 2 is 32
#define trigger_id_multiplier_rc_channel    10          // Channel trigger ID is defined as (channel number * 10) + switch position. 


// Function/Trigger Pair Definition
//-------------------------------------------------------------------------------------------------------------------------------------------------------------->>
#define MAX_FUNCTION_TRIGGERS           40  // Maximum number of function/trigger pairs we can save
struct _functionTrigger 
{
    uint16_t TriggerID;                     // Each _functionTrigger has a Trigger ID
    uint16_t FunctionID;                    // Each _functionTrigger has a function that will be executed when some input state matches the Trigger ID
};


// Function Pointers
//------------------------------------------------------------------------------------------------------------------------------------------------------------->>
// At the top of the sketch we will also create an array of MAX_FUNCTION_TRIGGERS function pointers. 
typedef void(*void_FunctionPointer)(void);


// RC Channel switch positions
enum switch_positions : byte {          // Names for the switch positions
    NullPos = 0,
    Pos1,
    Pos2,
    Pos3,
    Pos4,
    Pos5,
    Pos6
};

#endif
