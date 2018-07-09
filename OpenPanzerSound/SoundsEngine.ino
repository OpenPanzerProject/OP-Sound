
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// ENGINE SOUNDS
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// Typically Arduino creates function prototypes for all functions in the sketch, but if we want to use default arguments we have to specify our own. 
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
    if (Fade) EngineFader[EnCurrent].fadeIn(ENGINE_FADE_TIME_MS);           // Fade in if requested
    else      EngineFader[EnCurrent].fadeIn(1);                             // Instant fade-in otherwise
    Engine[EnCurrent].SDWav.play(Engine[EnCurrent].soundFile.fileName);     // Start playing
    Engine[EnCurrent].timeStarted = millis();                               // Save the time that we begin playing this sound
    Engine[EnCurrent].ID.Num++;                                             // Increment the unique ID number of this sound
    Engine[EnCurrent].ID.Slot = EnCurrent;                                  // Save the slot
    PrintStartEngineSound(EnCurrent);                                       // For debugging only

    SwapEngineSlot();                                                       // Swap engine sound banks
    
    if (Fade && Engine[EnNext].SDWav.isPlaying())                           // We are now at the prior sound, which we want to free up for the next sound
    { 
        EngineFader[EnNext].fadeOut(ENGINE_FADE_TIME_MS);                   // Fade out prior sound if it is still playing
        Engine[EnNext].timeWillFadeOut = (millis() + ENGINE_FADE_TIME_MS);  // When will this sound be completely faded out
        Engine[EnNext].fadingOut = true;                                    // Flag this sound as fading-out
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

    // We were at idle, but are no longer, or weren't but now are - handle the auto stop at idle timer
    if (EngineState_Prior != EngineState) 
    {
        if (EngineState_Prior == ES_IDLE)   ClearAutoStopAtIdleTimer();
        if (EngineState == ES_IDLE)         StartAutoStopAtIdleTimer();
        if (EngineState == ES_START)        DebugSerial.println(F("Start Engine")); 
        if (EngineState == ES_SHUTDOWN)     DebugSerial.println(F("Shutdown Engine"));
    }
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

    // If we are in RC mode, and the engine is off, and engine autostart setting is set to true, and the throttle speed has increased beyond idle, then automatically start the engine
    if ((InputMode == INPUT_RC) && EngineEnabled && Engine_AutoStart && (EngineState == ES_OFF) && (EngineSpeed > throttleHysterisisRC))
    {
        StartEngine();
        return;
    }
        
    // Otherwise, ignore the speed setting if we are not already enabled and running
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
        // But if we hadn't even got to run yet, skip the decel and go straight to idle
        else if (EngineState == ES_ACCEL || EngineState == ES_ACCEL_WAIT) SetEngineState(ES_START_IDLE);
    }
}

void UpdateEngine(void)
{
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
            // If we are at idle, UpdateIndividualEngineSounds() will automatically repeat the idle sound for as long as needed. 
            break;

        case ES_START_IDLE:
            // If we are not yet at idle, start it
            if (VehicleDamaged && EngineDamagedIdle.exists)                                 // This takes care of damaged idle, if it exists and if we are damaged
            {
                FadeRepeatEngineSound(EngineDamagedIdle);
            }
            else if (GetNextSound(IdleSound, nextIdleSound, NUM_SOUNDS_IDLE))               // This should definitely return true because engine is only enabled if we have at least one idle sound
            {
                FadeRepeatEngineSound(IdleSound[nextIdleSound]);                            // Fade in idle sound and start repeating it
            }

            // Change state to idle
            SetEngineState(ES_IDLE);            
            break;

        case ES_ACCEL:
            // Here we are just starting to move, so play the accel sound if we have one
            if (GetNextSound(AccelSound, nextAccelSound, NUM_SOUNDS_ACCEL))                 // Get next accel sound of the various we may have available to us
            {
                FadeEngineSound(AccelSound[nextAccelSound], EN_TR_ACCEL_RUN);               // Transition to run after accel when done
                SetEngineState(ES_ACCEL_WAIT);                                              // Don't go to run quite yet
            }
            else
            {                                                                               // If we don't have an accel sound, go straight to run
                FadeRepeatEngineSound(RunSound[GetRunSoundNumBySpeed()], EN_TR_RUN);        // If no accel sound available, fade straight into run sound and start repeating it when done
                SetEngineState(ES_RUN);                                                     // Set new engine state
            }
            break;

        case ES_ACCEL_WAIT: 
            // Don't need to do anything here. When the accel sound is finished playing, UpdateIndividualEngineSounds below will automatically change the engine state to RUN
            break;

        case ES_DECEL:
            // Here we are already moving, but we want to stop (return to idle). 
            if (GetNextSound(DecelSound, nextDecelSound, NUM_SOUNDS_DECEL))                 // Get next Decel sound of the various we may have available to us, only if we were in Run! (Not Accel)
            {
                FadeEngineSound(DecelSound[nextDecelSound], EN_TR_IDLE);                    // Transition to idle after decel when done
            }
            else                                                                            // If no Decel sound, or if we hadn't made it to run yet, go straight to idle
            {                                                                               // Get next idle sound of the various we may have available to us
                if (VehicleDamaged && EngineDamagedIdle.exists)                             // This takes care of damaged idle, if it exists and if we are damaged
                {
                    FadeRepeatEngineSound(EngineDamagedIdle);
                }
                else if (GetNextSound(IdleSound, nextIdleSound, NUM_SOUNDS_IDLE))           // This should definitely return true because engine is only enabled if we have at least one idle sound
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
            FadeEngineSound(EngineShutoff, EN_TR_TURN_OFF);                                 // Fade in the shutdown sound
            SetEngineState(ES_OFF);
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
                        if (VehicleDamaged && EngineDamagedIdle.exists)                     // This takes care of damaged idle, if it exists and if we are damaged
                        {
                            FadeRepeatEngineSound(EngineDamagedIdle);
                            SetEngineState(ES_IDLE);                                        // We are now in the idle state
                        }
                        else if (GetNextSound(IdleSound, nextIdleSound, NUM_SOUNDS_IDLE))   // This should definitely return true because engine is only enabled if we have at least one idle sound
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
                        SetEngineState(ES_RUN);                                             // Done with the accel sound, we can transition to run
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

                case EN_TR_TURN_OFF:
                    // Check if the engine shutdown sound is done playing - if so, we can truly set the EngineRunning state to off. 
                    if (Engine[i].SDWav.isPlaying() == false && (millis() - Engine[i].timeStarted) > TIME_TO_PLAY)   // According to https://www.pjrc.com/teensy/td_libs_AudioPlaySdWav.html can take up to 3mS to return isPlaying()
                    {
                        Engine[i].isActive = false;                                         // Clear flag
                        EngineRunning = false;                                              // Engine is no longer running
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

                    // Another time we might want to stop a sound playing is if we had faded it out, and the fade time is expired. If we don't do anything, the sound will continue to play 
                    // until the full length of the clip has been processed, but beyond the fade out time we will just be playing static. So to avoid the static portion, we stop the sound
                    // if the fade-out is complete. 
                    if (Engine[i].SDWav.isPlaying() && Engine[i].isActive && Engine[i].fadingOut && (millis() - Engine[i].timeWillFadeOut > 0))
                    {
                        Engine[i].SDWav.stop();
                        Engine[i].isActive = false;
                        Engine[i].fadingOut = false;
                    }                    
                    break;
            }
        }
    }
}

uint8_t GetRunSoundNumBySpeed()
{
    return map(EngineSpeed, 0, 255, 0, (NumRunSounds - 1)); // Subtract 1 because the run sound array is zero-based
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

void StartAutoStopAtIdleTimer(void)
{
    // If we are in RC mode and the user has specified an auto-stop time, 
    // then start the timer
    if (InputMode == INPUT_RC && Engine_AutoStop > 0)
    {
        if (Engine_AutoStop_TimerID > 0) timer.restartTimer(Engine_AutoStop_TimerID);
        else                             Engine_AutoStop_TimerID = timer.setTimeout(Engine_AutoStop, StopEngine);
    }
}

void ClearAutoStopAtIdleTimer(void)
{
    // In this case we must not have remained at idle for the necessary length of time, so we want to cancel the stop engine timer
    if (InputMode == INPUT_RC && Engine_AutoStop > 0)
    {
        timer.deleteTimer(Engine_AutoStop_TimerID);
        Engine_AutoStop_TimerID = 0;
    }        
}

