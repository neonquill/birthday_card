#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

/* Typo in the attiny definition. */

#define BODSE BPDSE
#define BODS BPDS

#include <avr/sleep.h>

#define NOTEP(v) {v, 4}, {0, 1}
#define NOTE1(v) {v, 9}, {0, 1}
#define NOTE2(v) {v, 19}, {0, 1}
#define NOTE3(v) {v, 29}, {0, 1}
#define NOTE5(v) {v, 49}, {0, 1}

enum {
  D4 = 217, /* 294 Hz */
  G4 = 162, /* 392 Hz */
  A4 = 144, /* 440 Hz */
  AS4 = 136, /* 466 Hz */
  B4 = 129, /* 494 Hz */
  C5 = 121, /* 523 Hz */
  D5 = 108, /* 587 Hz */
  E5 = 96, /* 659 Hz */
};

struct note {
  uint8_t pitch;
  uint8_t ticks;
};

enum {
  NOTE_COUNT = 80
};
    
const struct note song[NOTE_COUNT] PROGMEM =
{
  NOTE3(0),
  NOTE1(D4), NOTE1(G4), NOTE1(A4),
  NOTE2(B4), NOTE3(B4),
  NOTE1(B4), NOTE1(AS4), NOTE1(B4),
  NOTE2(G4), NOTE3(G4),
  NOTE1(G4), NOTE1(A4), NOTE1(B4),
  NOTE2(C5), NOTE3(E5),
  NOTE1(E5), NOTE1(D5), NOTE1(C5),
  NOTE5(B4), NOTEP(0),
  NOTE1(G4), NOTE1(A4), NOTE1(B4),
  NOTE2(C5), NOTE3(E5),
  NOTE1(E5), NOTE1(D5), NOTE1(C5),
  NOTE2(B4), NOTE3(G4),
  NOTE1(0), NOTE1(G4), NOTE1(A4),
  NOTE3(B4), NOTE1(C5),
  NOTE2(A4), NOTE1(A4), NOTE1(B4),
  NOTE5(G4)
};

// Define an empty interrupt for the watchdog timer, just to wake up.
EMPTY_INTERRUPT(WDT_vect);

void
setup(void) {
  // Speaker on pin 5 = PB0
  // LEDs on pins 2, 3, 7 = PB3, PB4, PB2.
  // LED and Speaker Pin as output, initially 0,
  // the rest as inputs with pullups enabled.
  DDRB = (1 << DDB0) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4);
  PORTB = (1 << PB1) | (1 << PB5);

  /*
   * Setup PWM.
   * Speaker is on PB0 which is OC0A.
   * WGM[2:0] = 010 = CTC
   * COM0A[1:0] = 01 = Toggle OC0A on compare match.
   * CS0[2:0] = 001 = Use I/O clock with no prescaling.
   */
  TCCR0A = (1 << COM0A0) | (1 << WGM01);
  TCCR0B = (1 << CS00);

  // Set sleep mode to idle.
  set_sleep_mode(SLEEP_MODE_IDLE);

  // Turn on interrupts.
  sei();
}

int
main(void) {
  int i, j, k;
  const struct note *note;
  uint8_t pitch, ticks;
  int count[3];
  int period[3] = {7, 5, 9};

  setup();

  count[0] = period[0];
  count[1] = period[1];
  count[2] = period[2];

  for (i = 0, note = song;
       i < NOTE_COUNT;
       i++, note++) {

    pitch = pgm_read_byte(&(note->pitch));
    ticks = pgm_read_byte(&(note->ticks));

    if (pitch == 0) {
      TCCR0A = 0;
      PORTB &= ~(1 << PB0);
    } else {
      TCCR0A = (1 << COM0A0) | (1 << WGM01);
      OCR0A = pitch;
    }

    for (j = 0; j < ticks; j++) {
      for (k = 0; k < 3; k++) {
        count[k]--;
        if (count[k] <= 0) {
          PORTB ^= (1 << (2 + k));
          count[k] = period[k];
        }
      }

      /*
       * Sleep, using the watchdog.
       * We keep WDE set to 0, because we don't need the reset functionality.
       * WDTIE = 1 enables the watchdog interrupt.
       *
       * We want to sleep for 4k cycles.
       * With 10 ticks per beat, this is 188 beats per minute.
       * WDP[3:0] = 0001 = 4k cycles, or about 32 ms.
       */
      WDTCR = (1 << WDTIE) | (1 << WDP0);
      wdt_reset();
      sleep_mode();
    }
  }

  TCCR0A = 0;
  PORTB = 0x3c;
  WDTCR = 0;
  wdt_reset();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  while (1) {
    sleep_bod_disable();
    sleep_mode();
  }
}
