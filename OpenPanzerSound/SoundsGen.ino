
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUNDS - GENERAL
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void InitializeSounds()
{
    uint8_t i;
    
    Mute();
    MuteAmp();     // Mute because we are going to play briefly each sound in order to determine its length

    // ENGINE
    // -------------------------------------------------------------------------------------------------->> 
        // Start
        InitializeSound(&EngineColdStart);
        InitializeSound(&EngineHotStart);
        // Idle
        for (i=0; i<NUM_SOUNDS_IDLE; i++)
        {
            InitializeSound(&IdleSound[i]);
        }
        InitializeSound(&EngineDamagedIdle);
        // Acceleration   
        for (i=0; i<NUM_SOUNDS_ACCEL; i++)
        {
            InitializeSound(&AccelSound[i]);
        }
        // Running
        for (i=0; i<NUM_SOUNDS_RUN; i++)
        {   // We start counting at zero and continue until we reach the first non-existent sound.
            // It won't matter if there are sounds present beyond the non-existent one, they won't be checked.
            // We will only use the first set of continuously-numbered run sounds. 
            if (InitializeSound(&RunSound[i])) NumRunSounds++;   // NumRunSounds will be the upper limit of the RunSound array in practice
        }
        // Deceleration
        for (i=0; i<NUM_SOUNDS_DECEL; i++)
        {
            InitializeSound(&DecelSound[i]);
        }
        // Shutoff
        InitializeSound(&EngineShutoff);

    // TRACK OVERLAY
    // -------------------------------------------------------------------------------------------------->> 
        InitializeSound(&TrackOverlayStart);
        for (i=0; i<NUM_SOUNDS_TRACK_OVERLAY; i++)
        {   // We start counting at zero and continue until we reach the first non-existent sound.
            // It won't matter if there are sounds present beyond the non-existent one, they won't be checked.
            // We will only use the first set of continuously-numbered track overlay sounds. 
            if (InitializeSound(&TrackOverlaySound[i])) NumOverlaySounds++;   // NumOverlaySounds will be the upper limit of the TrackOverlaySound array in practice
        }
        InitializeSound(&TrackOverlayStop);

    // SOUND EFFECTS
    // -------------------------------------------------------------------------------------------------->> 
        for (i=0; i<NUM_SOUND_FX; i++)
        {
            InitializeSound(&Effect[i]);
        }

    // CANNON RELOADED/READY SOUNDS
    // -------------------------------------------------------------------------------------------------->> 
        for (i=0; i<NUM_CANNON_READY_SOUNDS; i++)
        {
            InitializeSound(&CannonReadySound[i]);
        }

    // USER SOUNDS
    // -------------------------------------------------------------------------------------------------->>     
        // Individual
        for (i=0; i<NUM_USER_SOUNDS; i++)
        {
            InitializeSound(&UserSound[i]);
        }
        // Sound Bank A
        for (i=0; i<NUM_SOUNDS_BANK_A; i++)
        {
            InitializeSound(&SoundBankA[i]);
        }        
        // Sound Bank B
        for (i=0; i<NUM_SOUNDS_BANK_B; i++)
        {
            InitializeSound(&SoundBankB[i]);
        }                

    // SLOTS
    // -------------------------------------------------------------------------------------------------->> 
        // Initialize our sound effect slots
        for (i=0; i<NUM_FX_SLOTS; i++)
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
        for (i=0; i<NUM_ENGINE_SLOTS; i++)
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
        DebugSerial.println(F("ERROR: You need to update the COUNT_TOTAL_SOUNDFILES number!"));
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

/*  Adapted from "RandomHat" - Thanks to Paul Badger and User31481
 *  
 *  RandomHat
 *  Paul Badger 2007 - updated for Teensy compile 2017
 *  Choose one from a hat of n consecutive choices each time through loop
 *  Choose each number exactly once before reseting and choosing again
 *  
 *  User31481:
 *  https://arduino.stackexchange.com/questions/45413/select-from-custom-array-without-repeat
 *  
 */
int randomHat(int randomArray[], int numberInHat, int &numLeft, _soundfile s[]) 
{
    int Pick = -1;                              // This is the return variable with the random number from the pool
    int Index;
   
    if  (numLeft == 0)                          // Hat is empty - all have been choosen - fill up array again
    {   
        for (int i = 0 ; i < numberInHat; i++)  // Put {0, ..., n} into random array.
        {
            if (s[i].exists) { randomArray[i] = i; numLeft += 1; }
            else randomArray[i] = -1;           // Only activate index positions that match an existing sound slot
        }
        
        if (numLeft == 0) return -1;            // If there was nothing in the array, return -1
    }
    
    while (Pick == -1) 
    {
        Index = random(numberInHat);            // Choose a random index
        Pick = randomArray[Index];              // If the random selection has been used before, Pick will equal -1 and therefore the while loop will repeat until a value >-1 is found, meaning, it hasn't been used before
    }
    
    numLeft -= 1;                               // We have one fewer left for next time
    randomArray[Index] = -1;                    // This item in the list has been used, so set its index to -1 to indicate it is no longer available.

    return Pick;                                // Return the random index
}



