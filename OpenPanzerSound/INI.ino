
void ClearFunctionTriggers()
{
    for (uint8_t i=0; i<MAX_FUNCTION_TRIGGERS; i++)
    {
        SF_Trigger[i].ChannelNum = 0;
        SF_Trigger[i].ChannelPos = 0;
        SF_Trigger[i].swFunction = SF_NULL;
        SF_Trigger[i].actionNum = 1;
        SF_Trigger[i].switchAction = ACTION_OFFSTOP; 
    }
    triggerCount = 0;
}

void DefaultValues()
{
    // Clear to start
        ClearFunctionTriggers();
    
    // Analog function assignments: 
    // Default channel 1 to engine speed
        RC_Channel[0].Digital = false;
        RC_Channel[0].anaFunction = ANA_ENGINE_SPEED;
    // And channel 5 to volume
        RC_Channel[4].Digital = false;
        RC_Channel[4].anaFunction = ANA_MASTER_VOL;

    // Digital function assignments:
    //  Engine Start
        SF_Trigger[0].ChannelNum = 2;           // Channel 2, position 1 of 3 (recall all channels were initialized to 3 position by default)
        SF_Trigger[0].ChannelPos = 1;
        SF_Trigger[0].swFunction = SF_ENGINE_START;
    //  Engine Stop
        SF_Trigger[1].ChannelNum = 2;           // Channel 2, position 3 of 3 
        SF_Trigger[1].ChannelPos = 3;        
        SF_Trigger[1].swFunction = SF_ENGINE_STOP;
    //  Cannon Fire        
        SF_Trigger[2].ChannelNum = 3;           // Channel 3, position 1 of 3 
        SF_Trigger[2].ChannelPos = 1;        
        SF_Trigger[2].swFunction = SF_CANNON_FIRE;
        SF_Trigger[2].actionNum = 1;        
    //  MG Fire and stop
        SF_Trigger[3].ChannelNum = 3;           // Channel 3, position 2 of 3 
        SF_Trigger[3].ChannelPos = 2;        
        SF_Trigger[3].swFunction = SF_MG;
        SF_Trigger[3].actionNum = 2;
        SF_Trigger[3].switchAction = ACTION_OFFSTOP;
        SF_Trigger[4].ChannelNum = 3;           // Channel 3, position 3 of 3 
        SF_Trigger[4].ChannelPos = 3;        
        SF_Trigger[4].swFunction = SF_MG;
        SF_Trigger[4].actionNum = 2;
        SF_Trigger[4].switchAction = ACTION_ONSTART;
    //  User Sound 1
        SF_Trigger[5].ChannelNum = 4;           // Channel 4, position 1 of 3 
        SF_Trigger[5].ChannelPos = 1;        
        SF_Trigger[5].swFunction = SF_USER;
        SF_Trigger[5].actionNum = 1;
        SF_Trigger[5].switchAction = ACTION_ONSTART;                
    //  User Sound 2
        SF_Trigger[6].ChannelNum = 4;           // Channel 4, position 3 of 3 
        SF_Trigger[6].ChannelPos = 3;        
        SF_Trigger[6].swFunction = SF_USER;
        SF_Trigger[6].actionNum = 2;
        SF_Trigger[6].switchAction = ACTION_ONSTART;        
    // Defined trigger count
        triggerCount = 7; 

    // Squeak Defaults
        squeakInfo[0].intervalMin = 1500;
        squeakInfo[0].intervalMax = 4000;
        squeakInfo[1].intervalMin = 2000;
        squeakInfo[1].intervalMax = 5000;
        squeakInfo[2].intervalMin = 3000;
        squeakInfo[2].intervalMax = 8000;
        squeakInfo[3].intervalMin = 1000;
        squeakInfo[3].intervalMax = 2000;
        squeakInfo[4].intervalMin = 3000;
        squeakInfo[4].intervalMax = 4000;
        squeakInfo[5].intervalMin = 5000;
        squeakInfo[5].intervalMax = 6000;                
        for (uint8_t i=0; i<NUM_SQUEAKS; i++)
        {
            squeakInfo[i].enabled = false;
            squeakInfo[i].active = false;
            squeakInfo[i].lastSqueak = 0;
            squeakInfo[i].squeakAfter = 0;
        }        
        squeakMinSpeed = 25;

    // Volume defaults
        UpdateRelativeVolume(50, VC_ENGINE);
        UpdateRelativeVolume(50, VC_EFFECTS);
        UpdateRelativeVolume(50, VC_TRACK_OVERLAY);

    // Throttle settings
        ThrottleCenter = true;
        idleDeadbandPct = 5;

    // Light defaults
        for (uint8_t i=0; i<NUM_LIGHTS; i++)
        {
            lightSettings[i].BlinkOnTime = 30;
            lightSettings[i].BlinkOffTime = 30;
            lightSettings[i].FlashTime = 30;
        }

    // Sound bank auto-loop
        SoundBankA_Loop = false;
        SoundBankB_Loop = false;

    // Servo defaults
        servoReversed = false;
        timeToRecoil = 200;
        timeToReturn = 800;
        servoEndPointRecoiled = 100;
        servoEndPointBattery =  100;
        servoEPRecoiled_uS = 2000;          // Non-reversed recoiled position is 2000 uS
        servoEPBattery_uS = 1000;           // Non-reversed battery  position is 1000 uS
        CalculateServoEndPoints();          // Now calculate end-points
        CalculateRecoilParams();            // And recoil parameters

    // Engine auto-start/stop
        Engine_AutoStart = true;            // Start engine with throttle
        Engine_AutoStop = 120000;           // Auto-stop after two minutes
        ThrottleCenter = false;             // Throttle idle is at center stick

}

void LoadIniSettings(void)
{
    // Get each channel's input type - we are checking here for an analog function. 
    // If it exists we will save the function and the channel will be prevented from being a switch by having Digital set to false
    // First we have to clear the anaFunction and Digital flags though, because they might have been set in the default setup
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++) { RC_Channel[i].anaFunction = 0; RC_Channel[i].Digital = true; }
    ini.getValue("channel_func", "ch1_func", buffer, bufferLen, RC_Channel[0].anaFunction); 
    ini.getValue("channel_func", "ch2_func", buffer, bufferLen, RC_Channel[1].anaFunction);
    ini.getValue("channel_func", "ch3_func", buffer, bufferLen, RC_Channel[2].anaFunction);
    ini.getValue("channel_func", "ch4_func", buffer, bufferLen, RC_Channel[3].anaFunction);
    ini.getValue("channel_func", "ch5_func", buffer, bufferLen, RC_Channel[4].anaFunction);
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++) { if (RC_Channel[i].anaFunction > 0) RC_Channel[i].Digital = false; }

    // Get channel's reversed status
    ini.getValue("channel_reversed", "ch1_reverse", buffer, bufferLen, RC_Channel[0].reversed);
    ini.getValue("channel_reversed", "ch2_reverse", buffer, bufferLen, RC_Channel[1].reversed);
    ini.getValue("channel_reversed", "ch3_reverse", buffer, bufferLen, RC_Channel[2].reversed);
    ini.getValue("channel_reversed", "ch4_reverse", buffer, bufferLen, RC_Channel[3].reversed);
    ini.getValue("channel_reversed", "ch5_reverse", buffer, bufferLen, RC_Channel[4].reversed);

    // Get each channel's switch status if it is not an analog input
    if (RC_Channel[0].Digital) ini.getValue("channel_pos_count", "ch1_pos", buffer, bufferLen, RC_Channel[0].numSwitchPos);
    if (RC_Channel[1].Digital) ini.getValue("channel_pos_count", "ch2_pos", buffer, bufferLen, RC_Channel[1].numSwitchPos);
    if (RC_Channel[2].Digital) ini.getValue("channel_pos_count", "ch3_pos", buffer, bufferLen, RC_Channel[2].numSwitchPos);
    if (RC_Channel[3].Digital) ini.getValue("channel_pos_count", "ch4_pos", buffer, bufferLen, RC_Channel[3].numSwitchPos);
    if (RC_Channel[4].Digital) ini.getValue("channel_pos_count", "ch5_pos", buffer, bufferLen, RC_Channel[4].numSwitchPos);

    // Pick up all functions assigned to switch positions
    ClearFunctionTriggers();
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        if (RC_Channel[i].Digital)
        {
            for (uint8_t j=0; j<RC_Channel[i].numSwitchPos; j++)
            {
                uint32_t functionID = 0;        // 32 bits!
                uint16_t triggerID = (rc_channel_multiplier * (i+1)) + (j + 1);
                char cstr[3];   // Two digits plus terminating null character
                if (ini.getValue("channel_pos_triggers",itoa(triggerID,cstr,10), buffer, bufferLen, functionID))
                {   
                    if (triggerCount <= MAX_FUNCTION_TRIGGERS)
                    {
                        // Save Trigger conditions
                        SF_Trigger[triggerCount].ChannelNum = i + 1;
                        SF_Trigger[triggerCount].ChannelPos = j + 1;                        

                        // Unpack function ID into component parts (function, action, action number)
                        uint16_t func = (functionID / multiplier_switchfunction);
                        SF_Trigger[triggerCount].swFunction = static_cast<switch_function>(func); // Switch function
                        uint8_t act = (functionID - (func * multiplier_switchfunction)) / multiplier_switchaction;
                        SF_Trigger[triggerCount].switchAction = static_cast<switch_action>(act);
                        uint8_t num = (functionID - (func * multiplier_switchfunction) - (act * multiplier_switchaction));
                        SF_Trigger[triggerCount].actionNum = num;                        
                        
                        triggerCount += 1;
                    }
                }
            }
        }
    }

    // Squeak settings
    ini.getValue("squeak_settings", "s1min", buffer, bufferLen, squeakInfo[0].intervalMin);
    ini.getValue("squeak_settings", "s1max", buffer, bufferLen, squeakInfo[0].intervalMax);
    ini.getValue("squeak_settings", "s1en",  buffer, bufferLen, squeakInfo[0].enabled);
    ini.getValue("squeak_settings", "s2min", buffer, bufferLen, squeakInfo[1].intervalMin);
    ini.getValue("squeak_settings", "s2max", buffer, bufferLen, squeakInfo[1].intervalMax);
    ini.getValue("squeak_settings", "s2en",  buffer, bufferLen, squeakInfo[1].enabled);
    ini.getValue("squeak_settings", "s3min", buffer, bufferLen, squeakInfo[2].intervalMin);
    ini.getValue("squeak_settings", "s3max", buffer, bufferLen, squeakInfo[2].intervalMax);
    ini.getValue("squeak_settings", "s3en",  buffer, bufferLen, squeakInfo[2].enabled);
    ini.getValue("squeak_settings", "s4min", buffer, bufferLen, squeakInfo[3].intervalMin);
    ini.getValue("squeak_settings", "s4max", buffer, bufferLen, squeakInfo[3].intervalMax);
    ini.getValue("squeak_settings", "s4en",  buffer, bufferLen, squeakInfo[3].enabled);
    ini.getValue("squeak_settings", "s5min", buffer, bufferLen, squeakInfo[4].intervalMin);
    ini.getValue("squeak_settings", "s5max", buffer, bufferLen, squeakInfo[4].intervalMax);
    ini.getValue("squeak_settings", "s5en",  buffer, bufferLen, squeakInfo[4].enabled);
    ini.getValue("squeak_settings", "s6min", buffer, bufferLen, squeakInfo[5].intervalMin);
    ini.getValue("squeak_settings", "s6max", buffer, bufferLen, squeakInfo[5].intervalMax);
    ini.getValue("squeak_settings", "s6en",  buffer, bufferLen, squeakInfo[5].enabled);
    ini.getValue("squeak_settings", "minspeed",  buffer, bufferLen, squeakMinSpeed);

    // Volumes
    uint8_t level = 255;
    ini.getValue("volumes", "engine",  buffer, bufferLen, level);
        if (level < 255) { UpdateRelativeVolume(level, VC_ENGINE); }
        level = 255;
    ini.getValue("volumes", "effects",  buffer, bufferLen, level);
        if (level < 255) { UpdateRelativeVolume(level, VC_EFFECTS); }
        level = 255;
    ini.getValue("volumes", "overlay",  buffer, bufferLen, level);
        if (level < 255) { UpdateRelativeVolume(level, VC_TRACK_OVERLAY); }
        level = 255;                

    // Light Settings
    ini.getValue("lights", "l1_flash", buffer, bufferLen, lightSettings[0].FlashTime);
    ini.getValue("lights", "l1_blinkon", buffer, bufferLen, lightSettings[0].BlinkOnTime);
    ini.getValue("lights", "l1_blinkoff", buffer, bufferLen, lightSettings[0].BlinkOffTime);
    ini.getValue("lights", "l2_flash", buffer, bufferLen, lightSettings[1].FlashTime);
    ini.getValue("lights", "l2_blinkon", buffer, bufferLen, lightSettings[1].BlinkOnTime);
    ini.getValue("lights", "l2_blinkoff", buffer, bufferLen, lightSettings[1].BlinkOffTime);
    ini.getValue("lights", "l3_flash", buffer, bufferLen, lightSettings[2].FlashTime);
    ini.getValue("lights", "l3_blinkon", buffer, bufferLen, lightSettings[2].BlinkOnTime);
    ini.getValue("lights", "l3_blinkoff", buffer, bufferLen, lightSettings[2].BlinkOffTime);        

    // Sound bank auto-loop
    ini.getValue("soundbank", "SBA_loop", buffer, bufferLen, SoundBankA_Loop);
    ini.getValue("soundbank", "SBB_loop", buffer, bufferLen, SoundBankB_Loop);

    // Servo settings
    ini.getValue("servo", "servo_reverse", buffer, bufferLen, servoReversed);    
    ini.getValue("servo", "time_recoil", buffer, bufferLen, timeToRecoil);
    ini.getValue("servo", "time_return", buffer, bufferLen, timeToReturn);
    ini.getValue("servo", "ep_recoil", buffer, bufferLen, servoEndPointRecoiled);
    ini.getValue("servo", "ep_battery", buffer, bufferLen, servoEndPointBattery);
    // Now calculate end-points and recoil parameters
    CalculateServoEndPoints();
    CalculateRecoilParams();
    // Now set the servo to battery
    servo.writeMicroseconds(servoEPBattery_uS);

    // Engine auto-start / auto-stop
    ini.getValue("engine", "autostart", buffer, bufferLen, Engine_AutoStart);
    ini.getValue("engine", "autostop", buffer, bufferLen, Engine_AutoStop);

    // Throttle stick configuration
    ini.getValue("throttle", "centerthrottle", buffer, bufferLen, ThrottleCenter);
    ini.getValue("throttle", "idledeadband", buffer, bufferLen, idleDeadbandPct);
    // Calculate the actual value of idleDeadband from the percentage
    idleDeadband = (int)((((float)idleDeadbandPct/100.0)*255.0)+0.5);
}

void PrintVolumes(void)
{
    DebugSerial.println(F("Volumes")); 
    DebugSerial.print(F("Engine volume: "));
    DebugSerial.println((uint8_t)(fVols[0]*100.0));
    DebugSerial.print(F("Effects volume: "));
    DebugSerial.println((uint8_t)(fVols[1]*100.0));
    DebugSerial.print(F("Track Overlay volume: "));
    DebugSerial.println((uint8_t)(fVols[2]*100.0));
    DebugSerial.println();
}

void PrintSqueakSettings(void)
{
    DebugSerial.println(F("Squeak Settings")); 
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        DebugSerial.print(F("Squeak "));
        DebugSerial.print(i + 1);
        DebugSerial.print(F(" Min="));
        DebugSerial.print(squeakInfo[i].intervalMin);
        DebugSerial.print(F(" Max="));
        DebugSerial.print(squeakInfo[i].intervalMax);
        DebugSerial.print(F(" Enabled="));
        PrintYesNo(squeakInfo[i].enabled);
        DebugSerial.println();
    }
    DebugSerial.print(F("Squeak min speed="));
    DebugSerial.println(squeakMinSpeed);
    DebugSerial.println();
}

void PrintLightSettings(void)
{
    for (uint8_t i = 0; i < NUM_LIGHTS; i++)
    {
        DebugSerial.print(F("Light ")); 
        DebugSerial.print(i+1); 
        DebugSerial.print(F(" Flash Time: ")); 
        DebugSerial.print(lightSettings[i].FlashTime);
        DebugSerial.print(F(" Blink On Time: ")); 
        DebugSerial.print(lightSettings[i].BlinkOnTime);
        DebugSerial.print(F(" Blink Off Time: ")); 
        DebugSerial.print(lightSettings[i].BlinkOffTime);
        DebugSerial.println();            
    }
    DebugSerial.println();
}

void PrintSoundBankSettings(void)
{
    DebugSerial.print(F("Sound Bank A: "));
    if (SoundBank_AnyExist(SOUNDBANK_A))
    {
        DebugSerial.print(F("Present (Auto-loop: "));
        PrintYesNo(SoundBankA_Loop);
        DebugSerial.println(F(")"));
    }
    else DebugSerial.println(F("no files found"));
    
    DebugSerial.print(F("Sound Bank B: "));
    if (SoundBank_AnyExist(SOUNDBANK_B)) 
    {
        DebugSerial.print(F("Present (Auto-loop: "));
        PrintYesNo(SoundBankB_Loop);
        DebugSerial.println(F(")"));
    }
    else DebugSerial.println(F("no files found"));
    DebugSerial.println();
}

void PrintThrottleSettings(void)
{
    DebugSerial.println(F("Throttle/Engine Settings")); 
    
    DebugSerial.print(F("Auto Start Engine with Throttle: ")); 
    PrintLnYesNo(Engine_AutoStart);
    
    DebugSerial.print(F("Auto Stop Engine After Time at Idle: ")); 
    if (Engine_AutoStop > 0)    
    {
        DebugSerial.print(((float)Engine_AutoStop/60000.0),1);
        DebugSerial.println(F(" minutes"));
    }
    else
    {
        DebugSerial.println(F("N/A"));
    }

    DebugSerial.print(F("Throttle Stick Idle at: ")); 
    if (ThrottleCenter) DebugSerial.println(F("Stick Center")); 
    else                DebugSerial.println(F("Stick End")); 
    DebugSerial.print(F("Idle Deadband: "));
    DebugSerial.print(idleDeadbandPct);
    DebugSerial.println(F("%"));

    DebugSerial.println();
}

void PrintServoSettings(void)
{
    DebugSerial.println(F("Servo Settings"));
    DebugSerial.print(F("Servo Reversed: ")); 
    PrintLnYesNo(servoReversed);
    DebugSerial.print(F("Time to recoil: ")); 
    DebugSerial.print(timeToRecoil);
    DebugSerial.println(F(" mS")); 
    DebugSerial.print(F("Time to return: ")); 
    DebugSerial.print(timeToReturn);
    DebugSerial.println(F(" mS"));     
    DebugSerial.print(F("End points: ")); 
    DebugSerial.print(servoEndPointRecoiled);
    DebugSerial.print(F("% recoiled (")); 
    DebugSerial.print(servoEPRecoiled_uS);
    DebugSerial.print(F(" uS), "));
    DebugSerial.print(servoEndPointBattery);
    DebugSerial.print(F("% battery ("));
    DebugSerial.print(servoEPBattery_uS);
    DebugSerial.println(F(" uS)"));    
    DebugSerial.println();

}

void PrintRCFunctions(void)
{
    DebugSerial.println(F("Channel Types")); 
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        DebugSerial.print(F("Channel ")); 
        DebugSerial.print(i+1);
        DebugSerial.print(F(": ")); 
        if (RC_Channel[i].Digital)
        {
            DebugSerial.print(F("Switch (")); 
            DebugSerial.print(RC_Channel[i].numSwitchPos);
            DebugSerial.print(F(" positions)"));
        }
        else
        {
            DebugSerial.print(F("Variable Input - "));
            switch (RC_Channel[i].anaFunction)
            {
                case ANA_NULL_FUNCTION: DebugSerial.print(F("Null"));             break;
                case ANA_ENGINE_SPEED:  DebugSerial.print(F("Engine Speed"));     break;
                case ANA_MASTER_VOL:    DebugSerial.print(F("Master Volume"));    break;
                default:                DebugSerial.print(F("Unknown"));          break;
            }
        }
        if (RC_Channel[i].reversed)
        {
            DebugSerial.print(F(" (Channel Reversed)"));
        }
        DebugSerial.println();
    }
    DebugSerial.println();

    DebugSerial.println(F("Channel Triggers")); 
    for (uint8_t i=0; i<triggerCount; i++)
    {
        DebugSerial.print(F("Channel "));
        DebugSerial.print(SF_Trigger[i].ChannelNum);
        DebugSerial.print(F(" Pos "));
        DebugSerial.print(SF_Trigger[i].ChannelPos);
        DebugSerial.print(F(" - "));
        PrintSwitchFunctionName(SF_Trigger[i].swFunction, SF_Trigger[i].actionNum, SF_Trigger[i].switchAction);
        DebugSerial.println();
    }
    DebugSerial.println();
}

void PrintSwitchFunctionName(switch_function f, uint8_t num, switch_action act)
{
    switch (f)
    {
        case SF_ENGINE_START:   DebugSerial.print(F("Engine Start"));    break;
        case SF_ENGINE_STOP:    DebugSerial.print(F("Engine Stop"));     break;
        case SF_ENGINE_TOGGLE:  DebugSerial.print(F("Engine Toggle"));   break;
        case SF_CANNON_FIRE:    
            DebugSerial.print(F("Cannon ")); 
            DebugSerial.print(num);
            DebugSerial.print(F(" Fire")); 
            break;
        case SF_MG:             
            DebugSerial.print(F("Machine Gun "));    
            DebugSerial.print(num);
            switch(act)
            {
                case ACTION_ONSTART:        DebugSerial.print(F(" Start"));  break;
                case ACTION_OFFSTOP:        DebugSerial.print(F(" Stop"));   break;
                case ACTION_REPEATTOGGLE:   DebugSerial.print(F(" Toggle")); break;
                default: break;
            }
            break;
        case SF_LIGHT:          
            DebugSerial.print(F("Light "));
            DebugSerial.print(num);
            switch(act)
            {
                case ACTION_ONSTART:        DebugSerial.print(F(" On"));            break;
                case ACTION_OFFSTOP:        DebugSerial.print(F(" Off"));           break;
                case ACTION_REPEATTOGGLE:   DebugSerial.print(F(" Toggle"));        break;
                case ACTION_STARTBLINK:     DebugSerial.print(F(" Start blinking")); break;
                case ACTION_TOGGLEBLINK:    DebugSerial.print(F(" Toggle blinking")); break;
                case ACTION_FLASH:          DebugSerial.print(F(" Flash"));         break;
                default: break;
            }
            break;        
        case SF_USER:           
            DebugSerial.print(F("User Sound "));     
            DebugSerial.print(num);
            switch(act)
            {
                case ACTION_ONSTART:        DebugSerial.print(F(" Play"));          break;
                case ACTION_OFFSTOP:        DebugSerial.print(F(" Stop"));          break;
                case ACTION_REPEATTOGGLE:   DebugSerial.print(F(" Repeat"));        break;
                default: break;
            }
            break;
        case SF_SOUNDBANK:           
            DebugSerial.print(F("Sound Bank "));     
            if (num == 1) DebugSerial.print(F("A"));
            else          DebugSerial.print(F("B"));
            switch(act)
            {
                case ACTION_ONSTART:        DebugSerial.print(F(" Play/Stop"));     break;
                case ACTION_PLAYNEXT:       DebugSerial.print(F(" Play Next"));     break;
                case ACTION_PLAYPREV:       DebugSerial.print(F(" Play Previous")); break;
                case ACTION_PLAYRANDOM:     DebugSerial.print(F(" Play Random"));   break;
                default: break;
            }
            break;

        default:
            break;
    }
}



