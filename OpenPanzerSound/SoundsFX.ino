
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

boolean PlaySoundEffect_wCase(_soundfile &s, _sound_special_case specialCase=FX_SC_NONE);
boolean PlaySoundEffect_wCase(_soundfile &s, _sound_special_case specialCase)
{
    _sound_id notused;
    return PlaySoundEffect_AllOptions(s, notused, false, 0, specialCase);
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
    for (uint8_t i=0; i<AvailableFXSlots; i++)                          // We use AvailableFXSlots here, not NUM_FX_SLOTS (though it might be the same thing if track overlay is not being used)
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
        for (uint8_t i=0; i<AvailableFXSlots; i++)                      // We use AvailableFXSlots here, not NUM_FX_SLOTS (though it might be the same thing if track overlay is not being used)
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

void StopSoundEffect(uint8_t slot, boolean printstop)
{   
    FX[slot].SDWav.stop();                      // Stop the file from playing
    if (!TestRoutine && printstop && FX[slot].isActive) PrintStopFx(slot);    // Debug print that we're stopping this sound
    FX[slot].isActive = false;                  // Open this slot for future sounds
    FX[slot].repeat = false;                    // Clear repeat flags
    FX[slot].repeatTimes = 0;
    FX[slot].timesRepeated = 0;
    FX[slot].specialCase = FX_SC_NONE;          // Clear any special case flag
    FX[slot].fadingOut = false;
}

void StopAllSoundEffects(void)
{
    for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
    {
        StopSoundEffect(i, false);
    }
}

void UpdateEffects(void)
{
    // For each active effect, see if the sound has stopped playing. If so, we either repeat it or update the active flag to false.
    // In certain special cases we may also decide to proceed from one sound to another
    for (uint8_t i=0; i<AvailableFXSlots; i++)                                              // We use AvailableFXSlots here, not NUM_FX_SLOTS (though it might be the same thing if track overlay is not being used)
    {   
        // Keep in mind isPlaying may return false in the first few milliseconds the file is playing. That is why we also throw in a time check. 
        if (FX[i].isActive && FX[i].SDWav.isPlaying() == false && (millis() - FX[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
        {   
            switch (FX[i].specialCase)
            {
                case FX_SC_TURRET:                                                             
                    // This special case indicates we've just finished playing the special turret rotation start sound. Now we need to proceed to the repeating portion of the turret sound
                    if (EngineRunning == false && Effect[SND_TURRET_MAN].exists) FX[i].soundFile = Effect[SND_TURRET_MAN]; // Regular turret moving sound - manual version
                    else FX[i].soundFile = Effect[SND_TURRET];                              // Regular turret moving sound
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

                case FX_SC_CANNON1:
                    Cannon_Active[0] = false;                                               // Cannon 1 sound is done playing, clear the flag
                    StopSoundEffect(i, false);                                              // Clear the special case, don't bother printing a message
                    break;

                case FX_SC_CANNON2:
                    Cannon_Active[1] = false;                                               // Cannon 2 sound is done playing, clear the flag
                    StopSoundEffect(i, false);                                              // Clear the special case, don't bother printing a message
                    break;

                case FX_SC_CANNON3:
                    Cannon_Active[2] = false;                                               // Cannon 3 sound is done playing, clear the flag
                    StopSoundEffect(i, false);                                              // Clear the special case, don't bother printing a message
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
                    
                case FX_SC_MG_STOP:
                    MG_Active[0] = false;                                                   // [0] = MG #1
                    MG_Stopping[0] = false;                                                 // We're done stopping                        
                    StopSoundEffect(i, true);
                    break;
                    
                case FX_SC_MG2:
                    // This special case indicates we've just finished playing a special second machine gun start sound. Now we need to proceed to the repeating (loop) portion of the second MG sound
                    FX[i].soundFile = Effect[SND_MG2];                                      // Now we move on to the machine gun firing sound
                    FX[i].SDWav.play(FX[i].soundFile.fileName);                             // Start playing
                    FX[i].timeStarted = millis();                                           // Record time started
                    FX[i].repeat = true;                                                    // This part is repeating
                    FX[i].repeatTimes = 0;                                                  // Set repeatTimes to 0 to repeat indefinitely
                    FX[i].specialCase = FX_SC_NONE;                                         // No special case after this (the second machine gun stop command will take care of playing any special stop sound)
                    PrintStartFx(i);
                    break;

                case FX_SC_MG2_STOP:
                    MG_Active[1] = false;                                                   // [1] = MG #2
                    MG_Stopping[1] = false;                                                 // We're done stopping
                    StopSoundEffect(i, true);
                    break;
                    
                case FX_SC_MG3:
                    // This special case indicates we've just finished playing a special third machine gun start sound. Now we need to proceed to the repeating (loop) portion of the third MG sound
                    FX[i].soundFile = Effect[SND_MG3];                                      // Now we move on to the machine gun firing sound
                    FX[i].SDWav.play(FX[i].soundFile.fileName);                             // Start playing
                    FX[i].timeStarted = millis();                                           // Record time started
                    FX[i].repeat = true;                                                    // This part is repeating
                    FX[i].repeatTimes = 0;                                                  // Set repeatTimes to 0 to repeat indefinitely
                    FX[i].specialCase = FX_SC_NONE;                                         // No special case after this (the second machine gun stop command will take care of playing any special stop sound)
                    PrintStartFx(i);
                    break;

                case FX_SC_MG3_STOP:
                    MG_Active[2]= false;                                                    // [2] = MG #3
                    MG_Stopping[2] = false;                                                 // We're done stopping                        
                    StopSoundEffect(i, true);
                    break;

                case FX_SC_SBA_NEXT:    FX[i].isActive = false; if (SoundBankA_Loop) { SoundBank_PlayNext(SOUNDBANK_A); }       break;
                case FX_SC_SBA_PREV:    FX[i].isActive = false; if (SoundBankA_Loop) { SoundBank_PlayPrevious(SOUNDBANK_A); }   break;
                case FX_SC_SBA_RAND:    FX[i].isActive = false; if (SoundBankA_Loop) { SoundBank_PlayRandom(SOUNDBANK_A); }     break;

                case FX_SC_SBB_NEXT:    FX[i].isActive = false; if (SoundBankB_Loop) { SoundBank_PlayNext(SOUNDBANK_B); }       break;
                case FX_SC_SBB_PREV:    FX[i].isActive = false; if (SoundBankB_Loop) { SoundBank_PlayPrevious(SOUNDBANK_B); }   break;
                case FX_SC_SBB_RAND:    FX[i].isActive = false; if (SoundBankB_Loop) { SoundBank_PlayRandom(SOUNDBANK_B); }     break;
                
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

// Returns the current FX slot number that is playing a given sound by file name, or -1 if not playing
int8_t GetFXSlotByFileName(_soundfile *s)
{
    for (uint8_t i=0; i<NUM_FX_SLOTS; i++)
    {
        if (FX[i].isActive && (strcmp(FX[i].soundFile.fileName,s->fileName) == 0))
        {
            return i;
        }
    }
    // Otherwise not found
    return -1;
}



// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUND EFFECTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void CannonFire(uint8_t num)
{
    switch (num)
    {
        case 1: PlaySoundEffect_wCase(Effect[SND_CANNON1], FX_SC_CANNON1); break;
        case 2: PlaySoundEffect_wCase(Effect[SND_CANNON2], FX_SC_CANNON2); break;
        case 3: PlaySoundEffect_wCase(Effect[SND_CANNON3], FX_SC_CANNON3); break;
        default: break;
    }
}

void CannonHit()
{
    PlaySoundEffect(Effect[SND_HIT_CANNON]);
}

void CannonReady()
{
    // We can have multiple cannon ready sounds, play one at random
    int index  =    GetNextCannonReadySound();                  // If no sounds exist will return -1
    if (index >= 0) PlaySoundEffect(CannonReadySound[index]);   // -1 would not be a valid item in the array, so check first
    else            return;                                     // If nothing to play, exit
}
int GetNextCannonReadySound()
{
    static int numLeft = 0;
    static int randomCannonReadyIndex[NUM_CANNON_READY_SOUNDS];
    return randomHat(randomCannonReadyIndex, NUM_CANNON_READY_SOUNDS, numLeft, CannonReadySound);
}

void MGHit()
{
    PlaySoundEffect(Effect[SND_HIT_MG]);
}

void Destroyed()
{
    PlaySoundEffect(Effect[SND_HIT_DESTROY]);
}

void LightSwitch_Sound(uint8_t num)
{
    switch(num)
    {
        case 1: PlaySoundEffect(Effect[SND_LIGHT_SWITCH1]);  break;
        case 2: PlaySoundEffect(Effect[SND_LIGHT_SWITCH2]);  break;
        case 3: PlaySoundEffect(Effect[SND_LIGHT_SWITCH3]);  break;
    }
}

void Beep(uint8_t times)
{ 
    if (times == 1) PlaySoundEffect(Effect[SND_BEEP]);
    else            RepeatSoundEffect(Effect[SND_BEEP], times);
}

void BrakeSound()
{
    PlaySoundEffect(Effect[SND_BRAKE]);
}

void PlayTransmissionEngaged(boolean engaged)
{
    engaged ? PlaySoundEffect(Effect[SND_TRANS_ENGAGE]) : PlaySoundEffect(Effect[SND_TRANS_DISENGAGE]);
}

// Machine Gun - We have three. The programming is not sophisticated, we have duplicated the following function three times, and have duplicated code in UpdateEffects() above as well to handle the termination/transitions. 
// Deal with it. Things evolve, and sometimes we're lazy. We shouldn't ever need to go beyond three. 
void MG(uint8_t  num, boolean start)
{
    switch (num)
    {
        case 1: MG1(start);     break;
        case 2: MG2(start);     break;
        case 3: MG3(start);     break;
        default:                break;
    }
}

void MG1(boolean start)
{
    static _sound_id sid;

    if (start)
    {
        if (!MG_Active[0])                                                      // Don't start if we're already playing
        {
            // There are two options for playing the machine gun sound. 
            // One involves stringing along multiple sound files, the first being a "start" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_MG_START].exists)                                    // If the special start sound exists play it first. 
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_MG flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_MG_START], sid, FX_SC_MG)) MG_Active[0] = true;
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_MG], sid)) MG_Active[0] = true;  // No start sound, go straight to repeating the loop portion 

            // Also, we're not in the stopping portion
            MG_Stopping[0] = false;         
        }
    }
    else
    {   // In this case we want to stop the machine gun sound
        if (MG_Active[0] && !MG_Stopping[0])                                    // Is the sound even active? And not already stopping? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_MG_STOP].exists)                                     // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing MG sound
                MG_Stopping[0] = true;                                          // Now we're starting the stopping sound
                FX[sid.Slot].soundFile = Effect[SND_MG_STOP];                   // Set the soundfile to the special MG stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_MG_STOP;                       // Special case - so the update routine can clear the MG_Active flag when this sound is done
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the MG stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid.Slot, false);                               // In this case we don't have a separate stop sound, we can just immediately stop
                MG_Active[0] = false; 
                MG_Stopping[0] = false;                 
            }
        }
    }
}

void MG2(boolean start)
{
    static _sound_id sid;

    if (start)
    {
        if (!MG_Active[1])                                                      // Don't start if we're already playing
        {
            // There are two options for playing the second machine gun sound. 
            // One involves stringing along multiple sound files, the first being a "start" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_MG2_START].exists)                                   // If the special start sound exists play it first. 
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_MG2 flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_MG2_START], sid, FX_SC_MG2)) MG_Active[1] = true;   
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_MG2], sid)) MG_Active[1] = true;  // No start sound, go straight to repeating the loop portion 

            // Also, we're not in the stopping portion
            MG_Stopping[1] = false; 
        }
    }
    else
    {   // In this case we want to stop the machine gun sound
        if (MG_Active[1] && !MG_Stopping[1])                                    // Is the sound even active? And not already stopping? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_MG2_STOP].exists)                                    // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing MG sound
                MG_Stopping[1] = true;                                          // Now we're starting the stopping sound
                FX[sid.Slot].soundFile = Effect[SND_MG2_STOP];                  // Set the soundfile to the special MG stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_MG2_STOP;                      // Special case - so the update routine can clear the MG2_Active flag when this sound is done
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the MG stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid.Slot, false);                               // In this case we don't have a separate stop sound, we can just immediately stop
                MG_Active[1] = false;
                MG_Stopping[1] = false; 
            }
        }
    }
}

void MG3(boolean start)
{
    static _sound_id sid;

    if (start)
    {
        if (!MG_Active[2])                                                      // Don't start if we're already playing
        {
            // There are two options for playing the third machine gun sound. 
            // One involves stringing along multiple sound files, the first being a "start" sound, followed by the repeating (looping) portion of the movement. 
            // We can only kick off a single sound here, which will be the start sound. We will leave it to UpdateEffects() to move to the second portion if necessary. 
            if (Effect[SND_MG3_START].exists)                                   // If the special start sound exists play it first. 
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_MG3 flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_MG3_START], sid, FX_SC_MG3)) MG_Active[2] = true;   
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (RepeatSoundEffect_wOptions(Effect[SND_MG3], sid)) MG_Active[2] = true;  // No start sound, go straight to repeating the loop portion 

            // Also, we're not in the stopping portion
            MG_Stopping[2] = false; 
        }
    }
    else
    {   // In this case we want to stop the machine gun sound
        if (MG_Active[2] && !MG_Stopping[2])                                    // Is the sound even active? And not already stopping? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if (Effect[SND_MG3_STOP].exists)                                    // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing MG sound
                MG_Stopping[2] = true;                                          // Now we're starting the stopping sound
                FX[sid.Slot].soundFile = Effect[SND_MG3_STOP];                  // Set the soundfile to the special MG stop sound instead
                FX[sid.Slot].isActive = true;                                   // Keep this slot active since we are playing a new sound with it. 
                FX[sid.Slot].repeat = false;                                    // The stop sound does not repeat
                FX[sid.Slot].specialCase = FX_SC_MG3_STOP;                      // Special case - so the update routine can clear the MG3_Active flag when this sound is done
                FX[sid.Slot].SDWav.play(FX[sid.Slot].soundFile.fileName);       // Start playing the MG stop sound
                FX[sid.Slot].timeStarted = millis();                            // Save the time that we begin playing this sound
                PrintStartFx(sid.Slot);
            }
            else
            {
                StopSoundEffect(sid.Slot, false);                               // In this case we don't have a separate stop sound, we can just immediately stop
                MG_Active[2] = false; 
                MG_Stopping[2] = false;                 
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
            if (EngineRunning == false && Effect[SND_TURRET_START_MAN].exists)  // If engine is off and this sound exists, play it first
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_TURRET flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_TURRET_START_MAN], sid, FX_SC_TURRET)) active = true;                   
            }
            else if (Effect[SND_TURRET_START].exists)                           // Else if the engine is on or if the manual start doesn't exist, play the regular start sound first if it exists
            {
                // We pass a false flag meaning don't repeat this portion. We also pass the FX_SC_TURRET flag which will get saved with the playing file. 
                // UpdateEffects() will identify this flag when the start sound is done playing, and will then initiate the repeating portion
                if (PlaySoundEffect_wOptions(Effect[SND_TURRET_START], sid, FX_SC_TURRET)) active = true;   
            }
            // The other option for playing the turret sound is just to start directly with the looping portion, which is what we do if no start sound is found. 
            else if (EngineRunning == false && Effect[SND_TURRET_MAN].exists)
            {
                 if (RepeatSoundEffect_wOptions(Effect[SND_TURRET_MAN], sid)) active = true;  // No start sound, go straight to repeating the loop portion (engine off/manual version)
            }
            else if (RepeatSoundEffect_wOptions(Effect[SND_TURRET], sid)) active = true;  // No start sound, go straight to repeating the loop portion 
        }
    }
    else
    {   // In this case we want to stop the turret sound
        if (active)                                                             // Is the sound even active? 
        {
            // Here again we have two options for stopping. First we stop the repeating sound that is currently playing. 
            // Then, if a special stop sound exists, we play that. Otherwise that's it, we're done
            if ((EngineRunning == false && Effect[SND_TURRET_STOP_MAN].exists) || Effect[SND_TURRET_STOP].exists)    // Special stop sound exists
            {
                if (FX[sid.Slot].ID.Num == sid.Num) FX[sid.Slot].SDWav.stop();  // Stop the existing turret running sound
                if (EngineRunning) FX[sid.Slot].soundFile = Effect[SND_TURRET_STOP];// Set the soundfile to the special turret stop sound instead
                else FX[sid.Slot].soundFile = Effect[SND_TURRET_STOP_MAN];          // - manual version
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
                StopSoundEffect(sid.Slot, true);                                // In this case we don't have a separate stop sound, we can just immediately stop
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
                StopSoundEffect(sid.Slot, true);                                // In this case we don't have a separate stop sound, we can just immediately stop
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
        if (!active)                                                            // No need to start if it is already going
        {
            if (RepeatSoundEffect_wOptions(Effect[SND_REPAIR], RP_SID))         // Set the repeat flag for the Repair sound
            {
                active = true;
            }
        }
    }
    else                                                                        // In this case we want to stop the Repair sound
    {   
        if (active)                                                             // Is the sound even active? 
        {
            StopSoundEffect(RP_SID.Slot, true);                                 // Stop the sound
            active = false;                                                     // Update our internal flag
        }
    }
}



// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SQUEAKS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Used only in serial mode - if an INI file is present, the squeakInfo variable will be written directly
void SetSqueakMin(uint8_t val, uint8_t squeakNum)                       // Set squeak minimum interval time
{
    uint16_t minVal; 
    
    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        minVal = ((val * 50) + 500);                                    // Valid values are 0-190 which equate to intervals from 500mS to 10000mS (1/2 to 10 seconds)
        squeakInfo[squeakNum].intervalMin = minVal;                     // Update our variable in RAM
    }
}
// Used only in serial mode - if an INI file is present, the squeakInfo variable will be written directly
void SetSqueakMax(uint8_t val, uint8_t squeakNum)                       // Set squeak maximum interval time
{
    uint16_t maxVal; 
    
    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        maxVal = ((val * 50) + 500);                                    // Valid values are 0-190 which equate to intervals from 500mS to 10000mS (1/2 to 10 seconds)
        squeakInfo[squeakNum].intervalMax = maxVal;                     // Update our variable in RAM
    }
}
// Used only in serial mode - if an INI file is present, the squeakInfo variable will be written directly
void EnableSqueak(uint8_t val, uint8_t squeakNum)
{   // This is for enabling the squeak or not, which is a user setting. It is not the same thing
    // as active/inactive (that determines if the squeak is squeaking or just waiting to squeak).
    // If a squeak is disabled, the sound won't play. 

    // Make sure the squeak number is valid
    if (squeakNum > 0 && squeakNum <= OPSC_MAX_NUM_SQUEAKS)
    {   
        squeakNum -= 1;                                                 // Subtract 1 from number because array is zero-based
        squeakInfo[squeakNum].enabled = (boolean)val;                   // Update our variable in RAM
        AnySqueakEnabled = true;                                        // Yes, we now have an enabled squeak
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
            if (squeakInfo[i].enabled && !squeakInfo[i].active && Effect[SND_SQUEAK_OFFSET+i].exists) 
            { 
                squeakInfo[i].active = true;            // Activate this squeak 
                AssignRandomTimeToSqueak(i);            // Rather than play it now, which would cause them all to go off at once, assign it a random time in the future. 
                                                        // The UpdateSqueaks() function will squeak them when the time is up. 
            }
        }
        if (DEBUG && AnySqueakEnabled) { DebugSerial.println(F("Start Squeaks")); }
    }
}

void StopSqueaks(void)
{
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        squeakInfo[i].active = false;
    }
    AllSqueaks_Active = false;
    if (timer.isEnabled(SqueakTimerID)) timer.deleteTimer(SqueakTimerID);
    if (DEBUG && AnySqueakEnabled) { DebugSerial.println(F("Stop Squeaks")); }
}

void UpdateSqueaks(void)
{
    // If the random length of time has passed for a given squeak, squeak it again
    // This function needs to be polled routinely from the main loop. 
    
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        if (squeakInfo[i].active && (millis() - squeakInfo[i].lastSqueak) >= squeakInfo[i].squeakAfter) PlaySqueak(i);
    }
}

void PlaySqueak(uint8_t i)
{
    if (squeakInfo[i].active)
    {
        PlaySoundEffect(Effect[SND_SQUEAK_OFFSET + i]);         // Play the squeak sound
        AssignRandomTimeToSqueak(i);        // Assign a random time before we squeak again
        PrintNextSqueakTime(i);             // For debugging
    }
}

void AssignRandomTimeToSqueak(uint8_t i)
{
    squeakInfo[i].lastSqueak = millis();                                                        // Save the current time so we can compare to it later
    squeakInfo[i].squeakAfter = random(squeakInfo[i].intervalMin,squeakInfo[i].intervalMax);    // Calculate a random amount of time within the min and max squeak interval    
}




