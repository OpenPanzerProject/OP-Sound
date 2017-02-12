
boolean InitializeEEPROM(void)
{
    // Check if EEPROM has ever been initalized, if not, do so
    long Temp = EEPROM.readLong(offsetof(_eeprom_data, InitStamp)); // Get our EEPROM initialization stamp code
    if(Temp != EEPROM_INIT)                                         // EEPROM_INIT is defined at the top of the main sketch. It should be changed to a new number if changes have been made to _eeprom_data struct
    {   
        // The way we do this is set the values in our ramcopy struct to default, then write the entire struct to EEPROM (actually "update" instead of "write")
        Initialize_RAMcopy();                                       // Set RAM variables to sensible defaults
        ramcopy.InitStamp = EEPROM_INIT;                            // Set the InitStamp
        EEPROM.updateBlock(EEPROM_START_ADDRESS, ramcopy);          // Now write it all to EEPROM. We use the "update" function so as not to unnecessarily writebytes that haven't changed. 
        return true;                                                // If we initialized EEPROM, return true
    }
    else
    {
        // In this case, the values in EEPROM are what we want. We load them all to RAM
        loadRAMcopy();
        return false;
    }
}

// This takes all variables from the eeprom struct, and puts them into the RAM copy struct
void loadRAMcopy(void)
{
    EEPROM.readBlock(EEPROM_START_ADDRESS, ramcopy);
}

// If EEPROM has not been used before, we initialize to some sensible default values
void Initialize_RAMcopy(void) 
{       
    // Squeak settings
    for (uint8_t i=0; i<NUM_SQUEAKS; i++)
    {
        ramcopy.squeakInfo[i].intervalMin = 1000;
        ramcopy.squeakInfo[i].intervalMax = 4000;
        ramcopy.squeakInfo[i].enabled = false;
        ramcopy.squeakInfo[i].active = false;
        ramcopy.squeakInfo[i].lastSqueak = 0;
        ramcopy.squeakInfo[i].squeakAfter = 0;
    }
}

