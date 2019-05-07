
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// USER SOUNDS - ACCESSED DIRECTLY, PLAYED SINGLY
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void PlayUserSound(uint8_t n, boolean startSound, boolean repeat)
{
static _sound_id US_SID[NUM_USER_SOUNDS];
int8_t slotNum;     // Need signed

    // Only accept valid user sounds
    if (n > NUM_USER_SOUNDS) return;

    // We will be passed a user sound "n" from 1 to NUM_USER_SOUNDS
    // But we want to use this number to access elements of a zero-based array, so let's subtract 1
    n -= 1;

    // Before we start, let's see if this user sound is currently playing in any slot. slotNum will be -1 if not, or the slot number if so
    slotNum = GetFXSlotByFileName(&UserSound[n]);               

    // Now play, repeat or stop
    if (startSound)                                                             // In this case we want to start the sound
    {
        if (slotNum < 0)                                                        // No need to start/repeat if it is already playing
        {
            if (repeat) RepeatSoundEffect_wOptions(UserSound[n], US_SID[n]);    // Start repeating
            else        PlaySoundEffect_wOptions(UserSound[n], US_SID[n]);      // Start playing
        }
    }
    else if (slotNum >= 0) StopSoundEffect(slotNum, true);                      // Stop the sound
}

void StopAllUserSounds()
{
    for (uint8_t i=0; i<NUM_USER_SOUNDS; i++)
    {
        PlayUserSound(i+1, false, false);   // i + 1 because PlayUserSound expects the numbers to be 1 - NUM_USER_SOUNDS, not zero-based
        // The second argument is set to "false" meaning stop playing the sound
        // PlayUserSound will do the checking to see if the file is even playing or not, and if so, will stop it
    }
}


// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SOUND BANK - COLLECTION OF FILES THAT CAN BE PLAYED IN A LIST FORMAT (next, previous, loop through all, etc)
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void SoundBank_PlayToggle(soundbank SB)
{
    // If there is a sound playing, stop it
    if (SoundBank_Stop(SB)) return;

    // Otherwise if there is no sound playing, start it
    switch (SB)
    {
        case SOUNDBANK_A: 
            if (!SoundBankAExists) return;
            PlaySoundEffect_wOptions(SoundBankA[SoundBankA_CurrentIndex], SoundBankA_CurrentID, SBA_LastDirection);
            SBA_Started = true;                         // Sound Bank A has hence-forth been played
            break;
        
        case SOUNDBANK_B: 
            if (!SoundBankBExists) return;              
            PlaySoundEffect_wOptions(SoundBankB[SoundBankB_CurrentIndex], SoundBankB_CurrentID, SBB_LastDirection);
            SBB_Started = true;                         // Sound Bank B has hence-forth been played
            break;
        
        default: return;
    }
}

void SoundBank_PlayNext(soundbank SB)
{
    int currIndex;
    uint8_t numSounds;
    uint8_t startPos;
    _soundfile *s;
    boolean started;
    
    switch (SB)
    {
        case SOUNDBANK_A: 
            if (!SoundBankAExists) return;              // Do nothing if we have no sounds in this bank
            currIndex = SoundBankA_CurrentIndex;
            numSounds = NUM_SOUNDS_BANK_A;
            s = SoundBankA;
            started = SBA_Started;                      // Save the started variable
            if (!SBA_Started) SBA_Started = true;       // Sound Bank A has hence-forth been played
            break;
            
        case SOUNDBANK_B: 
            if (!SoundBankBExists) return;              // Do nothing if we have no sounds in this bank
            currIndex = SoundBankB_CurrentIndex;
            numSounds = NUM_SOUNDS_BANK_B;
            s = SoundBankB;
            started = SBB_Started;                      // Save the started variable
            if (!SBB_Started) SBB_Started = true;       // Sound Bank B has hence-forth been played
            break;
        
        default: return;            
    }

    // First, if there is a sound already playing from this bank, stop it
    SoundBank_Stop(SB);

    // Increment to next index
    if (started == false && currIndex == 0) { startPos = 0; }   // If this is the first time calling the next routine, and we are on zero, start on zero instead of incrementing this time
    else
    {
        if (currIndex >= (numSounds - 1)) startPos = 0;
        else                              startPos = currIndex + 1;
    }

    // Now try playing, if the next one doesn't exist, keep incrementing until we find the next sound that does exist
    for (uint8_t i=startPos; i<numSounds; i++)
    {
        if (s[i].exists) 
        { 
            switch (SB)
            {   case SOUNDBANK_A:   
                    PlaySoundEffect_wOptions(SoundBankA[i], SoundBankA_CurrentID, FX_SC_SBA_NEXT); SoundBankA_CurrentIndex = i; SBA_LastDirection = FX_SC_SBA_NEXT; return;
                    break;
                case SOUNDBANK_B:   
                    PlaySoundEffect_wOptions(SoundBankB[i], SoundBankB_CurrentID, FX_SC_SBB_NEXT); SoundBankB_CurrentIndex = i; SBB_LastDirection = FX_SC_SBB_NEXT; return;
                    break;   
            }
        }
    }

    // If we make it to here there wasn't another file found after, so loop back to the beginning and check from the start-to-before
    for (uint8_t i=0; i<currIndex; i++)
    {
        if (s[i].exists)
        {
            switch (SB)
            {   case SOUNDBANK_A:   
                    PlaySoundEffect_wOptions(SoundBankA[i], SoundBankA_CurrentID, FX_SC_SBA_NEXT); SoundBankA_CurrentIndex = i; SBA_LastDirection = FX_SC_SBA_NEXT; return;
                    break;
                case SOUNDBANK_B:   
                    PlaySoundEffect_wOptions(SoundBankB[i], SoundBankB_CurrentID, FX_SC_SBB_NEXT); SoundBankB_CurrentIndex = i; SBB_LastDirection = FX_SC_SBB_NEXT; return;
                    break;   
            }
        }
    }

    // This should not happen, but if it does, give us a message
    DebugSerial.println(F("Error: Next sound bank file not found")); 
}

void SoundBank_PlayPrevious(soundbank SB)
{
    int currIndex;
    uint8_t numSounds;
    uint8_t startPos;
    _soundfile *s;
    
    switch (SB)
    {
        case SOUNDBANK_A: 
            if (!SoundBankAExists) return;              // Do nothing if we have no sounds in this bank
            currIndex = SoundBankA_CurrentIndex;
            numSounds = NUM_SOUNDS_BANK_A;
            s = SoundBankA;
            SBA_Started = true;                         // Sound Bank A has hence-forth been played
            break;
            
        case SOUNDBANK_B: 
            if (!SoundBankBExists) return;              // Do nothing if we have no sounds in this bank
            currIndex = SoundBankB_CurrentIndex;
            numSounds = NUM_SOUNDS_BANK_B;
            s = SoundBankB;
            SBB_Started = true;                         // Sound Bank B has hence-forth been played
            break;

        default: return;
    }

    // First, if there is a sound already playing from this bank, stop it
    SoundBank_Stop(SB);

    // Decrement to prior index
    if (currIndex > 0) startPos = currIndex - 1;
    else               startPos = numSounds - 1;

    // Now try playing, if the previous one doesn't exist, keep decrementing until we find the previous sound that does exist
    for (uint8_t i=startPos; i>=0; i--)
    {
        if (s[i].exists) 
        {
            switch (SB)
            {   case SOUNDBANK_A:   
                    PlaySoundEffect_wOptions(SoundBankA[i], SoundBankA_CurrentID, FX_SC_SBA_PREV); SoundBankA_CurrentIndex = i; SBA_LastDirection = FX_SC_SBA_PREV; return;
                    break;
                case SOUNDBANK_B:   
                    PlaySoundEffect_wOptions(SoundBankB[i], SoundBankB_CurrentID, FX_SC_SBB_PREV); SoundBankB_CurrentIndex = i; SBB_LastDirection = FX_SC_SBB_PREV; return;
                    break;   
            }      
        }
    }

    // If we make it to here there wasn't another file found earlier, so loop back to the very end and check from the end-to-last
    startPos = numSounds - 1;      // End
    for (uint8_t i=startPos; i>=currIndex; i--)
    {
        if (s[i].exists)
        {
            switch (SB)
            {   case SOUNDBANK_A:   
                    PlaySoundEffect_wOptions(SoundBankA[i], SoundBankA_CurrentID, FX_SC_SBA_PREV); SoundBankA_CurrentIndex = i; SBA_LastDirection = FX_SC_SBA_PREV; return;
                    break;
                case SOUNDBANK_B:   
                    PlaySoundEffect_wOptions(SoundBankB[i], SoundBankB_CurrentID, FX_SC_SBB_PREV); SoundBankB_CurrentIndex = i; SBB_LastDirection = FX_SC_SBB_PREV; return;
                    break;   
            }
        }
    }

    // This should not happen, but if it does, give us a message
    DebugSerial.println(F("Error: Previous sound bank file not found")); 
}

void SoundBank_PlayRandom(soundbank SB)
{
    // First, if there is a sound already playing from this bank, stop it
    SoundBank_Stop(SB); 
    switch (SB)
    {
        case SOUNDBANK_A:   
            SoundBankA_CurrentIndex = SoundBankA_GetRandom();
            PlaySoundEffect_wOptions(SoundBankA[SoundBankA_CurrentIndex], SoundBankA_CurrentID, FX_SC_SBA_RAND);  
            SBA_LastDirection = FX_SC_SBA_RAND;   
            SBA_Started = true;
            break;
        
        case SOUNDBANK_B:   
            SoundBankB_CurrentIndex = SoundBankB_GetRandom();
            PlaySoundEffect_wOptions(SoundBankB[SoundBankB_CurrentIndex], SoundBankB_CurrentID, FX_SC_SBB_RAND);     
            SBB_LastDirection = FX_SC_SBB_RAND;
            SBB_Started = true;
            break;
            
        default: return;
    }    
}

int SoundBankA_GetRandom(void)
{
    static int numLeft = 0;                     // Number of items left, start at 0 so randomHat will initialize index
    static int randomBankASoundIndex[NUM_SOUNDS_BANK_A];

    return randomHat(randomBankASoundIndex, NUM_SOUNDS_BANK_A, numLeft, SoundBankA);
}

int SoundBankB_GetRandom(void)
{
    static int numLeft = 0;                     // Number of items left, start at 0 so randomHat will initialize index
    static int randomBankBSoundIndex[NUM_SOUNDS_BANK_B];

    return randomHat(randomBankBSoundIndex, NUM_SOUNDS_BANK_B, numLeft, SoundBankB);
}

boolean SoundBank_Stop(soundbank SB)
{
    _sound_id sid;
    switch (SB)
    {
        case SOUNDBANK_A: sid = SoundBankA_CurrentID;   break;
        case SOUNDBANK_B: sid = SoundBankB_CurrentID;   break;
        default: return false;
    }

    if (sid.Slot >= 0)
    {   
        for (uint8_t i=0; i<AvailableFXSlots; i++)
        {
            if (FX[i].isActive && FX[i].ID.Num == sid.Num && FX[i].ID.Slot == sid.Slot)
            {   
                StopSoundEffect(FX[i].ID.Slot, false);
                sid.Num = 0;
                sid.Slot = -1;
                return true;
            }
        }
    }

    return false;
}

boolean SoundBank_AnyExist(soundbank SB)
{
    uint8_t num;
    _soundfile *s;
    
    switch (SB)
    {
        case SOUNDBANK_A:   num = NUM_SOUNDS_BANK_A;   s = SoundBankA;  break;
        case SOUNDBANK_B:   num = NUM_SOUNDS_BANK_B;   s = SoundBankB;  break;
        default: return false;
    }

    for (uint8_t i=0; i<num; i++)
    {
        if (s[i].exists) return true;
    }
    return false;
}





