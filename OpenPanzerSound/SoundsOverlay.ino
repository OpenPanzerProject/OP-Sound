
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// TRACK OVERLAY SOUNDS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Track overlay sounds behave like the engine sounds, in that there are X amount possible, and they are played in order up and down the list 
// depending on the vehicle speed. However, unlike the two reserved engine sounds slots, we use two FX slots for these sounds. If track overlay 
// is enabled (sounds present), two of the total FX slots are removed from the effects list and dedicated solely to the track overlay effect

// Typically Arduino creates function prototypes for all functions in the sketch, but if we want to use default arguments we have to specify our own. 
boolean FadeOverlaySound(_soundfile &s, _track_transition trackTransition=TO_TR_NONE);
boolean FadeOverlaySound(_soundfile &s, _track_transition trackTransition)
{
    return PlayOverlaySound_wOptions(s, false, true, trackTransition);
}

boolean FadeRepeatOverlaySound(_soundfile &s, _track_transition trackTransition=TO_TR_NONE);
boolean FadeRepeatOverlaySound(_soundfile &s, _track_transition trackTransition)
{
    return PlayOverlaySound_wOptions(s, true, true, trackTransition);
}

boolean PlayOverlaySound(_soundfile &s, _track_transition trackTransition=TO_TR_NONE);
boolean PlayOverlaySound(_soundfile &s, _track_transition trackTransition)
{
    return PlayOverlaySound_wOptions(s, false, false, trackTransition);
}

boolean RepeatOverlaySound(_soundfile &s, _track_transition trackTransition=TO_TR_NONE);
boolean RepeatOverlaySound(_soundfile &s, _track_transition trackTransition)
{
    return PlayOverlaySound_wOptions(s, true, false, trackTransition);
}

boolean PlayOverlaySound_wOptions(_soundfile &s, boolean repeat, boolean Fade, _track_transition trackTransition=TO_TR_NONE);
boolean PlayOverlaySound_wOptions(_soundfile &s, boolean repeat, boolean Fade, _track_transition trackTransition)
{
    // First, check if the requested sound even exists
    if (!s.exists) return false; 

    TOCurrent = TONext;
    FX[TOCurrent].SDWav.stop();
    FX[TOCurrent].soundFile = s;                                        // Assign sound
    FX[TOCurrent].isActive = true;                                      // Set active flag
    FX[TOCurrent].repeat = repeat;                                      // Pass any repeat flag
    FX[TOCurrent].specialCase = trackTransition;                        // Pass any special case flag
    if (Fade) OverlayFader[TOCurrent - FIRST_OVERLAY_SLOT].fadeIn(TRACK_OVERLAY_FADE_TIME_MS); // Fade in if requested (we have to compensate by first slot since OverlayFader will be 0/1 but TOCurrent will be 2/3)
    else OverlayFader[TOCurrent - FIRST_OVERLAY_SLOT].fadeIn(1);        // Instant fade-in otherwise. When we have left this off we have found problems with the sound having zero volume
    FX[TOCurrent].SDWav.play(FX[TOCurrent].soundFile.fileName);         // Start playing
    FX[TOCurrent].timeStarted = millis();                               // Save the time that we begin playing this sound
    FX[TOCurrent].ID.Num++;                                             // Increment the unique ID number of this sound
    FX[TOCurrent].ID.Slot = TOCurrent;                                  // Save the slot
    PrintStartFx(TOCurrent);                                            // For debugging only

    SwapOverlaySlot();                                                  // Swap track overlay sound banks
    
    if (Fade && FX[TONext].SDWav.isPlaying())                           // We are now at the prior sound, which we want to free up for the next sound
    { 
        OverlayFader[TONext - FIRST_OVERLAY_SLOT].fadeOut(TRACK_OVERLAY_FADE_TIME_MS); // Fade out prior sound if it is still playing (we have to compensate by first slot since OverlayFader will be 0/1 but TONext will be 2/3)
        FX[TONext].timeWillFadeOut = (millis() + TRACK_OVERLAY_FADE_TIME_MS);    // When will this sound be completely faded out
        FX[TONext].fadingOut = true;                                    // Flag this sound as fading-out
        FX[TONext].repeat = false;                                      // We don't want that prior sound to repeat
        FX[TONext].specialCase = TO_TR_NONE;                            // Clear the special case from this sound because it is ending.
    }
    
    return true;
}

void SwapOverlaySlot(void)
{
    if (TONext == FIRST_OVERLAY_SLOT) TONext = FIRST_OVERLAY_SLOT + 1;  // Update TONext to the next slot
    else                              TONext = FIRST_OVERLAY_SLOT;      // We toggle between the last two FX slots
}

void SetVehicleSpeed(uint8_t speed)
{
    // Set our global vehicle speed variable
    VehicleSpeed = speed;    

    // Are we moving?
    VehicleSpeed > 0 ? VehicleMoving = true : VehicleMoving = false;
}

void UpdateVehicle(void)
{
static boolean wereMoving = false;

    if (!TrackOverlayEnabled) return;                                                                       // If track overlay is not enabled (inadequate sounds present), do nothing

    // We've started moving
    if (wereMoving == false && VehicleMoving == true)
    {  
        AvailableFXSlots = NUM_FX_SLOTS - NUM_TRACK_OVERLAY_SLOTS;                                          // Reserve the last 2 FX slots exclusively for track overlay sounds
        StopAllOverlaySounds();                                                                             // Start with overlay sound slots stopped, in case there were some other effects playing in those
        if (TrackOverlayStart.exists)   PlayOverlaySound(TrackOverlayStart, TO_TR_START_MOVING);            // Play the start sound and transition to moving after
        else FadeRepeatOverlaySound(TrackOverlaySound[GetOverlaySoundNumBySpeed()], TO_TR_KEEP_MOVING);     // If not, go directly to repeating the track overlay sound associated with our current vehicle speed
        wereMoving = true;
    }

    // We've come to a stop
    if (wereMoving == true && VehicleMoving == false)
    {
        if (TrackOverlayStop.exists)
        {
            FadeOverlaySound(TrackOverlayStop, TO_TR_STOP_MOVING);                                          // Fade into the stop sound if we have one
        }
        else 
        {   // This would normally be done after the stop sound is done playing (see below), but in this case just do it directly
            StopAllOverlaySounds();                                                                         // Otherwise just stop
            AvailableFXSlots = NUM_FX_SLOTS;                                                                // Release these for use by regular FX
        }
        wereMoving = false;
    }
}

void UpdateIndividualOverlaySounds(void)
{
uint8_t          OverlaySoundNum;
static uint8_t   LastOverlaySoundNum = 0; 

    // For each active FX slot, see if the sound has stopped playing. If so, we either repeat it or update the active flag to false.
    // We may also take different actions based on the specialCase flag
    for (uint8_t i=FIRST_OVERLAY_SLOT; i<NUM_FX_SLOTS; i++)
    {   // Keep in mind isPlaying may return false in the first few milliseconds the file is playing. That is why we also throw in a time check. 
        if (FX[i].isActive)
        {   
            switch (FX[i].specialCase)
            {
                case TO_TR_START_MOVING: 
                    // Here we transition from the track start sound to the moving sound by fading. 
                    // We wait until the start sound is nearly done playing
                    if (FX[i].SDWav.isPlaying() && (FX[i].SDWav.positionMillis() >= (FX[i].soundFile.length - TRACK_OVERLAY_FADE_TIME_MS)))
                    {   
                        FadeRepeatOverlaySound(TrackOverlaySound[GetOverlaySoundNumBySpeed()],TO_TR_KEEP_MOVING);    // Fade in and start repeating
                    }
                    break;
                
                case TO_TR_KEEP_MOVING:
                    // Here we are moving and we need to decide whether we should repeat the current overlay sound, or fade to a higher or lower overlay sound (speed change)
                    OverlaySoundNum = GetOverlaySoundNumBySpeed();
                    if (OverlaySoundNum != LastOverlaySoundNum)
                    {   // We need to transition to a new overlay sound. We don't wait for the previous one to finish.
                        FadeRepeatOverlaySound(TrackOverlaySound[OverlaySoundNum], TO_TR_KEEP_MOVING);    // Fade in to the new overlay sound and start repeating it
                        LastOverlaySoundNum = OverlaySoundNum;
                    }
                    else
                    {   // We are at the same speed (or within the same speed range encompassed by this sound). Repeat the sound if it is over. 
                        if (FX[i].SDWav.isPlaying() == false && (millis() - FX[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                        {
                            FX[i].SDWav.play(FX[i].soundFile.fileName);             // Restart
                            FX[i].timeStarted = millis();                           // All the prior flags can be left the same, but this one needs updated
                        }                            
                    }
                    break;

                case TO_TR_STOP_MOVING:
                    // Wait until stop sound is done playing
                    if (FX[i].SDWav.isPlaying() == false && (millis() - FX[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                    {
                        StopAllOverlaySounds();                                     // Stop all overlay sounds, we're done
                        AvailableFXSlots = NUM_FX_SLOTS;                            // Release these for use by regular FX                        
                    }
                    break; 
                    
                case TO_TR_NONE:    // No transition, just decide if a sound needs to be repeated or set to inactive. 
                    // Check if sound is done playing. We already know it is flagged as active, so if it is done playing we need to update the flag to inactive, or set it to repeat if required
                    if (FX[i].SDWav.isPlaying() == false && (millis() - FX[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                    {
                        if (FX[i].repeat == true)   
                        {
                            FX[i].SDWav.play(FX[i].soundFile.fileName);                 // Restart
                            FX[i].timeStarted = millis();                               // All the prior flags can be left the same, but this one needs updated
                        }
                        else FX[i].isActive = false;                                    // Otherwise clear the active flag and open this slot for future sounds
                    }

                    // Another time we might want to stop a sound playing is if we had faded it out, and the fade time is expired. If we don't do anything, the sound will continue to play 
                    // until the full length of the clip has been processed, but beyond the fade out time we will just be playing static. So to avoid the static portion, we stop the sound
                    // if the fade-out is complete. 
                    if (FX[i].SDWav.isPlaying() && FX[i].isActive && FX[i].fadingOut && (millis() - FX[i].timeWillFadeOut > 0))
                    {
                        FX[i].SDWav.stop();
                        FX[i].isActive = false;
                        FX[i].fadingOut = false;
                    }
                    break;
            }
        }
    }
}

uint8_t GetOverlaySoundNumBySpeed()
{
    return map(VehicleSpeed, 0, 255, 0, (NumOverlaySounds - 1)); // Subtract 1 because overlay sound array is zero-based
}

void StopAllOverlaySounds(void)
{
    for (uint8_t i=FIRST_OVERLAY_SLOT; i<NUM_FX_SLOTS; i++)
    {
        StopSoundEffect(i);
    }
    TONext = FIRST_OVERLAY_SLOT; // Reset the next overlay to the first overlay slot
}




