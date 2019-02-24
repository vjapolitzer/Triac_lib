#include "Arduino.h"
#include "Triac_lib.h"

// Phase delay time table (calculated for 50us timer with 60Hz AC power)
static const uint8_t power2cycles[] PROGMEM = {
  167, 162, 160, 159, 157, 156, 155, 154, 153, 152,
  151, 150, 150, 149, 148, 147, 146, 145, 145, 144,
  143, 142, 142, 141, 140, 140, 139, 138, 138, 137,
  136, 136, 135, 134, 134, 133, 132, 132, 131, 130,
  130, 129, 129, 128, 127, 127, 126, 125, 125, 124,
  124, 123, 122, 122, 121, 121, 120, 119, 119, 118,
  118, 117, 116, 116, 115, 115, 114, 113, 113, 112,
  112, 111, 111, 110, 109, 109, 108, 108, 107, 106,
  106, 105, 105, 104, 103, 103, 102, 102, 101, 100,
  100, 99,  99,  98,  97,  97,  96,  95,  95,  94,
  93,  93,  92,  92,  91,  90,  90,  89,  88,  88,
  87,  86,  86,  85,  84,  84,  83,  82,  81,  81,
  80,  79,  78,  78,  77,  76,  75,  75,  74,  73,
  72,  71,  71,  70,  69,  68,  67,  66,  65,  64,
  63,  62,  61,  60,  59,  58,  57,  56,  55,  53,
  52,  51,  50,  48,  47,  45,  43,  41,  40,  37,
  35,  32,  29,  25,  20,  0
};

Triac::Triac(uint8_t pin) :
       powerLevel(0)
{
  if (Triac::numTriacs < MAX_TRIAC)
  {
    this->triacIndex = Triac::numTriacs++;
    Triac::triacPinPorts[this->triacIndex] = portOutputRegister(digitalPinToPort(pin));
    Triac::triacPinMasks[this->triacIndex] = digitalPinToBitMask(pin);
    Triac::phaseDelay[this->triacIndex] = pgm_read_byte(&power2cycles[0]);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

uint8_t Triac::numTriacs = 0;
volatile uint8_t Triac::timerCycles = 0;

void Triac::begin()
{
  pinMode(TRIAC_ZERO_CROSS_PIN, INPUT);
  Triac::timerInit();
  Triac::extIntInit();
}

void Triac::set(uint8_t powerLevel)
{
  if (powerLevel > 165) powerLevel = 165;

  if (powerLevel != this->powerLevel)
  {
    this->powerLevel = powerLevel;
    Triac::phaseDelay[this->triacIndex] = pgm_read_byte(&power2cycles[powerLevel]);
  }
} 

void Triac::off()
{
  Triac::set(0);
}

uint8_t Triac::getPower()
{
  return this->powerLevel;
}

void Triac::timerInit()
{// Setup timer to fire every 50us @ 60Hz
  TCNT(TRIAC_TIMER) = 0;                            // Clear timer count register
  TCCRxA(TRIAC_TIMER) = TCCRxA_VALUE;               // Timer config byte A
  TCCRxB(TRIAC_TIMER) = TCCRxB_VALUE;               // Timer config byte B
  TIMSKx(TRIAC_TIMER) = (1 << OCIExA(TRIAC_TIMER)); // Enable timer compare match interrupt
  OCRxA(TRIAC_TIMER) = 100 * 60 / AC_FREQUENCY - 1; // Timer compare value
}

void extIntInit()
{
  EICRX &= ~0xFF;                     // Clear the External Interrupt Control Register
	EIMSK |= (1 << INTx);               // Enable the external interrupt
	EICRX |= (1 << ISCx1)|(1 << ISCx0); // Configure for RISING edge
}

void triacTimerISR()
{
  Triac::timerCycles++;

  // Turn triac pins on or off as needed
  for (uint8_t i = 0; i < Triac::numTriacs; i++)
  {
    if (Triac::timerCycles >= Triac::phaseDelay[i] + TRIAC_ON_CYCLES)
      *Triac::triacPinPorts[i] &= ~Triac::triacPinMasks[i];
    else if (Triac::timerCycles >= Triac::phaseDelay[i])
      *Triac::triacPinPorts[i] |= Triac::triacPinMasks[i];
  }
}

void zeroCrossISR()
{
  // Clear the timer and counter
  TCNT(TRIAC_TIMER) = 0;
  Triac::timerCycles = 0;
}

ISR(INT_vect)
{
  zeroCrossISR();
}

ISR(TIMER_COMPA_VECTOR(TRIAC_TIMER))
{
  triacTimerISR();
}
