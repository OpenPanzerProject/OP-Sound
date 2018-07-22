
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// VOLUME
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void SetVolume()
{    
    float fTot = 0.0;
    uint8_t finalCount = 0; 
    float g = 0.0; 
    uint8_t active[4] = {0, 0, 0, 0};
        
    // Volume from serial and RC sources update themselves whenever a command arrives.
    // But the volume knob has to be polled, we do so here
    UpdateVolume_Knob();

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Figure out pre-final mixer adjustments, and count how many sounds are playing
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Engine 0 & 1 
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if (Engine[0].isActive || Engine[1].isActive) 
        { 
            active[VC_ENGINE] = 1;
            fTot += fVols[VC_ENGINE]; 
            finalCount += 1; 
        }
        
    // FX 0 & 1
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if (FX[0].isActive || FX[1].isActive)
        {
            if (FX[0].isActive && FX[1].isActive) g = 0.5;      // Both active, divide in half through Mixer 2
            else                                  g = 1.0;      // Only one is active, no mixing required on Mixer 2
            
            for (uint8_t i=0; i<4; i++)                         // Set Mixer2 gain
            {
                if (dynamicallyScaleVolume) Mixer2.gain(i, g);   // Potentially adjusted if multiple effects playing at once
                else                        Mixer2.gain(i, 1.0); // Full volume no matter what
            }
            
            active[VC_EFFECTS] = 1;
            fTot += fVols[VC_EFFECTS];
            finalCount += 1;
        }
        
    // FX 2 & 3
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // EDIT: If TrackOverlay is enabled, we should just go ahead and compensate the engine sound regardless of whether those sounds are playing now or not. Otherwise what happens
        // is that engine idle sounds very loud (because we are not compensating for track sounds yet), but gets quieter when track sounds start, and go back to being loud after they stop
        if (TrackOverlayEnabled && EngineRunning) 
        {
            fTot += fVols[VC_TRACK_OVERLAY];
            active[VC_TRACK_OVERLAY] = 1; 
            finalCount += 1;
        }
        else 
        {
            if (FX[2].isActive || FX[3].isActive)
            {
                if (!TrackOverlayEnabled && FX[0].isActive && FX[1].isActive) 
                {
                    g = 0.5;                                        // Both active, regular effects, divide in half through Mixer 3
                    fTot += fVols[VC_EFFECTS];                      // Because these are not track overlay sounds they are by definition effects, so use the effects ratio
                }
                else if (TrackOverlayEnabled)         
                {
                    g = 1.0;                                        // By definition, track overlay will not be playing both at once (except to fade), so we can set all inputs to 1
                    fTot += fVols[VC_TRACK_OVERLAY];                // Use the track overlay ratio
                }
                else 
                {
                    g = 1.0;                                        // In this case track overlay is not enabled, and we also know that only one or 0 effects are playing, gain can be 1
                    fTot += fVols[VC_EFFECTS];                      // Because these are not track overlay sounds they are by definition effects, so use the effects ratio
                }
                
                for (uint8_t i=0; i<4; i++)                         // Set Mixer3 gain
                {
                    if (dynamicallyScaleVolume) Mixer3.gain(i, g);   // Potentially adjusted if multiple effects playing at once
                    else                        Mixer3.gain(i, 1.0); // Full volume no matter what
                }
                
                active[VC_TRACK_OVERLAY] = 1;                       // Regardless of the above, the active MixerFinal input here is number VC_TRACK_OVERLAY
                finalCount += 1;
            }
        }
        
    // Flash
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if (FlashRaw.isPlaying()) 
        { 
            fTot += fVols[VC_FLASH]; 
            active[VC_FLASH] = 1;
            finalCount +=1; 
        }


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // We may not even need any volume
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // If Volume is below some minimum amount then we can go ahead and shut off the amp, regardless of whether a sound is playing or not
    if (Volume <= MinVolume)
    {
        MuteAmp(); 
    }
    else
    {
        // Only if volume is above a certain level and if something is even playing we enable the amp, otherwise keep it off to avoid hearing any hissing, popping, whatever. 
        finalCount == 0 ? MuteAmp() : UnMuteAmp();
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Now mix the four inputs of the MixerFinal
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    if (finalCount <= 1) 
    {   
        // Easy case, only one sound playing, or none. 
        // Gain can be 1 for all MixerFinal inputs (only one or none will be playing) - or in other words, just pass through master volume directly
        for (uint8_t i=0; i<4; i++) { MixerFinal.gain(i, Volume); } 
    }
    else
    {
        // Harder case, we have multiple sounds playing
        if (fTot > 0.0)
        {   
            if (fTot < 1.0) fTot = 1.0;
            
            for (uint8_t i=0; i<4; i++)
            {   
                if (active[i]) 
                {
                    if (dynamicallyScaleVolume)
                    {
                        MixerFinal.gain(i, ((fVols[i]*Volume)/fTot));    // Scale each active input by the total so we maintain ratiometric quantities but don't exceed 1, and multiply by volume fraction to adjust absolute intensity
                    }
                    else
                    {
                        //MixerFinal.gain(i, (fVols[i]*Volume));           // We still scale each input by its relative limit, but we don't stop the total gain from surpassing 1
                        MixerFinal.gain(i, Volume);                     // Even that scheme can cause some unusual issues if you keep repeating too many sounds too quickly which some have complained about, so I'm leaving it off for now. 
                    }                                                   // In normal usage it wouldn't matter, but people are going to do crazy things. 
                }
                else    MixerFinal.gain(i,  0.0);
            }
        }
        else
        {
            for (uint8_t i=0; i<4; i++) MixerFinal.gain(i, 0.0);
        }
    }

    /* 
    // Volume debugging
    DebugSerial.print(Volume, 2); DebugSerial.print(" ");
    for (uint8_t i=0; i<4; i++)
    {
        active[i] ? DebugSerial.print(((fVols[i]/fTot)*Volume),2) : DebugSerial.print("0.00"); 
        DebugSerial.print(" ");
    }
    DebugSerial.println();
    */ 
}

void UpdateRelativeVolume(uint8_t level, uint8_t vc)
{
    // Save the user's relative volume setting
    level = constrain(level, 0, 100);

    // Save volume as floats (0.0 - 1.0)
    fVols[vc] = ((float)level / 100.0);

/*
    if (DEBUG)
    {
        switch (vc)
        {
            case VC_ENGINE:         DebugSerial.print(F("Engine Level: "));          DebugSerial.print(level);    break;
            case VC_TRACK_OVERLAY:  DebugSerial.print(F("Track Overlay Level: "));   DebugSerial.print(level);    break;
            case VC_EFFECTS:        DebugSerial.print(F("Effects Level: "));         DebugSerial.print(level);    break;
            case VC_FLASH:          DebugSerial.print(F("Flash Level: "));           DebugSerial.print(level);    break;
        }
        DebugSerial.println();
    }
*/
}

void UpdateVolume_Serial(uint8_t level)
{
    // This function gets called if we receive a serial command. It overrides any prior volume source. 
    // Implement hysterisis on the master device (ie, TCB)
    if (volumeSource != vsSerial)
    {
        volumeSource = vsSerial;
        DebugSerial.println(F("Volume Control Source: Serial")); 
    }
    Volume = (float)level / 100.0;              // Convert to percent (some number between 0 - 1)
}

void UpdateVolume_RC(uint8_t level)
{
    // This function gets called if we receive a volume command from an RC input. It overrides any prior volume source. 
    // Hysterisis is handled in the function that calls this one, namely ProcessRCCommand()
    if (volumeSource != vsRC)
    {
        volumeSource = vsRC;
        DebugSerial.println(F("Volume Control Source: RC")); 
    }
    Volume = (float)level / 100.0;              // Convert to percent (some number between 0 - 1)
}

void UpdateVolume_Knob()
{
    int reading = analogRead(Volume_Knob);; 
    static int lastReading = 0;
    // This needs to be a number greater than the variation in analog readings detected when the volume potentiometer is disconnected.
    // Even though we enable internal pullups we find quite a bit of wandering, so we set this rather high. 
    #define volumeKnobHysterisis    75

    // If the volume knob has changed significantly, and knob was not set to the volume source, make it so
    if (volumeSource != vsKnob && (abs(reading - lastReading) > volumeKnobHysterisis))
    {
        volumeSource = vsKnob;
        DebugSerial.println(F("Volume Control Source: Knob")); 
    }

    // If volume knob is the current volume source, assign the reading to the global Volume variable
    if (volumeSource == vsKnob)
    {
        Volume = (float)reading / 1024.0;       // Convert to percent (some number between 0 - 1)
        lastReading = reading;
    }
}

void Mute()
{
    Volume = 0;
    SetVolume();
}

void MuteAmp()
{
    switch (HardwareVersion)
    {
        case 1:     digitalWrite(Amp_Enable, LOW);  break;      // LM48310 - set low to disable
        case 2:     digitalWrite(Amp_Mute, HIGH);   break;      // MAX9768 - set high to mute 
    }
}

void UnMuteAmp()
{
    switch (HardwareVersion)
    {
        case 1:     digitalWrite(Amp_Enable, HIGH); break;      // LM48310 - set high to enable
        case 2:     digitalWrite(Amp_Mute, LOW);    break;      // MAX9768 - set low to un-mute 
    }
}




