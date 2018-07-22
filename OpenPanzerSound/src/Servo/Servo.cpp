/* NOTE: This version of the servo library has been stripped of unnecessary options and implements only the Low Power Timer option
 * for Teensy 3.x devices. This is because the default PDB timer that Servo would typically use for Teensy 3.2 interferes with
 * the Audio library. 
 * If you want to use servos in your own Teens projects, reference the full Servo library included in TeensyDuino, not this version. 
 * 
 * July 2018, Open Panzer
 *
 */

/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 
 A servo is activated by creating an instance of the Servo class passing the desired pin to the attach() method.
 The servos are pulsed in the background using the value most recently written using the write() method
 
 Note that analogWrite of PWM on pins associated with the timer are disabled when the first servo is attached.
 Timers are seized as needed in groups of 12 servos - 24 servos use two timers, 48 servos will use four.
 
 The methods are:
 
 Servo - Class for manipulating servo motors connected to Arduino pins.
 
 attach(pin )  - Attaches a servo motor to an i/o pin.
 attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
 default min is 544, max is 2400  
 
 write()     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
 writeMicroseconds() - Sets the servo pulse width in microseconds 
 read()      - Gets the last written servo pulse width as an angle between 0 and 180. 
 readMicroseconds()   - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
 attached()  - Returns true if there is a servo attached. 
 detach()    - Stops an attached servos from pulsing its i/o pin. 
 
*/

// ******************************************************************************
// Teensy-LC implementation, using Low Power Timer
// ******************************************************************************

#include <Arduino.h> 
#include "Servo.h"

#define LPTMR_CONFIG     LPTMR_CSR_TIE | LPTMR_CSR_TFC | LPTMR_CSR_TEN
#define usToTicks(us)    ((us) * 8)
#define ticksToUs(ticks) ((ticks) / 8)

//#if SERVOS_PER_TIMER <= 16
static uint16_t servo_active_mask = 0;
static uint16_t servo_allocated_mask = 0;
//#else
//static uint32_t servo_active_mask = 0;
//static uint32_t servo_allocated_mask = 0;
//#endif

static uint8_t servo_pin[MAX_SERVOS];
static uint16_t servo_ticks[MAX_SERVOS];

Servo::Servo()
{
	uint16_t mask;

	servoIndex = 0;
	for (mask=1; mask < (1<<MAX_SERVOS); mask <<= 1) {
		if (!(servo_allocated_mask & mask)) {
			servo_allocated_mask |= mask;
			servo_active_mask &= ~mask;
			return;
		}
		servoIndex++;
	}
	servoIndex = INVALID_SERVO;
}

uint8_t Servo::attach(int pin)
{
	return attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int minimum, int maximum)
{
	if (servoIndex < MAX_SERVOS) {
		pinMode(pin, OUTPUT);
		servo_pin[servoIndex] = pin;
		servo_ticks[servoIndex] = usToTicks(DEFAULT_PULSE_WIDTH);
		servo_active_mask |= (1<<servoIndex);
		min_ticks = usToTicks(minimum);
		max_ticks = usToTicks(maximum);
		if (!(SIM_SCGC5 & SIM_SCGC5_LPTIMER)) {
			SIM_SCGC5 |= SIM_SCGC5_LPTIMER; // TODO: use BME
			OSC0_CR |= OSC_ERCLKEN;
			LPTMR0_CSR = 0;
			LPTMR0_PSR = LPTMR_PSR_PRESCALE(0) | LPTMR_PSR_PCS(3); // 8 MHz
			LPTMR0_CMR = 1;
			LPTMR0_CSR = LPTMR_CONFIG;
			NVIC_SET_PRIORITY(IRQ_LPTMR, 32);
		}
		NVIC_ENABLE_IRQ(IRQ_LPTMR);
	}
	return servoIndex;
}

void Servo::detach()  
{
	if (servoIndex >= MAX_SERVOS) return;
	servo_active_mask &= ~(1<<servoIndex);
	servo_allocated_mask &= ~(1<<servoIndex);
	if (servo_active_mask == 0) {
		NVIC_DISABLE_IRQ(IRQ_LPTMR);
	}
}

void Servo::write(int value)
{
	if (servoIndex >= MAX_SERVOS) return;
	if (value >= MIN_PULSE_WIDTH) {
		writeMicroseconds(value);
		return;
	} else if (value > 180) {
		value = 180;
	} else if (value < 0) {
		value = 0;
	}
	if (servoIndex >= MAX_SERVOS) return;
	servo_ticks[servoIndex] = map(value, 0, 180, min_ticks, max_ticks);
}

void Servo::writeMicroseconds(int value)
{
	value = usToTicks(value);
	if (value < min_ticks) {
		value = min_ticks;
	} else if (value > max_ticks) {
		value = max_ticks;
	}
	if (servoIndex >= MAX_SERVOS) return;
	servo_ticks[servoIndex] = value;
}

int Servo::read() // return the value as degrees
{
	if (servoIndex >= MAX_SERVOS) return 0;
	return map(servo_ticks[servoIndex], min_ticks, max_ticks, 0, 180);     
}

int Servo::readMicroseconds()
{
	if (servoIndex >= MAX_SERVOS) return 0;
	return ticksToUs(servo_ticks[servoIndex]);
}

bool Servo::attached()
{
	if (servoIndex >= MAX_SERVOS) return 0;
	return servo_active_mask & (1<<servoIndex);
}

void lptmr_isr(void)
{
	static int8_t channel=0, channel_high=MAX_SERVOS;
	static uint32_t tick_accum=0;
	uint32_t ticks;
	int32_t wait_ticks;

	// first, if any channel was left high from the previous
	// run, now is the time to shut it off
	if (servo_active_mask & (1<<channel_high)) {
		digitalWrite(servo_pin[channel_high], LOW);
		channel_high = MAX_SERVOS;
	}
	// search for the next channel to turn on
	while (channel < MAX_SERVOS) {
		if (servo_active_mask & (1<<channel)) {
			digitalWrite(servo_pin[channel], HIGH);
			channel_high = channel;
			ticks = servo_ticks[channel];
			tick_accum += ticks;
			LPTMR0_CMR += ticks;
			LPTMR0_CSR = LPTMR_CONFIG | LPTMR_CSR_TCF;
			channel++;
			return;
		}
		channel++;
	}
	// when all channels have output, wait for the
	// minimum refresh interval
	wait_ticks = usToTicks(REFRESH_INTERVAL) - tick_accum;
	if (wait_ticks < usToTicks(100)) wait_ticks = usToTicks(100);
	else if (wait_ticks > 60000) wait_ticks = 60000;
	tick_accum += wait_ticks;
	LPTMR0_CMR += wait_ticks;
	LPTMR0_CSR = LPTMR_CONFIG | LPTMR_CSR_TCF;
	// if this wait is enough to satisfy the refresh
	// interval, next time begin again at channel zero 
	if (tick_accum >= usToTicks(REFRESH_INTERVAL)) {
		tick_accum = 0;
		channel = 0;
	}
}









