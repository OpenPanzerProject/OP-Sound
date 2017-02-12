
void DumpVersion()
{
    #define FIRMWARE_VERSION     1
    DebugSerial.println();
    PrintDebugLine();
    DebugSerial.print(F("FIRMWARE VERSION: "));
    String str = FIRMWARE_VERSION;
    DebugSerial.println(str);
}

void DumpSoundFileInfo()
{
    DebugSerial.println();
    PrintDebugLine();
    DebugSerial.println(F("SOUND FILE INFO")); 
    PrintDebugLine();
    DebugSerial.println(F("Filename           MM:SS:mS     Priority"));
    PrintDebugLine();

    PrintSoundFileLine(EngineColdStart);
    PrintSoundFileLine(EngineHotStart);
    PrintSoundFileLine(EngineShutoff);
    for (uint8_t i=0; i<NUM_SOUNDS_IDLE; i++)
    {
        PrintSoundFileLine(IdleSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_ACCEL; i++)
    {
        PrintSoundFileLine(AccelSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_DECEL; i++)
    {
        PrintSoundFileLine(DecelSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUNDS_RUN; i++)
    {
        PrintSoundFileLine(RunSound[i]);
    }
    for (uint8_t i=0; i<NUM_SOUND_FX; i++)
    {
        PrintSoundFileLine(Effect[i]);
    }        
    for (uint8_t i=0; i<NUM_USER_SOUNDS; i++)
    {
        PrintSoundFileLine(UserSound[i]);
    }        
    // Decide if we should enable engine sound functionality
    // We need at least one start sound, one idle sound, and one run sound
    if ((EngineColdStart.exists || EngineHotStart.exists) && IdleSound[0].exists && RunSound[0].exists) 
    {
        EngineEnabled = true;
        DebugSerial.print(F("Engine sounds enabled")); 
    }     
    else 
    {
        EngineEnabled = false;
        DebugSerial.print(F("Engine sounds disabled - files missing"));         
    }    
    DebugSerial.println();
}

void PrintSoundFileLine(_soundfile &s)
{
    for (uint8_t i = 0; i < sizeof(s.fileName); i++)
    {
        Serial.print(s.fileName[i]);
    }
    PrintSpaces(6); 
    if (s.exists)
    {
        PrintTrackTime(s.length); 
        PrintSpaces(6); 
        DebugSerial.print(s.priority); 
    }
    else
    {
        DebugSerial.print("-");
    }
    DebugSerial.println();
}

void PrintStartEngineSound(uint8_t slot)
{
    if (DEBUG) 
    { 
        Serial.print(F("Start playing ")); 
        for (uint8_t i = 0; i < sizeof(Engine[slot].soundFile.fileName); i++)
        {
            Serial.print(Engine[slot].soundFile.fileName[i]);
        }        
        Serial.print(F(" in Engine slot ")); 
        Serial.print(slot); 
        Serial.print(F(" (ID: ")); 
        Serial.print(Engine[slot].ID.Num); 
        Serial.print(F(")"));
        Serial.println();
    }
}

void PrintStartFx(uint8_t slot)
{
    if (DEBUG) 
    { 
        Serial.print(F("Start playing ")); 
        for (uint8_t i = 0; i < sizeof(FX[slot].soundFile.fileName); i++)
        {
            Serial.print(FX[slot].soundFile.fileName[i]);
        }        
        Serial.print(F(" in FX slot ")); 
        Serial.print(slot); 
        Serial.print(F(" (ID: ")); 
        Serial.print(FX[slot].ID.Num); 
        Serial.print(F(")"));
        Serial.println();
    }
}

void PrintStopFx(uint8_t slot)
{
    if (DEBUG) 
    { 
        Serial.print(F("Stop  playing ")); 
        for (uint8_t i = 0; i < sizeof(FX[slot].soundFile.fileName); i++)
        {
            Serial.print(FX[slot].soundFile.fileName[i]);
        }        
        Serial.print(F(" in FX slot ")); 
        Serial.print(slot); 
        Serial.print(F(" (ID: ")); 
        Serial.print(FX[slot].ID.Num); 
        Serial.print(F(")"));
        Serial.println();
    }
}

void PrintNextSqueakTime(uint8_t i)
{     
    if (DEBUG) 
    {
        DebugSerial.print(F("Squeak ")); 
        DebugSerial.print(i+1);     // Add one because the array is zero-based
        DebugSerial.print(F(" again in ")); 
        DebugSerial.print(ramcopy.squeakInfo[i].squeakAfter);
        DebugSerial.print(F(" mS")); 
        DebugSerial.println();
    }
}

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  

void PrintTrackTime(uint32_t length)
{  
int minutes = numberOfMinutes(length / 1000);
int seconds = numberOfSeconds(length / 1000);
int milliseconds = length % 1000;
int mS = round((float)milliseconds / 10.0);
    printDigits(minutes);
    DebugSerial.print(":");
    printDigits(seconds);
    DebugSerial.print(":");
    printDigits(mS);
}

void printDigits(byte digits)
{
    // utility function: prints leading 0
    if(digits < 10) DebugSerial.print('0');
    DebugSerial.print(digits,DEC);  
}

void PrintDebugLine()
{
    for (uint8_t i=0; i<45; i++) { DebugSerial.print(F("-")); }
    DebugSerial.println(); 
    DebugSerial.flush();   // This causes a pause until the serial transmission is complete
}

void PrintSpaceDash()
{
    DebugSerial.print(F(" - "));
}

void PrintSpaceBar()
{
    DebugSerial.print(F(" | "));
}

void PrintSpace()
{
    DebugSerial.print(F(" "));
}    

void PrintSpaces(uint8_t num)
{
    if (num == 0) return;
    for (uint8_t i=0; i<num; i++) { PrintSpace(); }
}

void PrintLine()
{
    DebugSerial.println();
}

void PrintLines(uint8_t num)
{    
    if (num == 0) return;
    for (uint8_t i=0; i<num; i++) { PrintLine(); }
}

void PrintTrueFalse(boolean boolVal)
{
    if (boolVal == true) { DebugSerial.print(F("TRUE")); } else { DebugSerial.print(F("FALSE")); }
}

void PrintLnTrueFalse(boolean boolVal)
{
    PrintTrueFalse(boolVal);
    DebugSerial.println();
}

void PrintYesNo(boolean boolVal)
{
    if (boolVal == true) { DebugSerial.print(F("Yes")); } else { DebugSerial.print(F("No")); }
}

void PrintLnYesNo(boolean boolVal)
{
    PrintYesNo(boolVal);
    DebugSerial.println();
}

void PrintHighLow(boolean boolVal)
{
    if (boolVal == true) { DebugSerial.println(F("HIGH")); } else { DebugSerial.println(F("LOW")); }
}

void PrintPct(uint8_t pct)
{
    DebugSerial.print(pct);
    DebugSerial.print(F("%"));
}

void PrintLnPct(uint8_t pct)
{
    PrintPct(pct);
    DebugSerial.println();
}


float Convert_mS_to_Sec(int mS)
{
    return float(mS) / 1000.0;
}

