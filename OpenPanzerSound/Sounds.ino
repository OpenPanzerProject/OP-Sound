
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUND SETUP
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void InitializeSounds()
{
    Mute();

    InitializeSound(EngineColdStart);
    InitializeSound(EngineHotStart);
    InitializeSound(EngineShutoff);
    for (uint8_t i=0; i<NUM_SOUNDS_IDLE; i++)
    {
        InitializeSound(IdleSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_ACCEL; i++)
    {
        InitializeSound(AccelSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_DECEL; i++)
    {
        InitializeSound(DecelSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_RUN; i++)
    {   // We start counting at zero and continue until we reach the first non-existent sound.
        // It won't matter if there are sounds present beyond the non-existent one, they won' be checked.
        // We will only use the first set of continuously-numbered run sounds. 
        if (InitializeSound(RunSound[i])) NumRunSounds++;   // NumRunSounds will be the upper limit of the RunSound array in practice
        else break;
    }
    for (uint8_t i=0; i<NUM_SOUND_FX; i++)
    {
        InitializeSound(Effect[i]);
    }
    for (uint8_t i=0; i<NUM_USER_SOUNDS; i++)
    {
        InitializeSound(UserSound[i]);
    }

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
    }

    // Initialize our engine slots
    for (uint8_t i=0; i<NUM_ENGINE_SLOTS; i++)
    {
        Engine[i].isActive = false;
        Engine[i].repeat = false;
        Engine[i].ID.Num = 0;
        Engine[i].ID.Slot = i;
        Engine[i].specialCase = EN_TR_NONE;
    }
}

boolean InitializeSound(_soundfile &s)
{   
    // We want to see if the file exists on the SD card
    // If so we want to check the track length
    if (SD.exists(s.fileName))
    {
        FX[0].SDWav.stop();                                             // We can use any SD wav slot, it doesn't matter. We choose FX[0]
        FX[0].SDWav.play(s.fileName);                                   // We have to start playing the file before we can obtain track length
        elapsedMillis timeUp;
        do                                                              // Wait a bit to give time to read the file header, see: https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html
        {
            if (FX[0].SDWav.isPlaying()) s.length = FX[0].SDWav.lengthMillis();     // When the file returns playing we can read the length
        } while (s.length == 0 && timeUp < 6);                          // Wait until length is calculated or 6 mS transpires, whichever happens first (should be able to read length in about 3 mS) 
        if (s.length > 100) 
        {
            s.exists = true;                                            // If the file is some minimum length we consider it valid
            FX[0].SDWav.stop();
            return true;
        }
        else return false;
    }
    else return false;
}


// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUND EFFECT ROUTINES
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Typically Arduino creates function prototypes for all functions in the sketch, but
// if we want to use default arguments we have to specify our own. 
boolean PlaySoundEffect(_soundfile &s);
boolean PlaySoundEffect(_soundfile &s)
{
    _sound_id notused;
    return PlaySoundEffect_AllOptions(s, notused, false, 0, FX_SC_NONE);
}

boolean RepeatSoundEffect(_soundfile &s, uint8_t repeatTimes=0);    // When repeatTimes = 0 the sound will be repeated indefinitely
boolean RepeatSoundEffect(_soundfile &s, uint8_t repeatTimes)
{
    _sound_id notused;
    return PlaySoundEffect_AllOptions(s, notused, true, repeatTimes, FX_SC_NONE);
}

boolean RepeatSoundEffect_wOptions(_soundfile &s, _sound_id &sid, uint8_t repeatTimes=0, _sound_special_case specialCase=FX_SC_NONE);    // When repeatTimes = 0 the sound will be repeated indefinitely
boolean RepeatSoundEffect_wOptions(_soundfile &s, _sound_id &sid, uint8_t repeatTimes,   _sound_special_case specialCase           )
{
    return PlaySoundEffect_AllOptions(s, sid, true, repeatTimes, specialCase);
}

boolean PlaySoundEffect_wOptions(_soundfile &s, _sound_id &sid, _sound_special_case specialCase=FX_SC_NONE);
boolean PlaySoundEffect_wOptions(_soundfile &s, _sound_id &sid, _sound_special_case specialCase           )
{
    return PlaySoundEffect_AllOptions(s, sid, false, 0, specialCase);
}

boolean PlaySoundEffect_AllOptions(_soundfile &s, _sound_id &sid, boolean repeat, uint8_t repeatTimes=0, _sound_special_case specialCase=FX_SC_NONE);
boolean PlaySoundEffect_AllOptions(_soundfile &s, _sound_id &sid, boolean repeat, uint8_t repeatTimes,   _sound_special_case specialCase           )
{
    // We have several slots reserved for sound effects (ie, everything that is not an engine sound). 
    // When we need to play a sound effect we check first to see if there is an unused slot available, and if so, use that one.
    // If all are presently in use we see if one of them is of lower priority than the sound we want to play. If so, we
    // bump the lower priority sound and play the new one instead. If all slots are being used for sounds of higher priority than 
    // the requested sound, then the requested sound simply gets ignored. 

    boolean assigned = false;
    uint8_t FX_Slot = 0;

    // First, check if the requested sound even exists
    if (!s.exists) return false; 

    // Assign the sound to the first unused slot, if any
    for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
    {
        if (assigned == false && FX[i].isActive == false)
        {
            FX[i].SDWav.stop();                                         // There should be nothing playing here since isActive was false, but just in case, make sure to stop prior sound
            FX[i].soundFile = s;                                        // Assign new sound
            FX[i].isActive = true;                                      // Set active flag
            FX[i].repeat = repeat;                                      // Pass any repeat flags
            FX[i].repeatTimes = repeatTimes;                            
            FX[i].timesRepeated = 0;
            FX[i].specialCase = specialCase;                            // Pass any special case flag
            FX[i].SDWav.play(FX[i].soundFile.fileName);                 // Start playing
            FX[i].timeStarted = millis();                               // Save the time that we begin playing this sound
            FX[i].ID.Num++;                                             // Increment the unique ID number of this sound
            FX[i].ID.Slot = i;                                          // Save the slot
            FX_Slot = i;                                                // Save the slot
            assigned = true;                                            // Internal flag so we know this sound was assigned
            PrintStartFx(i);                                            // For debugging only
        }
    }

    // If no unused slot available, assign the sound to the first slot with lower priority, if any
    if (assigned == false)
    {
        for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
        {
            if (assigned == false && FX[i].soundFile.priority < s.priority)   
            {
                FX[i].SDWav.stop();                                     // Stop lower priority sound
                FX[i].soundFile = s;                                    // Assign new sound
                FX[i].isActive = true;                                  // Set active flag
                FX[i].repeat = repeat;                                  // Pass any repeat flag
                FX[i].repeatTimes = repeatTimes;                            
                FX[i].timesRepeated = 0;                
                FX[i].specialCase = specialCase;                        // Pass any special case flag
                FX[i].SDWav.play(FX[i].soundFile.fileName);             // Start playing this sound
                FX[i].timeStarted = millis();                           // Save the time that we begin playing this sound
                FX[i].ID.Num++;                                         // Increment the unique ID number of this sound
                FX[i].ID.Slot = i;                                      // Save the slot
                FX_Slot = i;                                            // Save the slot
                assigned = true;                                        // Internal flag so we know this sound was assigned
                PrintStartFx(i);                                        // For debugging only
            }
        }
    }
   
    if (assigned) 
    {
        sid = FX[FX_Slot].ID;       // Return the slot info to the calling function by saving it in the passed _sound_id struct
        return true;
    }
    else
    {
        return false;               // Sound will not be played, return false. 
    }
}

void StopSoundEffect(_sound_id sid)
{
    FX[sid.Slot].SDWav.stop();                      // Stop the file from playing
    FX[sid.Slot].isActive = false;                  // Open this slot for future sounds
    FX[sid.Slot].repeat = false;                    // Clear repeat flags
    FX[sid.Slot].repeatTimes = 0;
    FX[sid.Slot].timesRepeated = 0;
    FX[sid.Slot].specialCase = FX_SC_NONE;          // Clear any special case flag
    PrintStopFx(sid.Slot);
}

void UpdateEffects(void)
{
    // For each active effect, see if the sound has stopped playing. If so, we either repeat it or update the active flag to false.
    // In certain special cases we may also decide to proceed from one sound to another
    for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
    {   // Keep in mind isPlaying may return false in the first few milliseconds the file is playing. That is why we also throw in a time check. 
        if (FX[i].isActive && FX[i].SDWav.isPlaying() == false && (millis() - FX[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
        {   
            switch (FX[i].specialCase)
            {
                case FX_SC_TURRET:                                                             
                    // This special case indicates we've just finished playing the special turret rotation start sound. Now we need to proceed to the repeating portion of the turret sound
                    FX[i].soundFile = Effect[SND_TURRET];                                   // Regular turret moving sound
                    FX[i].SDWav.play(FX[i].soundFile.fileName);                             // Start playing
                    FX[i].timeStarted = millis();                                           // Record time started
                    FX[i].repeat = true;                                                    // This part is repeating
                    FX[i].repeatTimes = 0;                                                  // Set repeatTimes to 0 to repeat indefinitely
                    FX[i].specialCase = FX_SC_NONE;                                         // No special case after this (the turret stop moving command will take care of playing any special turret stop sound)
                    PrintStartFx(i);
                    break;

                case FX_SC_BARREL:
                    // This special case indicates we've just finished playing the special barrel elevation start sound. Now we need to proceed to the repeating portion of the barrel sound
                    FX[i].soundFile = Effect[SND_BARREL];                                   // Now we move on to the barrel moving sound
                    FX[i].SDWav.play(FX[i].soundFile.fileName);                             // Start playing
                    FX[i].timeStarted = millis();                                           // Record time started
                    FX[i].repeat = true;                                                    // This part is repeating
                    FX[i].repeatTimes = 0;                                                  // Set repeatTimes to 0 to repeat indefinitely
                    FX[i].specialCase = FX_SC_NONE;                                         // No special case after this (the barrel stop moving command will take care of playing any special barrel stop sound)
                    PrintStartFx(i);
                    break;

                case FX_SC_MG:
                    // This special case indicates we've just finished playing a special machine gun start sound. Now we need to proceed to the repeating (loop) portion of the MG sound
                    FX[i].soundFile = Effect[SND_MG];                                       // Now we move on to the machine gun firing sound
                    FX[i].SDWav.play(FX[i].soundFile.fileName);                             // Start playing
                    FX[i].timeStarted = millis();                                           // Record time started
                    FX[i].repeat = true;                                                    // This part is repeating
                    FX[i].repeatTimes = 0;                                                  // Set repeatTimes to 0 to repeat indefinitely
                    FX[i].specialCase = FX_SC_NONE;                                         // No special case after this (the machine gun stop command will take care of playing any special stop sound)
                    PrintStartFx(i);
                    break;
                    
                case FX_SC_NONE:
                default:
                    if (FX[i].repeat == true)   
                    {
                        if (FX[i].repeatTimes == 0)                                         // Repeat indefinitely
                        {
                            FX[i].SDWav.play(FX[i].soundFile.fileName);                     // Restart
                            FX[i].timeStarted = millis();                                   // All the prior flags can be left the same, but this one needs updated
                        }
                        else                                                                // Repeat a specified number of times then stop
                        {
                            if (++FX[i].timesRepeated < FX[i].repeatTimes)                  // Increment repeat count and check against the total number of times we want to repeat
                            {
                                FX[i].SDWav.play(FX[i].soundFile.fileName);                 // Restart
                                FX[i].timeStarted = millis();                               // All the prior flags can be left the same, but this one needs updated
                            }
                            else                                                            // In this case the sound has been repeated enough times, so we can stop it. 
                            {
                                FX[i].isActive = false;
                                FX[i].repeat = false;                                       // Clear the repeat flags
                                FX[i].repeatTimes = 0;
                                FX[i].timesRepeated = 0;
                            }
                        }
                    }
                    else FX[i].isActive = false;                                            // Otherwise this was not a repeating sound, clear the active flag and open this slot for future sounds
            }
        }
    }
}


// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUND EFFECTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void CannonFire()
{
    PlaySoundEffect(Effect[SND_FIRE_CANNON]); 
}

void CannonHit()
{
    PlaySoundEffect(Effect[SND_HIT_CANNON]);
}

void MGHit()
{
    PlaySoundEffect(Effect[SND_HIT_MG]);
}

void Destroyed()
{
    PlaySoundEffect(Effect[SND_HIT_DESTROY]);
}

void LightSwitch()
{
    PlaySoundEffect(Effect[SND_LIGHT_SWITCH]);
}

void Beep(uint8_t times)
{ 
    if (times == 1) PlaySoundEffect(Effect[SND_BEEP]);
    else            RepeatSoundEffect(Effect[SND_BEEP], times);
}

void MG(boolean start)
{
    static _sound_id sid;
    static boolean active = false; 

    if (start)
    {
        if (!active)                                                            // No need to start if it is already going
        {
            // There are two options for playing the machine gun sound. 
            // One involves stringing along multiple sound files, the first being a "start" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_MG_START].exists)                                // If the special start sound exists play it first. 
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_MG flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_MG_START], sid, FX_SC_MG)) active = true;   
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_MG], sid)) active = true;  // No start sound, go straight to repeating the loop portion 
        }
    }
    else
    {   // In this case we want to stop the machine gun sound
        if (active)                                                             // Is the sound even active? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_MG_STOP].exists)                                     // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing turret running sound
                FX[sid.Slot].soundFile = Effect[SND_MG_STOP];                   // Set the soundfile to the special turret stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_NONE;                          // No special case
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the turret stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                active = false;                                                 // We are done with the turret sound after this one is over, so set this function's internal active flag to false (not the same as the FX[].isActive flag!)
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid);   // In this case we don't have a separate stop sound, we can just immediately stop
                active = false; 
            }
        }
    }
}

void TurretRotation(boolean start)
{
    static _sound_id sid;
    static boolean active = false; 

    if (start)
    {
        if (!active)                                                            // No need to start if it is already going
        {
            // There are two options for playing the turret rotation sound. 
            // One involves stringing along multiple sound files, the first being a special "start moving" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_TURRET_START].exists)                                // If the special start sound exists play it first. 
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_TURRET flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_TURRET_START], sid, FX_SC_TURRET)) active = true;   
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_TURRET], sid)) active = true;  // No start sound, go straight to repeating the loop portion 
        }
    }
    else
    {   // In this case we want to stop the turret sound
        if (active)                                                             // Is the sound even active? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_TURRET_STOP].exists)                                 // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing turret running sound
                FX[sid.Slot].soundFile = Effect[SND_TURRET_STOP];               // Set the soundfile to the special turret stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_NONE;                          // No special case
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the turret stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                active = false;                                                 // We are done with the turret sound after this one is over, so set this function's internal active flag to false (not the same as the FX[].isActive flag!)
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid);   // In this case we don't have a separate stop sound, we can just immediately stop
                active = false; 
            }
        }
    }
}

void BarrelElevation(boolean start)
{
    static _sound_id sid;
    static boolean active = false; 

    if (start)
    {
        if (!active)                                                            // No need to start if it is already going
        {
            // There are two options for playing the barrel elevation sound. 
            // One involves stringing along multiple sound files, the first being a special "start moving" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_BARREL_START].exists)                                // If the special start sound exists play it first. 
            {
                // We pass the FX_SC_BARREL flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_BARREL_START], sid, FX_SC_BARREL)) active = true;   
            }
            // The other option for playing the barrel sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_BARREL], sid)) active = true; // No start sound, go straight to loop portion and set the repeat flag 
        }
    }
    else
    {   // In this case we want to stop the barrel sound
        if (active)                                                             // Is the sound even active? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_BARREL_STOP].exists)                                 // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing barrel running sound
                FX[sid.Slot].soundFile = Effect[SND_BARREL_STOP];               // Set the soundfile to the special barrel stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_NONE;                          // No special case
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the barrel stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                active = false;                                                 // We are done with the barrel sound after this one is over, so set this function's internal active flag to false (not the same as the FX[].isActive flag!)
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid);   // In this case we don't have a separate stop sound, we can just immediately stop
                active = false; 
            }
        }
    }
}

void Repair(boolean startRepair)
{
    static _sound_id RP_SID;
    static boolean active = false; 

    if (startRepair && !active)
    {
        if (!active)                                                   // No need to start if it is already going
        {
            if (RepeatSoundEffect_wOptions(Effect[SND_REPAIR], RP_SID))    // Set the repeat flag for the Repair sound
            {
                active = true;
            }
        }
    }
    else                                                                // In this case we want to stop the Repair sound
    {   
        if (active)                                                     // Is the sound even active? 
        {
            StopSoundEffect(RP_SID);                                    // Stop the sound
            active = false;                                             // Update our internal flag
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------->
// USER SOUNDS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void PlayUserSound(uint8_t i, boolean startSound, boolean repeat)
{
static _sound_id US_SID[NUM_USER_SOUNDS];
static boolean active[NUM_USER_SOUNDS] = {false, false, false, false, false, false}; 

    // Only accept valid user sounds
    if (i > NUM_USER_SOUNDS) return;

    // We will be passed a user sound "i" from 1 to NUM_USER_SOUNDS
    // But we want to use this number to access elements of a zero-based array, so let's subtract 1
    i -= 1;

    if (startSound)                                                     // In this case we want to start the sound
    {
        if (repeat)
        {
            if (!active[i])                                             // No need to start if it is already going
            {   
                if (RepeatSoundEffect_wOptions(UserSound[i], US_SID[i])) active[i] = true; 
            }
        }
        else
        {
            PlaySoundEffect(UserSound[i]);                              // If we are not repeating we don't need to worry about the internal active flag, 
            active[i] = false;                                          // we can just keep it false
        }
    }
    else                                                                // In this case we want to stop the sound
    {   
        if (active[i])                                                  // Is the sound even active? 
        {
            StopSoundEffect(US_SID[i]);                                 // Stop the sound
            active[i] = false;                                          // Update our internal flag
        }
    }    
    
}



// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SQUEAKS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void SetSqueakMin(uint8_t val, uint8_t squeakNum)                       // Set squeak minimum interval time
{
    uint16_t minVal; 
    
    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        minVal = ((val * 50) + 500);                                    // Valid values are 0-190 which equate to intervals from 500mS to 10000mS (1/2 to 10 seconds)
        ramcopy.squeakInfo[squeakNum].intervalMin = minVal;             // Update our variable in RAM and in EEPROM
        EEPROM.updateBlock(offsetof(_eeprom_data, squeakInfo[squeakNum].intervalMin), ramcopy.squeakInfo[squeakNum].intervalMin);
    }
}
void SetSqueakMax(uint8_t val, uint8_t squeakNum)                       // Set squeak maximum interval time
{
    uint16_t maxVal; 
    
    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        maxVal = ((val * 50) + 500);                                    // Valid values are 0-190 which equate to intervals from 500mS to 10000mS (1/2 to 10 seconds)
        ramcopy.squeakInfo[squeakNum].intervalMax = maxVal;             // Update our variable in RAM and in EEPROM
        EEPROM.updateInt(offsetof(_eeprom_data, squeakInfo[squeakNum].intervalMax), ramcopy.squeakInfo[squeakNum].intervalMax);   
    }
}
void EnableSqueak(uint8_t val, uint8_t squeakNum)
{   // This is for enabling the squeak or not, which is a user setting. It is not the same thing
    // as active/inactive (that determines if the squeak is squeaking or just waiting to squeak).
    // If a squeak is disabled, the sound won't play. 

    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        ramcopy.squeakInfo[squeakNum].enabled = (boolean)val;           // Update our variable in RAM and in EEPROM
        EEPROM.updateInt(offsetof(_eeprom_data, squeakInfo[squeakNum].enabled), ramcopy.squeakInfo[squeakNum].enabled);   
    }                
}
void StartSqueaks(void)
{   
// We actually don't start squeaking right away because that can sound weird. We wait until the tank has been moving for
// some amount of time before truly starting them
    #define SQUEAK_DELAY_mS     3000     
    if (AllSqueaks_Active == false)
    {   
        SqueakTimerID = timer.setTimeout(SQUEAK_DELAY_mS, StartSqueaksForReal);
        AllSqueaks_Active = true;
    }
}
void StartSqueaksForReal(void)
{
    if (AllSqueaks_Active)
    {
        for (uint8_t i=0; i<NUM_SQUEAKS; i++)
        {   // If squeak is enabled but not active
            if (ramcopy.squeakInfo[i].enabled && !ramcopy.squeakInfo[i].active && Effect[SND_SQUEAK_OFFSET+i].exists) 
            { 
                ramcopy.squeakInfo[i].active = true;    // Activate this squeak 
                AssignRandomTimeToSqueak(i);            // Rather than play it now, which would cause them all to go off at once, assign it a random time in the future. 
                                                        // The UpdateSqueaks() function will squeak them when the time is up. 
            }
        }
        if (DEBUG) { DebugSerial.println(F("Start Squeaks")); }
    }
}

void StopSqueaks(void)
{
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        ramcopy.squeakInfo[i].active = false;
    }
    AllSqueaks_Active = false;
    if (timer.isEnabled(SqueakTimerID)) timer.deleteTimer(SqueakTimerID);
    if (DEBUG) { DebugSerial.println(F("Stop Squeaks")); }
}

void UpdateSqueaks(void)
{
    // If the random length of time has passed for a given squeak, squeak it again
    // This function needs to be polled routinely from the main loop. 
    
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        if (ramcopy.squeakInfo[i].active && (millis() - ramcopy.squeakInfo[i].lastSqueak) >= ramcopy.squeakInfo[i].squeakAfter) PlaySqueak(i);
    }
}

void PlaySqueak(uint8_t i)
{
    if (ramcopy.squeakInfo[i].active)
    {
        PlaySoundEffect(Effect[SND_SQUEAK_OFFSET + i]);         // Play the squeak sound
        AssignRandomTimeToSqueak(i);        // Assign a random time before we squeak again
        PrintNextSqueakTime(i);             // For debugging
    }
}

void AssignRandomTimeToSqueak(uint8_t i)
{
    ramcopy.squeakInfo[i].lastSqueak = millis();                                                                        // Save the current time so we can compare to it later
    ramcopy.squeakInfo[i].squeakAfter = random(ramcopy.squeakInfo[i].intervalMin,ramcopy.squeakInfo[i].intervalMax);    // Calculate a random amount of time within the min and max squeak interval    
}


// -------------------------------------------------------------------------------------------------------------------------------------------------->
// ENGINE SOUNDS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Typically Arduino creates function prototypes for all functions in the sketch, but
// if we want to use default arguments we have to specify our own. 
boolean FadeEngineSound(_soundfile &s, _engine_transition engineTransition=EN_TR_NONE);
boolean FadeEngineSound(_soundfile &s, _engine_transition engineTransition)
{
    return PlayEngineSound_wOptions(s, false, true, engineTransition);
}

boolean FadeRepeatEngineSound(_soundfile &s, _engine_transition engineTransition=EN_TR_NONE);
boolean FadeRepeatEngineSound(_soundfile &s, _engine_transition engineTransition)
{
    return PlayEngineSound_wOptions(s, true, true, engineTransition);
}

boolean PlayEngineSound(_soundfile &s, _engine_transition engineTransition=EN_TR_NONE);
boolean PlayEngineSound(_soundfile &s, _engine_transition engineTransition)
{
    return PlayEngineSound_wOptions(s, false, false, engineTransition);
}

boolean RepeatEngineSound(_soundfile &s, _engine_transition engineTransition=EN_TR_NONE);
boolean RepeatEngineSound(_soundfile &s, _engine_transition engineTransition)
{
    return PlayEngineSound_wOptions(s, true, false, engineTransition);
}

boolean PlayEngineSound_wOptions(_soundfile &s, boolean repeat, boolean Fade, _engine_transition engineTransition=EN_TR_NONE);
boolean PlayEngineSound_wOptions(_soundfile &s, boolean repeat, boolean Fade, _engine_transition engineTransition)
{
    // First, check if the requested sound even exists
    if (!s.exists) return false; 

    EnCurrent = EnNext;
    Engine[EnCurrent].SDWav.stop();
    Engine[EnCurrent].soundFile = s;                                        // Assign sound
    Engine[EnCurrent].isActive = true;                                      // Set active flag
    Engine[EnCurrent].repeat = repeat;                                      // Pass any repeat flag
    Engine[EnCurrent].specialCase = engineTransition;                       // Pass any special case flag
    if (Fade) Fader[EnCurrent].fadeIn(ENGINE_FADE_TIME_MS);                 // Fade in if requested
    Engine[EnCurrent].SDWav.play(Engine[EnCurrent].soundFile.fileName);     // Start playing
    Engine[EnCurrent].timeStarted = millis();                               // Save the time that we begin playing this sound
    Engine[EnCurrent].ID.Num++;                                             // Increment the unique ID number of this sound
    Engine[EnCurrent].ID.Slot = EnNext;                                     // Save the slot
    PrintStartEngineSound(EnCurrent);                                       // For debugging only

    SwapEngineSlot();                                                       // Swap engine sound banks
    
    if (Fade && Engine[EnNext].SDWav.isPlaying())                           // We are now at the prior sound, which we want to free up for the next sound
    { 
        Fader[EnNext].fadeOut(ENGINE_FADE_TIME_MS);                         // Fade out prior sound if it is still playing
        Engine[EnNext].repeat = false;                                      // We don't want that prior sound to repeat
        Engine[EnNext].specialCase = EN_TR_NONE;                            // Clear the special case from this sound because it is ending.
    }
    
    return true;
}

void SwapEngineSlot(void)
{
    if (EnNext == 0) EnNext = 1;                                            // Update EnNext to the next slot
    else             EnNext = 0;
}

void SetEngineState(_engine_state ES)
{   // Save the existing state to prior, then update. 
    EngineState_Prior = EngineState; 
    EngineState = ES;
}
void StartEngine(void)
{
    // We can only start the engine if the engine is enabled and currently off
    if (EngineEnabled && EngineState == ES_OFF) SetEngineState(ES_START);
}

void StopEngine(void)
{
    // We can stop the engine anytime if we aren't already stopped or in the process of shutting down
    if (EngineState != ES_OFF && EngineState != ES_SHUTDOWN) SetEngineState(ES_SHUTDOWN);
}

void SetEngineSpeed(uint8_t speed)
{

    // Set our global engine speed variable
    EngineSpeed = speed;    

    // Ignore if we are not already enabled and running
    if (!EngineEnabled || EngineState == ES_OFF || EngineState == ES_START || EngineState == ES_SHUTDOWN)   return;
   
    // Otherwise check for transition cases
    if (speed > 0)
    {
        // If we are in idle state and speed is positive, we need to transition to accelerating (and from there to run)
        if (EngineState == ES_IDLE) SetEngineState(ES_ACCEL);
    }
    else
    {   // If we are running and speed is 0, we need to transition to decelerating (and from there to idle)
        if (EngineState == ES_RUN) SetEngineState(ES_DECEL);
    }
}

void UpdateEngine(void)
{
    static boolean EngineRunning = false;
    static boolean EngineCold = true;
    static int8_t  nextAccelSound = -1;           // We can have multiple accel sounds. This variable keeps track of which one we used last. 
    static int8_t  nextDecelSound = -1;           // We can have multiple decel sounds. This variable keeps track of which one we used last. 
    static int8_t  nextIdleSound = -1;            // We can have multiple idle sounds. This variable keeps track of which one we used last. 

    switch (EngineState)
    {
        case ES_OFF:
            break;

        case ES_START:
            if (!EngineRunning)
            {
                StopAllEngineSounds();                                                      // No engine sounds should be playing, but let's make sure
                if (EngineCold)                                                             // Cold start
                {   
                    if (EngineColdStart.exists)     PlayEngineSound(EngineColdStart, EN_TR_IDLE); // Try to play the cold start sound, 
                    else if (EngineHotStart.exists) PlayEngineSound(EngineHotStart, EN_TR_IDLE);  // But if it doesn't exist we don't mind using the hot one anyway
                }
                else                                                                        // Hot start 
                {
                    if      (EngineHotStart.exists)  PlayEngineSound(EngineHotStart, EN_TR_IDLE);  // Try to play the hot start sound, 
                    else if (EngineColdStart.exists) PlayEngineSound(EngineColdStart, EN_TR_IDLE); // But if it doesn't exist we don't mind using the cold one anyway 
                }
                EngineCold = false;                                                         // From here on out we will use the hot start sound
                EngineRunning = true;                                                       // Set the engine running flag
            }
            break;

        case ES_IDLE:
            // We don't need to do anything here. UpdateIndividualEngineSounds() will automatically repeat the idle sound for as long as needed. 
            break;

        case ES_ACCEL:
            // Here we are just starting to move, so play the accel sound if we have one
            if (GetNextSound(AccelSound, nextAccelSound, NUM_SOUNDS_ACCEL))                 // Get next accel sound of the various we may have available to us
            {
                FadeEngineSound(AccelSound[nextAccelSound], EN_TR_ACCEL_RUN);               // Transition to run after accel when done
            }
            else
            {                                                                               // If we don't have an accel sound, go straight to run
                FadeRepeatEngineSound(RunSound[GetRunSoundNumBySpeed()], EN_TR_RUN);        // If no accel sound available, fade straight into run sound and start repeating it when done
            }
            // Either way, we are going to the run state next
            SetEngineState(ES_RUN);
            break;

        case ES_DECEL:
            // Here we are already moving, but we want to stop (return to idle). We play the decel sound if we have one
            if (GetNextSound(DecelSound, nextDecelSound, NUM_SOUNDS_DECEL))                 // Get next Decel sound of the various we may have available to us
            {
                FadeEngineSound(DecelSound[nextDecelSound], EN_TR_IDLE);                    // Transition to idle after decel when done
            }
            else                                                                            // If no Decel sound go straight to idle
            {                                                                               // Get next idle sound of the various we may have available to us
                if (GetNextSound(IdleSound, nextIdleSound, NUM_SOUNDS_IDLE))                // This should definitely return true because engine is only enabled if we have at least one idle sound
                {
                    FadeRepeatEngineSound(IdleSound[nextIdleSound]);                        // Fade in idle sound and start repeating it
                }
            }            
            // Either way, we are going to the idle state next
            SetEngineState(ES_IDLE);
            break;
            
        case ES_RUN:
            // We don't need to do anything here. UpdateIndividualEngineSounds() will automatically update and/or repeat the run sound for as long as needed, based on EngineSpeed
            break;

        case ES_SHUTDOWN:
            FadeEngineSound(EngineShutoff);                                                 // Fade in the shutdown sound
            SetEngineState(ES_OFF);
            EngineRunning = false;                                                          // Set the engine running flag
            // But we may want to treat shutdown differently depending on where we are at (running, idling, whatever)
            break;
        
    }
}

void UpdateIndividualEngineSounds(void)
{
static int8_t    nextIdleSound = -1;            // We can have multiple idle sounds. This variable keeps track of which one we used last. 
uint8_t          RunSoundNum;
static uint8_t   LastRunSoundNum = 0;           // 

    // For each active engine slot, see if the sound has stopped playing. If so, we either repeat it or update the active flag to false.
    // We may also take different actions based on the specialCase flag
    for (uint8_t i=0; i<NUM_ENGINE_SLOTS; i++)
    {   // Keep in mind isPlaying may return false in the first few milliseconds the file is playing. That is why we also throw in a time check. 
        if (Engine[i].isActive)
        {   
            switch (Engine[i].specialCase)
            {
                case EN_TR_IDLE:
                    // Here we transition from some other state into idle by fading. The other state will either be startup or deceleration
                    // We wait until the prior sound is nearly done playing
                    if (Engine[i].SDWav.isPlaying() && (Engine[i].SDWav.positionMillis() >= (Engine[i].soundFile.length - ENGINE_FADE_TIME_MS)))
                    {
                        // Get next idle sound of the various we may have available to us
                        if (GetNextSound(IdleSound, nextIdleSound, NUM_SOUNDS_IDLE))        // This should definitely return true because engine is only enabled if we have at least one idle sound
                        {
                            FadeRepeatEngineSound(IdleSound[nextIdleSound]);                // Fade in and start repeating
                            SetEngineState(ES_IDLE);                                        // We are now in the idle state
                        }
                        else 
                        {
                            // An error occured. We go back to the stop state, and disable the engine function completely
                            DisableEngine();
                        }
                    }
                    break;

                case EN_TR_ACCEL_RUN:
                    // Here we transition from accel sound to run sound. We wait until the accel sound is nearly complete before fading. 
                    if (Engine[i].SDWav.isPlaying() && (Engine[i].SDWav.positionMillis() >= (Engine[i].soundFile.length - ENGINE_FADE_TIME_MS)))
                    {
                        RunSoundNum = GetRunSoundNumBySpeed();
                        FadeRepeatEngineSound(RunSound[RunSoundNum], EN_TR_RUN);            // Fade into running sound and start repeating it. 
                        LastRunSoundNum = RunSoundNum;
                        SetEngineState(ES_RUN);
                    }
                    break;


                case EN_TR_RUN:
                    // Here we are in run mode and we need to decide whether we should repeat the current run sound, or fade to a higher or lower run sound (speed change)
                    if (EngineState == ES_RUN)
                    {
                        RunSoundNum = GetRunSoundNumBySpeed();
                        if (RunSoundNum != LastRunSoundNum)
                        {   // We need to transition to a new run sound. We don't wait for the previous one to finish.
                            FadeRepeatEngineSound(RunSound[RunSoundNum], EN_TR_RUN);        // Fade in to the new engine run sound and start repeating it
                            LastRunSoundNum = RunSoundNum;
                        }
                        else
                        {   // We are at the same speed (or within the same speed range encompassed by this sound). Repeat the sound if it is over. 
                            if (Engine[i].SDWav.isPlaying() == false && (millis() - Engine[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                            {
                                Engine[i].SDWav.play(Engine[i].soundFile.fileName);         // Restart
                                Engine[i].timeStarted = millis();                           // All the prior flags can be left the same, but this one needs updated
                            }                            
                        }
                    }
                    break;
                
                case EN_TR_NONE:    // No transition, just decide if a sound needs to be repeated or set to inactive. 
                    // Check if sound is done playing. We already know it is flagged as active, so if it is done playing we need to update the flag to inactive, or set it to repeat if required
                    if (Engine[i].SDWav.isPlaying() == false && (millis() - Engine[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                    {
                        if (Engine[i].repeat == true)   
                        {
                            Engine[i].SDWav.play(Engine[i].soundFile.fileName);             // Restart
                            Engine[i].timeStarted = millis();                               // All the prior flags can be left the same, but this one needs updated
                        }
                        else Engine[i].isActive = false;                                    // Otherwise clear the active flag and open this slot for future sounds
                    }
                    break;
            }
        }
    }
}

uint8_t GetRunSoundNumBySpeed()
{
    return map(EngineSpeed, 0, 255, 0, NumRunSounds);
}

void StopAllEngineSounds(void)
{
    for (uint8_t i=0; i<NUM_ENGINE_SLOTS; i++)
    {
        Engine[i].SDWav.stop();
    }
    EnNext = 0; // Reset the next engine to the first slot
}

void DisableEngine(void)
{
    EngineState = ES_OFF;
    EngineEnabled = false;
    StopAllEngineSounds();
}


// -------------------------------------------------------------------------------------------------------------------------------------------------->
// GENERAL SOUND UTILITIES
// -------------------------------------------------------------------------------------------------------------------------------------------------->
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


