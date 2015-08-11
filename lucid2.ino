/*********************************************
lucid13.c
V 0.2b

Lucid dreaming device

Software by gmoon (Doug Garmon)
Hardware by guyfrom7up (Brian _)

* Chip type : ATtiny13
* Clock frequency : Internal clock 1.2 Mhz
*********************************************/
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define F_CPU 1200000UL // 1.2 MHz default clock

// ramping fx increments
#define PWM_VAL 4
#define TRANS_VAL 6

// the overall pulse width and delay between pulses
#define MACRO_WIDTH 1500
#define MACRO_GAP 1500

// mode
#define MODE_WAITING 0
#define MODE_DREAM 1
#define WAIT_LENGTH 1

#define BUTTON PB4 // button
#define LED _BV(PB1) | _BV(PB2) // LED output

volatile uint16_t macropulse;
volatile uint16_t waitstate;

// IRQ vector
ISR (TIM0_OVF_vect)
{
static uint8_t modeflag;
static uint8_t pwm;
static uint16_t transition;

switch (modeflag) {

case MODE_DREAM:
if(macropulse < MACRO_WIDTH)
{
pwm += PWM_VAL;
if (pwm > transition)
PORTB &= ~(LED); // turn off LEDs
else
PORTB |= LED; // turn on

if (!pwm)
transition += TRANS_VAL;
}
// delay between pulses
else
{
pwm = transition = 0;
PORTB &= ~(LED); // turn off
if (macropulse > (MACRO_GAP + MACRO_WIDTH))
macropulse = 0;
}

macropulse++;
break;

case MODE_WAITING:
macropulse++;
if (!macropulse)
waitstate++;

if (waitstate >= WAIT_LENGTH)
{
modeflag = MODE_DREAM;
TCCR0B = _BV(CS00); // new prescaler
}
break;
}
}

// init the IRQ
void irqinit (void)
{
// timer scaler value
TCCR0B |= (_BV(CS02) | _BV(CS00));

// Enable timer overflow irq
TIMSK0 = _BV (TOIE0);
sei (); // IRQ on
}


int main(void)
{
uint8_t userflag;

/* INITIALIZE */
DDRB &= ~_BV(DDB4); // clear bit, input fire-button
PORTB |= _BV(BUTTON); // set bit, enable pull-up resistor

DDRB |= _BV(DDB1) | _BV(DDB2); // set output
//PORTB &= ~(LED); // set bits off

// Check if button is pressed when powering up...
if(!(PINB & _BV(BUTTON)))
userflag = 1;

// init the IRQ
irqinit();

// turn on LED,
// delay before checking user input again
while(macropulse < 14)
{
PORTB |= (LED);
}
PORTB &= ~(LED);

// button still pressed? Enter immediate mode...
if(!(PINB & _BV(BUTTON)) && userflag)
waitstate = WAIT_LENGTH;

// place the CPU into idle mode
set_sleep_mode (SLEEP_MODE_IDLE);
sleep_mode ();

// infinite loop--the IRQ does all the work...
while(1) {
}

return(0);
}
