
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUNDS - GENERAL
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void InitializeSounds()
{
    Mute();     // Mute because we are going to play briefly each sound in order to determine its length

    // ENGINE
    // -------------------------------------------------------------------------------------------------->> 
        // Start
        InitializeSound(&EngineColdStart);
        InitializeSound(&EngineHotStart);
        // Idle
        for (uint8_t i=0; i<NUM_SOUNDS_IDLE; i++)
        {
            InitializeSound(&IdleSound[i]);
        }
        // Acceleration   
        for (uint8_t i=0; i<NUM_SOUNDS_ACCEL; i++)
        {
            InitializeSound(&AccelSound[i]);
        }
        // Running
        for (uint8_t i=0; i<NUM_SOUNDS_RUN; i++)
        {   // We start counting at zero and continue until we reach the first non-existent sound.
            // It won't matter if there are sounds present beyond the non-existent one, they won't be checked.
            // We will only use the first set of continuously-numbered run sounds. 
            if (InitializeSound(&RunSound[i])) NumRunSounds++;   // NumRunSounds will be the upper limit of the RunSound array in practice
        }
        // Deceleration
        for (uint8_t i=0; i<NUM_SOUNDS_DECEL; i++)
        {
            InitializeSound(&DecelSound[i]);
        }
        // Shutoff
        InitializeSound(&EngineShutoff);

    // TRACK OVERLAY
    // -------------------------------------------------------------------------------------------------->> 
        InitializeSound(&TrackOverlayStart);
        for (uint8_t i=0; i<NUM_SOUNDS_TRACK_OVERLAY; i++)
        {   // We start counting at zero and continue until we reach the first non-existent sound.
            // It won't matter if there are sounds present beyond the non-existent one, they won't be checked.
            // We will only use the first set of continuously-numbered track overlay sounds. 
            if (InitializeSound(&TrackOverlaySound[i])) NumOverlaySounds++;   // NumOverlaySounds will be the upper limit of the TrackOverlaySound array in practice
        }
        InitializeSound(&TrackOverlayStop);

    // CANNON FIRE SOUNDS
    // -------------------------------------------------------------------------------------------------->> 
        for (uint8_t i=0; i<NUM_SOUNDS_CANNON; i++)
        {
            InitializeSound(&CannonFireSound[i]);
        }

    // SOUND EFFECTS
    // -------------------------------------------------------------------------------------------------->> 
        for (uint8_t i=0; i<NUM_SOUND_FX; i++)
        {
            InitializeSound(&Effect[i]);
        }

    // USER SOUNDS
    // -------------------------------------------------------------------------------------------------->>     
        for (uint8_t i=0; i<NUM_USER_SOUNDS; i++)
        {
            InitializeSound(&UserSound[i]);
        }


    // SLOTS
    // -------------------------------------------------------------------------------------------------->> 
        // Initialize our sound effect slots
        for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
        {
            FX[i].isActive = false;
            FX[i].repeat = false;
            FX[i].repeatTimes = 0;
            FX[i].timesRepeated = 0;
            FX[i].ID.Num = 0;
            FX[i].ID.Slot = i;
            FX[i].specialCase = FX_SC_NONE;
            FX[i].fadingOut = false;
        }
    
        // Initialize our engine slots
        for (uint8_t i=0; i<NUM_ENGINE_SLOTS; i++)
        {
            Engine[i].isActive = false;
            Engine[i].repeat = false;
            Engine[i].ID.Num = 0;
            Engine[i].ID.Slot = i;
            Engine[i].specialCase = EN_TR_NONE;
            Engine[i].fadingOut = false;
        }
}

boolean InitializeSound(_soundfile *s)
{   
static uint8_t num = 0;

    if (num < COUNT_TOTAL_SOUNDFILES)
    {
        allSoundFiles[num++] = s;       // Save a pointer to every single sound file
    }
    else
    {
        Serial.println(F("ERROR: You need to update the COUNT_TOTAL_SOUNDFILES number!"));
    }
    
    // We want to see if the file exists on the SD card
    // If so we want to check the track length
    if (SD.exists(s->fileName))
    {
        FX[0].SDWav.stop();                                             // We can use any SD wav slot, it doesn't matter. We choose FX[0]
        FX[0].SDWav.play(s->fileName);                                  // We have to start playing the file before we can obtain track length
        elapsedMillis timeUp;
        do                                                              // Wait a bit to give time to read the file header, see: https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html
        {
            if (FX[0].SDWav.isPlaying()) s->length = FX[0].SDWav.lengthMillis();     // When the file returns playing we can read the length
        } while (s->length == 0 && timeUp < 6);                         // Wait until length is calculated or 6 mS transpires, whichever happens first (should be able to read length in about 3 mS) 
        if (s->length > 50)                                             // Require sound to be at least 50ms long before we consider it an actual sound (some MG sounds can actually be quite short...)
        {
            s->exists = true;                                           // If the file is some minimum length we consider it valid
            FX[0].SDWav.stop();
            return true;
        }
        else return false;
    }
    else return false;
}

void StopAllSounds(void)
{
    StopAllSoundEffects();
    StopAllEngineSounds();
}  

boolean GetNextSound(_soundfile s[], int8_t &startPos, uint8_t numToCheck)
{
    // This function finds the next available sound in the current set. 
    // Various sound sets permit more than one sound for the same action - for example, we can have multiple idle sounds, 
    // the idea being to play a different one each time we come to idle for variety. 
    // If the next sound is found, the function returns true, and it also returns the new position in the set through the startPos variable passed by reference.
    // This can let the calling routine pass the new startPos next time so we increment to the next sound.
    // If no sound is found within the set the function returns false and startPos remains unchanged. 
    
    int8_t nextSound = startPos;
    boolean fileFound = false;
    uint8_t checks = 0;

    do
    {
        nextSound += 1;
        if (nextSound >= numToCheck) nextSound = 0;
        if (s[nextSound].exists) fileFound = true;    
        checks += 1;
    }
    while (fileFound == false && checks < numToCheck);    

    if (fileFound) 
    {
        startPos = nextSound;
        return true;
    }
    else return false;
}




