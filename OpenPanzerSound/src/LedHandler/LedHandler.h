/* OP_LedHandler.h      Open Panzer Led Handler - class for handling LEDs including blink effects, requires use of elapsedMillis
 * Source:              openpanzer.org              
 * Authors:             Luke Middleton
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */  

#ifndef OPS_LedHandler_h
#define OPS_LedHandler_h

#include <Arduino.h>

#define DEFAULT_BLINK_INTERVAL           200
#define MAX_STREAM_STEPS                  10            // A stream consists of a pattern of on/off blinks separated by user-specified lengths of time. A single blink (on/off) takes 2 steps. 
typedef struct                                          // This struct holds an array of blink patterns, and a flag to indicate if it should repeat or not
{
    uint16_t        interval[MAX_STREAM_STEPS];
    boolean         repeat;
} BlinkStream;

// Generic blink defines
#define BLINK_RATE_HEARTBEAT        750         // How often to blink the light in heartbeat mode, in milliseconds
#define BLINK_RATE_LOST_SIGNAL      40          // Blink rate when radio signal lost

class OPS_LedHandler
{   public:
        OPS_LedHandler() {}; 
        
        void begin (byte p, boolean i=false); 
        void on(void);
        void off(void);
        void toggle(void);
        void update(void);                                                      // Update blinking effect
        boolean isBlinking(void);
        void ExpireIn(uint16_t); 
        void Blink(uint16_t interval=DEFAULT_BLINK_INTERVAL);                   // Blinks once at set interval
        void Blink(uint8_t times, uint16_t  interval=DEFAULT_BLINK_INTERVAL);   // Overload - Blinks N times at set interval
        void blinkHeartBeat(void);
        void blinkLostSignal(void);
        void startBlinking(uint16_t on_interval=DEFAULT_BLINK_INTERVAL, uint16_t off_interval=DEFAULT_BLINK_INTERVAL);   // Starts a continuous blink at the set intervals, to stop call stopBlinking();
        void stopBlinking(void);
        void DoubleTap(boolean repeat=false);
        void TripleTap(boolean repeat=false);
        void QuadTap(boolean repeat=false);
        void StreamBlink(BlinkStream bs, uint8_t numSteps);
        
    private:
        void ClearBlinker(void);
        void pinOn(void);
        void pinOff(void);
        elapsedMillis   _time;
        elapsedMillis   _expireMe;
        uint16_t        _expireTime; 
        byte            _pin;
        boolean         _invert;
        boolean         _isBlinking;
        uint8_t         _curStep;
        uint8_t         _numSteps;
        uint16_t        _nextWait;
        boolean         _fixedInterval;
        BlinkStream     _blinkStream;
      
};


#endif 
