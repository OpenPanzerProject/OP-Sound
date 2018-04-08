
void ClearFunctionTriggers()
{
    for (uint8_t i=0; i<MAX_FUNCTION_TRIGGERS; i++)
    {
        SF_Trigger[i].TriggerID = 0;
        SF_Trigger[i].FunctionID = SF_NULL_FUNCTION;
    }
    triggerCount = 0;
}

void DefaultFunctionTriggers()
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
        SF_Trigger[0].TriggerID = 21;       // Channel 2, position 1 of 3 (all channels were initialized to 3 position by default)
        SF_Trigger[0].FunctionID = function_id_other_function_start_range + SF_ENGINE_START;
    //  Engine Stop
        SF_Trigger[1].TriggerID = 23;       // Channel 2, position 3 of 3 
        SF_Trigger[1].FunctionID = function_id_other_function_start_range + SF_ENGINE_STOP;
    //  Cannon Fire        
        SF_Trigger[2].TriggerID = 31;       // Channel 3, position 1 of 3 
        SF_Trigger[2].FunctionID = function_id_other_function_start_range + SF_CANNON_FIRE;
    //  MG Fire and stop
        SF_Trigger[3].TriggerID = 33;       // Channel 3, position 3 of 3 
        SF_Trigger[3].FunctionID = function_id_other_function_start_range + SF_MG_FIRE;
        SF_Trigger[4].TriggerID = 32;       // Channel 3, position 2 of 3 
        SF_Trigger[4].FunctionID = function_id_other_function_start_range + SF_MG_STOP;
    //  User Sound 1
        SF_Trigger[5].TriggerID = 41;       // Channel 4, position 1 of 3 
        SF_Trigger[5].FunctionID = (1 * function_id_usersound_multiplier) + SOUND_PLAY; // Play User Sound 1
    //  User Sound 2
        SF_Trigger[6].TriggerID = 43;       // Channel 4, position 3 of 3 
        SF_Trigger[6].FunctionID = (2 * function_id_usersound_multiplier) + SOUND_PLAY; // Play User Sound 2
    // Defined trigger count
        triggerCount = 7; 
}

void LoadIniSettings(void)
{
    // Get each channel's input type - we are checking here for an analog function. 
    // If it exists we will save the function and the channel will be prevented from being a switch by having Digital set to false
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
                uint16_t functionID = 0;
                uint16_t triggerID = (trigger_id_multiplier_rc_channel * (i+1)) + (j + 1);
                char cstr[3];   // Two digits plus terminating null character
                if (ini.getValue("channel_pos_triggers",itoa(triggerID,cstr,10), buffer, bufferLen, functionID))
                {   
                    if (triggerCount <= MAX_FUNCTION_TRIGGERS)
                    {
                        SF_Trigger[triggerCount].TriggerID = triggerID;
                        SF_Trigger[triggerCount].FunctionID = functionID;
                        triggerCount += 1;
                    }
                }
            }
        }
    }
}

void PrintRCFunctions(void)
{
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        Serial.print(F("Channel ")); 
        Serial.print(i+1);
        Serial.print(F(": ")); 
        if (RC_Channel[i].Digital)
        {
            Serial.print(F("Switch (")); 
            Serial.print(RC_Channel[i].numSwitchPos);
            Serial.print(F(" positions)"));
        }
        else
        {
            Serial.print(F("Variable Input (Function "));
            Serial.print(RC_Channel[i].anaFunction);
            Serial.print(F(")"));
        }
        if (RC_Channel[i].reversed)
        {
            Serial.print(F(" (Reversed)"));
        }
        Serial.println();
    }

    Serial.println();

    for (uint8_t i=0; i<triggerCount; i++)
    {
        Serial.print(F("Trigger id: ")); 
        Serial.print(SF_Trigger[i].TriggerID);
        Serial.print(F(" Function id: ")); 
        Serial.println(SF_Trigger[i].FunctionID);
    }

    Serial.println();
}


