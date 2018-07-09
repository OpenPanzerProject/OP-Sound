
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// RC INPUTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void InitializeRCChannels(void)
{
    // Some of these settings will be overwritten later if the ini file is found, but we initialize to default values here
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        pinMode(RC_Channel[i].pin, INPUT_PULLUP);
        RC_Channel[i].state = RC_SIGNAL_ACQUIRE;
        RC_Channel[i].pulseWidth = 1500;
        RC_Channel[i].value = 0;
        RC_Channel[i].reversed = false;
        RC_Channel[i].numSwitchPos = 3;             // Start with switches at 3-position by default
        RC_Channel[i].switchPos = 2;                // And start with switch in center position
        RC_Channel[i].lastEdgeTime = 0;
        RC_Channel[i].lastGoodPulseTime = 0;
        RC_Channel[i].acquireCount = 0;
        RC_Channel[i].Digital = true;                
        RC_Channel[i].anaFunction = ANA_NULL_FUNCTION;
    }
}

void EnableRCInterrupts(void)
{   // Pin change interrupts
    attachInterrupt(RC_Channel[0].pin, RC0_ISR, CHANGE); 
    attachInterrupt(RC_Channel[1].pin, RC1_ISR, CHANGE); 
    attachInterrupt(RC_Channel[2].pin, RC2_ISR, CHANGE);     
    attachInterrupt(RC_Channel[3].pin, RC3_ISR, CHANGE);     
    attachInterrupt(RC_Channel[4].pin, RC4_ISR, CHANGE);     
}

void DisableRCInterrupts(void)
{
    detachInterrupt(digitalPinToInterrupt(RC_Channel[0].pin)); 
    detachInterrupt(digitalPinToInterrupt(RC_Channel[1].pin)); 
    detachInterrupt(digitalPinToInterrupt(RC_Channel[2].pin)); 
    detachInterrupt(digitalPinToInterrupt(RC_Channel[3].pin)); 
    detachInterrupt(digitalPinToInterrupt(RC_Channel[4].pin)); 
}

void RC0_ISR()
{
    ProcessRCPulse(0);
}

void RC1_ISR()
{
    ProcessRCPulse(1);
}

void RC2_ISR()
{
    ProcessRCPulse(2);
}

void RC3_ISR()
{
    ProcessRCPulse(3);
}

void RC4_ISR()
{
    ProcessRCPulse(4);
}

void ProcessRCPulse(uint8_t ch)
{
    uint32_t    pulseWidth;
    uint32_t    uS;
    uint8_t     countSame = 0;

    uS =  micros();    
    
    if (digitalRead(RC_Channel[ch].pin))
    {   // Rising edge - save the time and exit
        RC_Channel[ch].lastEdgeTime = uS;

        // Also if this is the test channel, cancel the test routine
        if (ch == TEST_CHANNEL) CancelTestRoutine = true;
        
        return;
    }
    else
    {   // Falling edge - pulse received
        pulseWidth = uS - RC_Channel[ch].lastEdgeTime;
        if (pulseWidth >= PULSE_WIDTH_ABS_MIN && pulseWidth <= PULSE_WIDTH_ABS_MAX)
        {
            RC_Channel[ch].pulseWidth = pulseWidth;
            RC_Channel[ch].lastGoodPulseTime = uS;
            // Update the channel's state if needed 
            switch (RC_Channel[ch].state)
            {
                case RC_SIGNAL_SYNCHED:
                    // Do something with the pulse
                    ProcessRCCommand(ch);
                    break;
                        
                case RC_SIGNAL_LOST:
                    RC_Channel[ch].state = RC_SIGNAL_ACQUIRE;
                    RC_Channel[ch].acquireCount = 1;
                    break;
                
                case RC_SIGNAL_ACQUIRE:
                    if (++RC_Channel[ch].acquireCount >= RC_PULSECOUNT_TO_ACQUIRE)
                    {
                        RC_Channel[ch].state = RC_SIGNAL_SYNCHED;
                    }
                    break;
            }            
         }
        else 
        {
            // Invalid pulse. If we haven't had a good pulse for a while, set the state of this channel to SIGNAL_LOST. 
            if (uS - RC_Channel[ch].lastGoodPulseTime > RC_TIMEOUT_US)
            {
                // Except! For TestRoutine or if we are not yet sure of the mode
                if (TestRoutine && ch == TEST_CHANNEL)  RC_Channel[ch].state = RC_SIGNAL_ACQUIRE;   // Keep it on acquire
                else if (InputMode != INPUT_RC)         RC_Channel[ch].state = RC_SIGNAL_ACQUIRE;   // Keep it on acquire
                else                                    RC_Channel[ch].state = RC_SIGNAL_LOST;      // Ok, now we treat it as lost
                RC_Channel[ch].acquireCount = 0;
            }            
        }
    
        // We know what the individual channel's state is, but let's combine all channel's states into a single 'RC state' 
        // If all channels share the same state, then that is also the state of the overall RC system
        countSame = 1;
        for (uint8_t i=1; i<NUM_RC_CHANNELS; i++)
        {
            if (RC_Channel[i-1].state == RC_Channel[i].state) countSame++;
        }
        
        // After that, if countSame = NUM_RC_CHANNELS then all states are the same
        if (countSame == NUM_RC_CHANNELS)   RC_State = RC_Channel[0].state;
        else
        {   // In this case, the channels have different states, so we need to condense. 
            // If even one channel is synched, we consider the system synched. 
            // Any other combination we consider signal lost. 
            RC_State = RC_SIGNAL_LOST;
            for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
            {
                if (RC_Channel[i].state == RC_SIGNAL_SYNCHED) 
                { 
                    RC_State = RC_SIGNAL_SYNCHED;
                    break;
                }
            }
        }

        // Has RC_State changed?
        if (RC_State != Last_RC_State)  ChangeRCState();
    }
}

void UpdateRCStatus(void)
{
    uint32_t        uS;                         // Temp variable to hold the current time in microseconds                        
    static uint32_t TimeLastRCCheck = 0;        // Time we last did a watchdog check on the RC signal
    uint8_t         countOverdue = 0;           // How many channels are overdue (disconnected)

    // The RC pin change ISRs will try to determine the status of each channel, but of course if a channel becomes disconnected the ISR won't even trigger. 
    // So we have the main loop poll this function to do an overt check once every so often (RC_TIMEOUT_MS) 
    if (millis() - TimeLastRCCheck > RC_TIMEOUT_MS)
    {
        TimeLastRCCheck = millis();
        uS = micros();                          // Current time
        AudioNoInterrupts();                    // If we don't do this prior to cli() we will mess up the sound 
        cli();                                  // We need to disable interrupts for this check, otherwise value of (uS - LastGoodPulseTime) could return very big number if channel updates in the middle of the check
            for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
            {
                if ((uS - RC_Channel[i].lastGoodPulseTime) > RC_TIMEOUT_US) 
                {
                    countOverdue += 1;
                    // If this channel had previously been synched, set it now to lost
                    if (RC_Channel[i].state == RC_SIGNAL_SYNCHED)
                    {
                        RC_Channel[i].state = RC_SIGNAL_LOST;
                        RC_Channel[i].acquireCount = 0;
                    }
                }
            }
        sei();                                  // Resume interrupts
        AudioInterrupts();                      // Also for the audio library
        
        if (countOverdue == NUM_RC_CHANNELS)
        {   
            RC_State = RC_SIGNAL_LOST;          // Ok, we've lost radio on all channels
        }

        // If state has changed, update the LEDs
        if (RC_State != Last_RC_State) ChangeRCState();
    }
}

void ChangeRCState(void)
{
    switch (RC_State)
    {
        case RC_SIGNAL_SYNCHED:
            BlueLed.on();
            break;
                
        case RC_SIGNAL_LOST:
            BlueLed.off();
            BlueLed.blinkLostSignal();
            break;
        
        case RC_SIGNAL_ACQUIRE:
            BlueLed.off();
            BlueLed.blinkHeartBeat();
            break;        
    }
    Last_RC_State = RC_State;
}


void ProcessRCCommand(uint8_t ch)
{
    uint8_t val;
    uint8_t pos;
    uint8_t num = 0;
    #define volumeHysterisis 5
    
    if (RC_Channel[ch].Digital)     // This is a switch
    {
        pos = PulseToMultiSwitchPos(ch);                                                            // Calculate switch position
        if (pos != RC_Channel[ch].switchPos)                                                        // Proceed only if switch position has changed
        {
            RC_Channel[ch].switchPos = pos;                                                         // Update switch position
            for (uint8_t t=0; t<triggerCount; t++)
            {   // If we have a function trigger whose trigger conditions match this channel and switch position, execute the function associated with it
                if (SF_Trigger[t].ChannelNum == (ch+1) && SF_Trigger[t].ChannelPos == RC_Channel[ch].switchPos)
                {
                    num = SF_Trigger[t].actionNum;   // Remember, actionNum is not zero-based! It starts at 1.
                    
                    switch (SF_Trigger[t].swFunction)
                    {
                        case SF_ENGINE_START:   StartEngine();                                      break;
                        case SF_ENGINE_STOP:    StopEngine();                                       break;
                        case SF_ENGINE_TOGGLE:  EngineRunning ? StopEngine() : StartEngine();       break;                               
                        
                        case SF_CANNON_FIRE:    
                            CannonFire(num);                                    // Sound
                            HandleLight(num, ACTION_FLASH);                     // Flash
                            break;
                        
                        case SF_MG:   
                            switch (SF_Trigger[t].switchAction)
                            {
                                case ACTION_ONSTART:        
                                    if (!MG_Active[num-1]) { MG(num, true);  HandleLight(num, ACTION_STARTBLINK); }
                                    break;  
                                case ACTION_OFFSTOP:        
                                    if (MG_Active[num-1])  { MG(num, false); HandleLight(num, ACTION_OFFSTOP); }
                                    break;  
                                case ACTION_REPEATTOGGLE:   
                                    if (MG_Active[num-1])  { MG(num, false); HandleLight(num, ACTION_OFFSTOP); } else { MG(num, true); HandleLight(num, ACTION_STARTBLINK); } break;
                                    break;
                                default: break;
                            }
                            break;

                        case SF_LIGHT:
                            switch (SF_Trigger[t].switchAction) // For some actions we play a sound
                            {
                                case ACTION_ONSTART:        
                                case ACTION_OFFSTOP:        
                                case ACTION_REPEATTOGGLE: 
                                    LightSwitch_Sound(num);
                                    break;
                                default: 
                                    break;
                            }
                            HandleLight(num, SF_Trigger[t].switchAction);
                            break;

                        case SF_USER:
                            switch (SF_Trigger[t].switchAction)
                            {
                                case ACTION_ONSTART:        PlayUserSound(num, true, false);    break;  // Play
                                case ACTION_OFFSTOP:        PlayUserSound(num, false, false);   break;  // Stop
                                case ACTION_REPEATTOGGLE:   PlayUserSound(num, true, true);     break;  // Repeat
                                default: break;
                            }
                            break;
                            
                        case SF_NULL:                                                               
                            break;  // Do nothing
                    }
                }
            }
        }            
    }
    else    // Variable input 
    {
        RC_Channel[ch].pulseWidth = constrain(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX); // Constrain pulse width
        
        switch (RC_Channel[ch].anaFunction)
        {
            case ANA_MASTER_VOL:
                val = map(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX, 0, 100);     // Map to volume range
                if (abs((int16_t)RC_Channel[ch].value - (int16_t)val) > volumeHysterisis)                   // If volume command has changed, update
                {
                    RC_Channel[ch].value = val;
                    UpdateVolume_RC(RC_Channel[ch].value);
                }
                break;
            
            case ANA_ENGINE_SPEED:
                if (ThrottleCenter)
                {
                    // Idle when stick at center
                    // Calculate distance from center
                    int16_t temp = abs((RC_Channel[ch].pulseWidth - PULSE_WIDTH_CENTER));
                    val = map(temp, 0, (PULSE_WIDTH_TYP_MAX - PULSE_WIDTH_CENTER), 0, 255); 
                }
                else
                {
                    // Idle when stick at one extreme. Which end depends on channel reversed flag. 
                    if (RC_Channel[ch].reversed)
                    {
                        val = map(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX, 0, 255);
                    }
                    else
                    {
                        val = map(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX, 255, 0);
                    }
                }
                // Now val is some number between 0-255 representing the actual engine speed
                // Update if change exceeds hysterisis, or if we are just coming to idle
                if ((abs((int16_t)RC_Channel[ch].value - (int16_t)val) > throttleHysterisisRC) || (val < throttleHysterisisRC))
                {
                    RC_Channel[ch].value = val;
                    SetEngineSpeed(RC_Channel[ch].value);
                }
                break;
        }
    }
}


int PulseToMultiSwitchPos(uint8_t ch)
{
    // Thanks to Rob Tillaart for the distance function
    // http://forum.arduino.cc/index.php?topic=254836.0

    // This takes a pulse and returns the switch position with the 
    // nearest matching pulse

    int pulse = RC_Channel[ch].pulseWidth;
    uint8_t numPos = RC_Channel[ch].numSwitchPos;
    
    byte POS = 0; 
    int d = 9999;
    int distance = abs(RC_MULTISWITCH_START_POS - pulse);

    for (int i = 1; i < numPos; i++)
    {
        switch (numPos)
        {
            case 2: d = abs(MultiSwitch_MatchArray2[i] - pulse);   break;
            case 3: d = abs(MultiSwitch_MatchArray3[i] - pulse);   break;
            case 4: d = abs(MultiSwitch_MatchArray4[i] - pulse);   break;
            case 5: d = abs(MultiSwitch_MatchArray5[i] - pulse);   break;
            case 6: d = abs(MultiSwitch_MatchArray6[i] - pulse);   break;
        }

        if (d < distance)
        {
            POS = i;
            distance = d;
        }
    }
   
    // Add 1 to POS because from here we don't want zero-based
    POS += 1;
    
    // Swap positions if channel is reversed. 
    if (RC_Channel[ch].reversed)
    {
        switch (numPos)
        {
            case 2: 
                if (POS == Pos1) POS = Pos2; 
                else POS = Pos1; 
                break;
            case 3: 
                if (POS == Pos1) POS = Pos3; 
                else if (POS == Pos3) POS = Pos1; 
                break;
            case 4: 
                if (POS == Pos1) POS = Pos4; 
                else if (POS == Pos2) POS = Pos3; 
                else if (POS == Pos3) POS = Pos2; 
                else if (POS == Pos4) POS = Pos1; 
                break;
            case 5: 
                if (POS == Pos1) POS = Pos5; 
                else if (POS == Pos2) POS = Pos4; 
                else if (POS == Pos4) POS = Pos2; 
                else if (POS == Pos5) POS = Pos1; 
                break;                
            case 6: 
                if (POS == Pos1) POS = Pos6; 
                else if (POS == Pos2) POS = Pos5; 
                else if (POS == Pos3) POS = Pos4; 
                else if (POS == Pos4) POS = Pos3; 
                else if (POS == Pos5) POS = Pos2; 
                else if (POS == Pos6) POS = Pos1; 
                break;                                
        }    
    }

    return POS;
}


