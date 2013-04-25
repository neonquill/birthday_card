#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

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
  NOTE_COUNT = 78
};
    
const struct note song[NOTE_COUNT] PROGMEM =
{
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
  // LED on pin 6 = PB1.
  // LED and Speaker Pin as output, initially 0,
  // the rest as inputs with pullups enabled.
  DDRB = 0x03;
  PORTB = 0x3c;

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

void
blink(int count) {
  int i;

  for (i = 0; i < count; i ++) {
    PORTB |= (1 << PB1);
    _delay_ms(500);
    PORTB &= ~(1 << PB1);
    _delay_ms(500);
  }
}

void
blink_pitch(int pitch) {
  switch (pitch) {
  case D4: blink(1); break;
  case G4: blink(2); break;
  case A4: blink(3); break;
  default: blink(10); break;
  }
  _delay_ms(1000);
}

int
main(void) {
  int i, j;
  const struct note *note;
  uint8_t pitch, ticks;
  int count;

  setup();

  count = 13;

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
      // XXX OCR0A = pitch;
    }

    for (j = 0; j < ticks; j++) {
      count--;
      if (count <= 0) {
        PORTB ^= (1 << PB1);
        count = 13;
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

  while (1) {
    /*
    PORTB = 0x3f;
    _delay_ms(500);
    PORTB = 0x3c;
    _delay_ms(500);
    */
  }
}
