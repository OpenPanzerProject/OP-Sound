
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// VOLUME
// -------------------------------------------------------------------------------------------------------------------------------------------------->
void SetVolume()
{
    // The volume knob has to be polled, we do so here
    UpdateVolume_Knob();
    
    // Volume from serial updates itself whenever a serial command arrives
    // 
    
    // Volume is actually applied by adjusting the gain on MixerFinal
    float quartervol = Volume / 4.0; 
    for (uint8_t i=0; i<4; i++) 
    { 
        Gain[i] = quartervol; 
        MixerFinal.gain(i, Gain[i]);
    }    
}

void UpdateVolume_Serial(uint8_t level)
{
    // This function gets called if we receive a serial command. It overrides any prior volume source. 
    // Implement hysterisis on the master device (ie, TCB)
    volumeSource = vsSerial;
    Volume = (float)level / 100.0;              // Convert to percent (some number between 0 - 1)
}

void UpdateVolume_Knob()
{
    int reading = analogRead(Volume_Knob);; 
    static int lastReading = 0;
    #define volumeKnobHysterisis    10

    // If the volume knob has changed significantly, and knob was not set to the volume source, make it so
    if (volumeSource != vsKnob && (abs(reading - lastReading) > volumeKnobHysterisis))
    {
        volumeSource = vsKnob;
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

