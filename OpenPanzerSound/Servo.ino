
void servoloop() 
{ 
    uint8_t pos;
    
    for(pos = 10; pos < 170; pos += 1)  // goes from 10 degrees to 170 degrees 
    {                                   // in steps of 1 degree 
        servo.write(pos);               // tell servo to go to position in variable 'pos' 
        delay(15);                      // waits 15ms for the servo to reach the position 
    } 
    
    for(pos = 180; pos>=1; pos-=1)      // goes from 180 degrees to 0 degrees 
    {                                
        servo.write(pos);               // tell servo to go to position in variable 'pos' 
        delay(15);                      // waits 15ms for the servo to reach the position 
    } 
} 
