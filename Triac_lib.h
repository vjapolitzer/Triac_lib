/**********************************************************************************************
 * Arduino Triac Library - Version 1.0
 * by Vincent Politzer
 * 
 * This Library is licensed under the MIT License
 **********************************************************************************************/

#ifndef TRIAC_LIB_H
#define TRIAC_LIB_H

#include "Arduino.h"

//Maximum number of triacs that can be instantiated
#define MAX_TRIAC 2
// 50Hz not tested, but should still work okay
#define AC_FREQUENCY 60
// The external interrupt pin - DO NOT CHANGE
#define TRIAC_ZERO_CROSS_PIN 2
// Cycles for propagation delay of triac driver
#define TRIAC_ON_CYCLES 18

// Macros for timer configuration
#define _TCNT(X) TCNT ## X
#define TCNT(X) _TCNT(X)
#define _TCCRxA(X) TCCR ## X ## A
#define TCCRxA(X) _TCCRxA(X)
#define _TCCRxB(X) TCCR ## X ## B
#define TCCRxB(X) _TCCRxB(X)
#define _TIMSKx(X) TIMSK ## X
#define TIMSKx(X) _TIMSKx(X)
#define _OCRxA(X) OCR ## X ## A
#define OCRxA(X) _OCRxA(X)
#define _OCIExA(X) OCIE ## X ## A
#define OCIExA(X) _OCIExA(X)
#define _TIMER_COMPA_VECTOR(X) TIMER ## X ## _COMPA_vect
#define TIMER_COMPA_VECTOR(X) _TIMER_COMPA_VECTOR(X)

// Timer and interrupt
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define TRIAC_TIMER 4
// 16-bit timer - CTC mode, Prescaler = 8 => Cycle = 0.5us @ 16MHz
#define TCCRxA_VALUE 0x00
#define TCCRxB_VALUE 0x0A
// External interrupt (digital pin 2 on Arduino MEGA 2560)
#define INT_vect INT4_vect   
#define INTx INT4
#define EICRX EICRB
#define ISCx1 ISC41
#define ISCx0 ISC40

#elif defined(__AVR_ATmega32U4__)

#define TRIAC_TIMER 3
// 16-bit timer - CTC mode, Prescaler = 8 => Cycle = 0.5us @ 16MHz
#define TCCRxA_VALUE 0x00
#define TCCRxB_VALUE 0x0A
// External interrupt (digital pin 2 on Arduino Leonardo/Micro)
#define INT_vect INT1_vect
#define INTx INT1
#define EICRX EICRA
#define ISCx1 ISC11
#define ISCx0 ISC10

#else

#define TRIAC_TIMER 2
// 8-bit timer - CTC mode, Prescaler = 8 => Cycle = 0.5us @ 16MHz
#define TCCRxA_VALUE 0x02
#define TCCRxB_VALUE 0x02
// External interrupt (digital pin 2 on Arduino UNO)
#define INT_vect INT0_vect
#define INTx INT0
#define EICRX EICRA
#define ISCx1 ISC01
#define ISCx0 ISC00

#endif

class Triac
{
    public:
        /* Constructor
        * ...Parameters:
        * ......uint8_t pin -- pin for activating triac
        * ...Returns:
        * ......Nothing
        */
        Triac(uint8_t);

        /* begin()
        * ...Configures and enables interrupts and timers
        * ...Only call ONCE
        * ...Returns:
        * ......Nothing
        */
        void begin();

        /* set(...)
        * Sets the desired load power from 0-165
        * ...Parameters:
        * ......uint8_t powerLevel
        * ...Returns:
        * ......Nothing
        */
        void set(uint8_t);

        /* off()
        * Turns off load (same as set(0))
        * ...Returns:
        * ......Nothing
        */
        void off();

        /* getPower()
        * ...Returns:
        * ......Current load power setting from 0-165
        */
        uint8_t getPower();

    private:
        // Instance variables
        uint8_t triacIndex; // index of instance in static class arrays
        uint8_t powerLevel;

        // Class variables
        static uint8_t numTriacs; // number of instantiated Triacs

        // Triac pin and phase variables. Static class arrays for faster execution
        static volatile uint8_t* triacPinPorts[]; // ports and pin masks
        static uint8_t triacPinMasks[];           // for fast GPIO 
        static uint8_t phaseDelay[];              // phase delay time (in timer cycles)

        // Timer cycles since zero crossing
        static volatile uint8_t timerCycles;

        // initialize timer and interrupts
        void timerInit();
        void extIntInit();

        // Interrupt service routines
        friend void triacTimerISR();
        friend void zeroCrossISR();
};

#endif