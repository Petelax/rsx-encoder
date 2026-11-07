#include <Arduino.h>

volatile uint16_t riseTime = 0;
volatile uint16_t pulseTicks = 0;
volatile bool newReading = false;

void setup() {
    Serial.begin(115200);
    pinMode(8, INPUT);

    // Disable Timer1 interrupts during setup
    TIMSK1 = 0;

    // Normal mode, no PWM
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Clear pending capture flag
    TIFR1 = (1 << ICF1);

    // Enable input capture interrupt
    TIMSK1 = (1 << ICIE1);

    // Rising edge capture, no prescaler
    TCCR1B = (1 << ICES1) | (1 << CS10);
}

ISR(TIMER1_CAPT_vect) {
    uint16_t captured = ICR1;

    if (TCCR1B & (1 << ICES1)) {
        // Rising edge
        riseTime = captured;

        // Capture falling edge next
        TCCR1B &= ~(1 << ICES1);
    } else {
        // Falling edge
        pulseTicks = captured - riseTime;
        newReading = true;

        // Capture rising edge next
        TCCR1B |= (1 << ICES1);
    }
}

void loop() {
    uint16_t ticks;
    bool updated;

    noInterrupts();
    ticks = pulseTicks;
    updated = newReading;
    newReading = false;
    interrupts();

    if (updated) {
        float pulseUs = ticks / 16.0f;
        float degrees = (pulseUs - 3.884f) * (360.0f / 1000.0f);
        Serial.println(degrees, 4);
    }
    
}