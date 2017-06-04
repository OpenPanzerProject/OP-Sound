
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// RC INPUTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void InitializeRCChannels(void)
{
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        pinMode(RC_Channel[i].pin, INPUT_PULLUP);
        RC_Channel[i].state = RC_SIGNAL_ACQUIRE;
        RC_Channel[i].pulseWidth = 1500;
        RC_Channel[i].value = 0;
        RC_Channel[i].switchPos = 0;
        RC_Channel[i].lastEdgeTime = 0;
        RC_Channel[i].lastGoodPulseTime = 0;
        RC_Channel[i].acquireCount = 0;
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
                RC_Channel[ch].state = RC_SIGNAL_LOST;
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

// ====================================================================================================================================>>
// ------------------------------------------------------------------------------------------------------------------------------------>>
// 
// Since the TCB can communicate with this device over serial, and since that gives us far greater control and options without
// the user needing to do any device configuration (other than loading their sounds to he SD card), we are not too interested 
// in RC control. 
//
// The code herein gets you started, and handles automatically selecting between RC and Serial modes, reading the RC channels, 
// managing the status of the receiver, etc... You can also easily add more RC channels just by changing the NUM_RC_CHANNELS define
// and creating new ISR entries for them above. But this is all really just a skeleton of what would be needed for useful or 
// advanced features. 
// 
// The code in ProcessRCCommand() below will allow you to toggle the engine with one channel, set the throttle with another, 
// and trigger certain sounds with a third, very much like the Benedini TBS. 
// 
// But due to the severe limitations of RC generally, one really needs to implement some sort of configuration program in the 
// same manner the Benedini does. Less ideal alternatives from the end-user's point of view could be an ini file on the SD card, 
// or just a settings.h file within the sketch that could be adjusted and then the whole project re-compiled. 
//
// Given the Benedini costs ~$150 USD and has less capable hardware than this board, there is definitely an opportunity and perhaps
// a need in the market for a quality sound device for RC models generally (ie, not paired with the TCB). 
// 
// We leave development on that front for other interested and motivated coders to undertake. This will get you started. 
// 
// ------------------------------------------------------------------------------------------------------------------------------------>>
// ====================================================================================================================================>>

void ProcessRCCommand(uint8_t ch)
{
    uint8_t val;
    uint8_t pos;
    #define throttleHysterisis 5
    
    switch (ch)
    {
        case 0:                                                                                         // Throttle
            RC_Channel[ch].pulseWidth = constrain(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX); // Constrain pulse width
            val = map(RC_Channel[ch].pulseWidth, PULSE_WIDTH_TYP_MIN, PULSE_WIDTH_TYP_MAX, 0, 255);     // Map to engine speed range
            if (abs((int16_t)RC_Channel[ch].value - (int16_t)val) > throttleHysterisis)                                 // If engine speed has changed, update
            {
                RC_Channel[ch].value = val;
                SetEngineSpeed(RC_Channel[ch].value);
            }
            break;

        case 1:                                                                                         // Engine on/off (two position switch)
            if (RC_Channel[ch].pulseWidth > 1500 && RC_Channel[ch].switchPos == 0)                      // Start engine if new value
            {
                RC_Channel[ch].switchPos = 1;
                StartEngine();
            }
            else if (RC_Channel[ch].pulseWidth <=1500 && RC_Channel[ch].switchPos == 1)                 // Stop engine if new value
            {
                RC_Channel[ch].switchPos = 0;
                StopEngine();
            }
            break;

        case 2:                                                                                         // Multi-position switch
            pos = PulseToMultiSwitchPos(RC_Channel[ch].pulseWidth);                                     // Calculate switch position
            if (pos != RC_Channel[ch].switchPos)                                                        // Update only if changed
            {
                RC_Channel[ch].switchPos = pos;
                switch (RC_Channel[ch].switchPos)
                {
                    case 0:                                 break;  // Center-off, do nothing
                    case 1: CannonFire();                   break;
                    case 3: LightSwitch();                  break;
                    // etc... 
                }
            }
            break;
    }
}

int PulseToMultiSwitchPos(int pulse)
{
    // Thanks to Rob Tillaart for the distance function
    // http://forum.arduino.cc/index.php?topic=254836.0

    // This takes a pulse and returns the switch position with the 
    // nearest matching pulse
    
    int idx = 0; 
    int distance = abs(MultiSwitch_MatchArray[idx] - pulse); 

    for (int i = 1; i < RC_MULTISWITCH_POSITIONS; i++)
    {
        int d = abs(MultiSwitch_MatchArray[i] - pulse);
        if (d < distance)
        {
            idx = i;
            distance = d;
        }
    }
    return idx;
}

