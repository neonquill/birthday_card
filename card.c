#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

void
setup(void) {
  // LED on pin 5 = PB0.
  // LED Pin as output, initially 0, the rest as inputs with pullups enabled.
  DDRB = 0x01;
  PORTB = 0x3e;

  /*
   * Setup PWM.
   * LED is on PB0 which is OC0A.
   * WGM[2:0] = 011 = Fast PWM with top at 0xff.
   * COM0A[1:0] = 11 = Set OC0A on compare match, clear at top.
   * CS0[2:0] = 001 = Use I/O clock with no prescaling.
   */
  /*
  TCCR0A = (1 << COM0A1) | (1 << COM0A0) | (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS00);
  */

  // Set sleep mode to idle.
  /*
  set_sleep_mode(SLEEP_MODE_IDLE);
  */

  // Turn on interrupts.
  /*
  sei();
  */
}

int
main(void) {
  setup();

  while (1) {
    PORTB = 0x3f;
    _delay_ms(500);
    PORTB = 0x3e;
    _delay_ms(500);
  }
}
