
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// SERIAL COMMANDS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

boolean CheckSerial(void)
{
    byte ByteIn;
    static char input_line[SENTENCE_BYTES];     // An array to store incoming bytes
    boolean SentenceReceived = false;           // Start off false, will get set to true if a valid sentence was received
    static boolean addressReceived = false;     // Have we received a byte that matches our address
    static uint8_t numBytes = 0;                // Start off with no data received
    static DataSentence Sentence;               // A struct to store incoming commands

    // Read all the bytes that are available, starting with the first byte that matches our address
    while(CommSerial.available())               
    {
        ByteIn = CommSerial.read();
        if (ByteIn == OPSC_ADDRESS)
        {
            addressReceived = true;         // Matching address
            input_line[0] = ByteIn;         // Save it in our array
            numBytes = 1;                   // Subsequent bytes will be added to the array until we have enough to compare against INIT_STRING
        }
        else if (addressReceived)
        {
            input_line[numBytes++] = ByteIn;
            if (numBytes >= SENTENCE_BYTES) break;  // We have enough bytes for a full sentence, so evaluate it
        }
    }

    // If we have enough bytes for a full sentence, save it
    if (numBytes >= SENTENCE_BYTES)
    {   // We have enough bytes for a full sentence
        Sentence.Address  = input_line[0];
        Sentence.Command  = input_line[1];
        Sentence.Value    = input_line[2];
        Sentence.Modifier = input_line[3];
        Sentence.Checksum = input_line[4];

        // Now verify the checksum
        if (ChecksumValid(&Sentence))
        {
            SentenceReceived = true;        // Yes, a valid sentence has been received!  
            TimeLastSerial = millis();      // Save the time
            BlueLed.on();                   // Set the Blue LED on whenever serial is received
            ProcessCommand(&Sentence);      // Do whatever we're told
        }

        // Start everything over
        input_line[0] = '\0';
        addressReceived = false;
        numBytes = 0;
    }

    return SentenceReceived;
}

boolean ChecksumValid(DataSentence * sentence)
{
    uint8_t check = (sentence->Address + sentence->Command + sentence->Value + sentence->Modifier) & B01111111;

    if (check == sentence->Checksum) return true;
    else                             return false;
}

void ProcessCommand(DataSentence * sentence)
{
    switch (sentence->Command)
    {
        case OPSC_CMD_SERIAL_WATCHDOG:                                                          break;  // Not implemented for now
        case OPSC_CMD_BAUD_RATE:            SetCommBaudRate(sentence->Value);                   break;
        case OPSC_CMD_ENGINE_START:         StartEngine();                                      break;
        case OPSC_CMD_ENGINE_STOP:          StopEngine();                                       break;
        case OPSC_CMD_ENGINE_SET_SPEED:     SetEngineSpeed(sentence->Value);                    break;
        case OPSC_CMD_ENGINE_SET_IDLE:      SetEngineSpeed(0);                                  break;        
        case OPSC_CMD_REPAIR_START:         Repair(true);                                       break;  // "true"  to "start" Repair sound
        case OPSC_CMD_REPAIR_STOP:          Repair(false);                                      break;  // "false"  to "stop" Repair sound
        case OPSC_CMD_CANNON:               CannonFire();                                       break;
        case OPSC_CMD_CANNON_HIT:           CannonHit();                                        break;
        case OPSC_CMD_TANK_DESTROYED:       Destroyed();                                        break;
        case OPSC_CMD_MG_START:             MG(true);                                           break;  // "true"  to "start" MG sound
        case OPSC_CMD_MG_STOP:              MG(false);                                          break;  // "false" to "stop" MG sound
        case OPSC_CMD_MG_HIT:               MGHit();                                            break;
        case OPSC_CMD_TURRET_START:         TurretRotation(true);                               break;  // "true"  to "start" turret sound
        case OPSC_CMD_TURRET_STOP:          TurretRotation(false);                              break;  // "false" to "stop" turret sound
        case OPSC_CMD_BARREL_START:         BarrelElevation(true);                              break;  // "true"  to "start" barrel sound
        case OPSC_CMD_BARREL_STOP:          BarrelElevation(false);                             break;  // "false"  to "stop" barrel sound
        case OPSC_CMD_HEADLIGHT:            LightSwitch();                                      break;        
        case OPSC_CMD_USER_SOUND_PLAY:      PlayUserSound(sentence->Modifier, true, false);     break;  // Modifier indicates which sound, "true" for "start", "false" for "don't repeat"
        case OPSC_CMD_USER_SOUND_REPEAT:    PlayUserSound(sentence->Modifier, true, true);      break;  // Modifier indicates which sound, "true" for "start", "true" for "repeat"
        case OPSC_CMD_USER_SOUND_STOP:      PlayUserSound(sentence->Modifier, false, false);    break;  // Modifier indicates which sound, "false" for "stop", repeat argument irrelvant
        case OPSC_CMD_SQUEAKS_START:        StartSqueaks();                                     break;        
        case OPSC_CMD_SQUEAKS_STOP:         StopSqueaks();                                      break;        
        case OPSC_CMD_SQUEAK_SET_MIN:       SetSqueakMin(sentence->Value, sentence->Modifier);  break;
        case OPSC_CMD_SQUEAK_SET_MAX:       SetSqueakMax(sentence->Value, sentence->Modifier);  break;
        case OPSC_CMD_SQUEAK_ENABLE:        EnableSqueak(sentence->Value, sentence->Modifier);  break;        
        case OPSC_CMD_BEEP_ONCE:            Beep(1);                                            break;        
        case OPSC_CMD_BEEP_X:               Beep(sentence->Value);                              break;        
        case OPSC_CMD_SET_VOLUME:           UpdateVolume_Serial(sentence->Value);               break;
        default:
            break;
    }
}

void SetCommBaudRate(uint8_t val)
{
    // Change baud rate for the comm port to the TCB (not USB!)
    // If valid value passed, re-start the comm hardware port at the selected baud rate
    switch (val)
    {
        case OPSC_BAUD_CODE_2400:    CommSerial.begin(2400);     break;
        case OPSC_BAUD_CODE_9600:    CommSerial.begin(9600);     break;
        case OPSC_BAUD_CODE_19200:   CommSerial.begin(19200);    break;
        case OPSC_BAUD_CODE_38400:   CommSerial.begin(38400);    break;
        case OPSC_BAUD_CODE_115200:  CommSerial.begin(115200);   break;
        case OPSC_BAUD_CODE_57600:   CommSerial.begin(57600);    break;
    }
}


