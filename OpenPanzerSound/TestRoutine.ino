
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// TEST ROUTINE
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// If on startup a jumper is placed on RC Input #1 (signal held to ground), the sound card will enter a test routine. During this time it will play
// each sound one by one of all the sounds it is programmed to detect. 

void DetectJumper(void)
{
    static uint8_t testState = 0; 
    static uint32_t t; 
    static int TID = 0;
    #define HOW_LONG_TO_WAIT 1500

    // A rising edge was detected, quit the test routine
    if (CancelTestRoutine)
    {
        TestRoutine = false;
        timer.deleteTimer(TID); 
        return;                 // Leave and never come back! 
    }

    switch (testState)
    {
        case 0:         // Just booted up
            if (digitalRead(RC_Channel[TEST_CHANNEL].pin)  == LOW)
            {   // Ok, the pin is low, but it needs to stay low for some time before we can be sure this is a jumper and not a standard signal                            
                // By the way, input pullups are turned on above, so that's good....
                t = millis();                                   // Save the time
                TID = timer.setTimeout(20, DetectJumper);       // Come back here again in 20mS (typical RC PWM frame length)
                testState = 1;                                  // Move to next state
            }
            break;

        case 1: 
            if ((digitalRead(RC_Channel[TEST_CHANNEL].pin) == LOW) && RC_Channel[TEST_CHANNEL].state == RC_SIGNAL_ACQUIRE)
            {
                // We're still low, and so far the RC channel reader hasn't detected a series of valid frame, or indeed even a rising edge (that would have triggered CancelTestRoutine)
                if ((millis() - t) < HOW_LONG_TO_WAIT)
                {
                    TID = timer.setTimeout(20, DetectJumper);   // Come back again to this state and keep checking
                }
                else
                {
                    TestRoutine = true;                         // Enough time has passed, and input is still low - enter the test routine
                    testState = 2;                              // Move to next state
                    TID = timer.setTimeout(100, DetectJumper);  // And come back 
                }
            }
            else
            {
                TestRoutine = false;                            // We will exit and never come back
            }
            break;

        case 2:
            if (digitalRead(RC_Channel[TEST_CHANNEL].pin) == HIGH)
            {
                TestRoutine = false;                            // Exit and never return
            }
            else
            {
                TID = timer.setTimeout(100, DetectJumper);      // Come back again to this state and keep checking, doesn't have to be as often. The rising edge would also cause us to come here and cancel. 
            }
            break;
    }
}

void PrintStartTestRoutine(void)
{
    DebugSerial.println("");
    PrintDebugLine();
    DebugSerial.println(F("START TEST ROUTINE"));
    PrintDebugLine();    
    DebugSerial.println(F("Filename           MM:SS:mS"));
    PrintDebugLine();    
}

void PrintEndTestRoutine(void)
{
    DebugSerial.println("");
    PrintDebugLine();
    DebugSerial.println(F("END TEST ROUTINE"));
    PrintDebugLine();    
    DebugSerial.println("");
}

void RunTestRoutine(void)
{
static int16_t soundNum = -1;
static boolean firstStart = false;
static boolean wasPlaying = true;       // Start this at True to get things kicked off
static uint32_t endTime;
static uint32_t startTime;
#define timeBetweenStop     800         // For the user's sanity, wait briefly between each sound
#define timeAfterStart      100         // Minimum amount of time after starting the last sound to check for the end of it (because it won't show as playing right away)
    
    // While in the test routine, play a sound, wait for it to finish, then play the next one, over and over

    // For complicated reasons we need a little pause between each sound

    // We can use any SD wav slot, it doesn't matter. We choose FX[0]

    if (!FX[0].SDWav.isPlaying() && wasPlaying && ((millis() - startTime) > timeAfterStart))       
    {
        // We were playing, but the sound ended

        if (!firstStart) firstStart = true; 
        else if (allSoundFiles[soundNum]->exists) DebugSerial.println(F("Done"));   // We're done with this sound        
        
        wasPlaying = false;                                     // Reset
        endTime = millis();                                     // Save the time
        soundNum++;                                             // Increment to next sound
        if (soundNum >= COUNT_TOTAL_SOUNDFILES) 
        {
            soundNum = 0;                                       // Loop
            DebugSerial.println();
            PrintDebugLine();
            DebugSerial.println();
        }
    }

    if (!FX[0].SDWav.isPlaying() && !wasPlaying && ((millis() - endTime) > timeBetweenStop))
    {
        // Ok, the last sound was already over, we have already no been playing anything, and the time since we last played something exceeds our minimum amount. 
        // We can now start the next sound
        if (SD.exists(allSoundFiles[soundNum]->fileName))
        {
            FX[0].SDWav.play(allSoundFiles[soundNum]->fileName);    // Play next sound
            startTime = millis();
        }
        wasPlaying = true;                                      // We are playing
        PrintSoundFileLineTest(allSoundFiles[soundNum]);
    }
}

void PrintSoundFileLineTest(_soundfile *s)
{
uint8_t c = 0; 
        
    for (uint8_t i = 0; i < sizeof(s->fileName); i++)
    {
        DebugSerial.print(s->fileName[i]);
        c++;
    }
    if (c < 12) { c = 12 - c; PrintSpaces(c); }
    PrintSpaces(6); 
    if (s->exists)
    {
        PrintTrackTime(s->length); 
        PrintSpaces(6); 
        DebugSerial.print(F("Playing...  ")); 
    }
    else
    {
        DebugSerial.print("-");
        PrintSpaces(13);
        DebugSerial.print(F("Not found"));
        DebugSerial.println();
    }
}



