

void HandleLight(uint8_t num, switch_action action) // Num should be 1 through NUM_LIGHTS (ie, not zero-based)
{   
    uint8_t n = 0; 
    if (num > 0) n = num - 1;   // We do however need a zero-based light number for the arrays
    
    switch (action)
    {
        case ACTION_ONSTART:        
            LED_OUTPUT[n].on();     
            if (DEBUG) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" on")); }
            break;
        
        case ACTION_OFFSTOP:        
            LED_OUTPUT[n].off();                                
            if (DEBUG) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" off")); }
            break;
        
        case ACTION_REPEATTOGGLE:   
            LED_OUTPUT[n].toggle();
            if (DEBUG) 
            { 
                if (LED_OUTPUT[n].isOn()) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" toggle on")); }
                else                      { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" toggle off"));}
            }
            break;
        
        case ACTION_STARTBLINK:     
            LED_OUTPUT[n].startBlinking(lightSettings[n].BlinkOnTime, lightSettings[n].BlinkOffTime);    
            if (DEBUG) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" start blinking")); }
            break;
        
        case ACTION_TOGGLEBLINK:    
            LED_OUTPUT[n].isBlinking() ? LED_OUTPUT[n].stopBlinking() : LED_OUTPUT[n].startBlinking(lightSettings[n].BlinkOnTime, lightSettings[n].BlinkOffTime); 
            if (DEBUG) 
            { 
                if (LED_OUTPUT[n].isBlinking()) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" start blinking"));}
                else                            { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" stop blinking")); }
            }
            break;
        
        case ACTION_FLASH:          
            LED_OUTPUT[n].Blink(lightSettings[n].FlashTime);    // Will blink once, aka, flash
            if (DEBUG) { DebugSerial.print(F("Light ")); DebugSerial.print(num); DebugSerial.println(F(" flash")); }  
            break;
        
        case ACTION_NULL:                                                               
            break;

        default:
            break;
    }
}

