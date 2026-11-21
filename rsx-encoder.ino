
// #include <Servo.h>

#include "science_can.h"

// OLD PINOUT IDK IF TRUE:
// PINOUT to Arduino Uno
// SCK to Pin 13
// SI to Pin 11
// SO to Pin 12
// CS to 10
//
// REAL PINOUT:
// encoder to Pin 8

module_t CAN_MODULE = kModuleDiffBar;

volatile uint16_t riseTime = 0;
volatile uint16_t pulseTicks = 0;
volatile bool newReading = false;

float degrees = 0;

const unsigned long TX_PERIOD = 50;
unsigned long lastTx = 0;

void setup() {
  Serial.begin(115200);

  // Timer setup

  pinMode(8, INPUT);

  TIMSK1 = 0;

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  TIFR1 = (1 << ICF1);

  TIMSK1 = (1 << ICIE1);

  TCCR1B = (1 << ICES1) | (1 << CS10);

  // CAN setup

  SPI.begin();

  // Reset MCP2515
  Science::mcp2515.reset();

  // Set CAN speed (must match your bus!)
  Science::mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);

  // Switch to normal mode
  Science::mcp2515.setNormalMode();

  Serial.println("MCP2515 init OK Yayyyyy :)");

  // attaches the servo on pin 9 to the servo object
  // rightAngleServo.attach(3);

  Serial.println("Servo attached!");

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
      degrees = (pulseUs - 3.884f) * (360.0f / 1000.0f);
  }


  if (const int recv_cnt = Science::process_rx()) {
    Serial.print("Received ");
    Serial.print(recv_cnt);
    Serial.println(" Messages.");
  }

  if (millis() - lastTx >= TX_PERIOD) {
    lastTx = millis();

    Science::ScienceCANMessage send_back;
    send_back.priority_ = 0x0;
    send_back.multipacket_id_ = 0x0;
    send_back.sender_ = kModuleDiffBar;
    send_back.receiver_ = kModuleNone;
    send_back.peripheral_ = kPeripheralAbsoluteEncoder;
    send_back.dlc_ = 4;

    Serial.print("Encoder degrees: ");
    Serial.println(degrees);
    *((float*) (send_back.data_)) = degrees; // Copy 32b float into 8b[4]
    Science::tx_buffer.push(send_back);
  }

  while (!Science::rx_buffer.empty()) {
    Science::ScienceCANMessage incoming_message = Science::rx_buffer.pop();
    if (incoming_message.peripheral_ = kPeripheralUltrasonic) {

    }
  }

  if (const int send_cnt = Science::process_tx()) {
    Serial.print("Sent ");
    Serial.print(send_cnt);
    Serial.println(" Messages.");
  }
}

