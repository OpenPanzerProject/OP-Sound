
void RecoilServo(void)
{   
    servo.writeMicroseconds(servoEPRecoiled_uS);            // Go to the recoiled position as fast as the servo will travel
    timer.setTimeout(timeToRecoil, RecoilHandler);          // Wait for the recoil amount of time, then slowly return to battery
}

void RecoilHandler(void)
{
    static uint8_t recoilState = 0;
    static int currentServoPos;
       
    switch (recoilState)
    {
        case 0:
            // We are just now starting the return 
            currentServoPos = servoEPRecoiled_uS;                   // Initialize the servo's current position
            currentServoPos += servoStep;                           // Adjust by step
            servo.writeMicroseconds(currentServoPos);               // Take the first step back to battery
            recoilState = 1;                                        // Advance to next state
            timer.setTimeout(updateInterval, RecoilHandler);        // Come back here after our fixed updateInterval
            break;

        case 1:
            // We are in the process of returning
            currentServoPos += servoStep;                           // Increment servo position

            // If we've reached/exceeded the battery position, stop
            if ((servoReversed && (currentServoPos >= servoEPBattery_uS)) || (!servoReversed && (currentServoPos <= servoEPBattery_uS)))
            {   
                currentServoPos = servoEPBattery_uS;
                recoilState = 2;
            }

            servo.writeMicroseconds(currentServoPos);               // Write new position

            // If we're not done yet, come back here after interval
            if (recoilState == 1) timer.setTimeout(updateInterval, RecoilHandler);  
            else recoilState = 0;                                   // Otherwise we're done, reset
            break;
    }
}

void CalculateRecoilParams(void)
{
    int posDiff;
    float numUpdates; 
    
    // First figure out how far the servo has to travel (difference between end-points)
    if (servoReversed) posDiff = servoEPBattery_uS  - servoEPRecoiled_uS;
    else               posDiff = servoEPRecoiled_uS - servoEPBattery_uS; 

    // This is how many times we will update the servo position (timeToRecoil / fixed update interval)
    numUpdates = (float)timeToReturn / (float)updateInterval;

    // This is how far we will move the servo each update
    servoStep = (int)(((float)posDiff / numUpdates) + 0.5);

    // Typically we decrement the servo's position (negative number)
    if (servoReversed == false) servoStep = -servoStep;     

}

void CalculateServoEndPoints(void)
{
    float diff_recoil;
    float diff_batt;
        
    // End point adjustment
    diff_recoil = ((float)servoEndPointRecoiled - 100.0) * 5.0;     // Calculate amount over/under 100, convert to percent, multiply by half of standard servo travel (500)
    diff_batt =   ((float)servoEndPointBattery  - 100.0) * 5.0;     // Calculate amount over/under 100, convert to percent, multiply by half of standard servo travel (500)

    if (servoReversed)
    {
        servoEPRecoiled_uS = (int)(1000.0 + diff_recoil + 0.5);     // Apply adjustment and round to nearest integer
        servoEPBattery_uS  = (int)(2000.0 + diff_batt   + 0.5);
    }
    else
    {
        servoEPRecoiled_uS = (int)(2000.0 + diff_recoil + 0.5);
        servoEPBattery_uS  = (int)(1000.0 + diff_batt   + 0.5);
    }
}


