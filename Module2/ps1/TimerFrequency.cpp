// ----------------------------------------------------------------------------
// File: 		TimerFrequency.ino
// Author: 		Codey Funston
// Description: 
//              
//              Using the physically oscillator in the Arduino's
//              board (ATmega32) to send of an interrupt at an
//              interval based on the input from a potentiometer.
//
// ----------------------------------------------------------------------------

const uint8_t LED_PIN = 13;
const uint8_t METER_PIN = A4;
const uint16_t FREQ_MIN_HZ = 1;
const uint16_t FREQ_MAX_HZ = 4;

uint16_t prev_meter_val = 0;

// Sets the frequency based on the analog input from the 
// potentiometer if the value has changed.
void setFrequency() {
  uint16_t meter_val = analogRead(METER_PIN);

  if (meter_val != prev_meter_val) {
    // map() relates the input value that is between 0 and 1023 
    // to be in the range of frequencies. In one or more of my
    // other tasks I did this manually before I knew about
    // map() from the seminar demonstration.
    double frequency = map(meter_val, 0, 1023, FREQ_MIN_HZ, FREQ_MAX_HZ);
    startTimer(frequency);
    prev_meter_val = meter_val;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);
      
  Serial.begin(9600);
}

// To have the LED flash every two seconds (first part of this task)
// you would simply call setFrequency() in setup() once with an input
// of 0.5 as the frequency.
void loop() { 
  setFrequency();
  delay(100);  
}

// Initialises the timer in the board so that it resets relative to the
// desired frequency.
void startTimer(double frequency){
  noInterrupts();
  
  // Clearing for initialisation.
  TCCR1A = 0, TCCR1B = 0, TCNT1 = 0;
  
  // The combination of the bits CS10 = 1, CS11 = 0, and CS12 = 1
  // in the Timer/Counter control register B will prescale by a 
  // factor of 1024. Obviously we don't need to set CS11 to 0
  // manually but I did so for myself getting used to setting
  // register bits as in my other tasks I used wrapper functions.
  TCCR1B |= (1 << CS10) | (0 << CS11) | (1 << CS12);
  
  // To use clear timer on compare (CTC) we need to set the 
  // waveform generation mode to that type. Which is 1 for
  // the WGM12 (wave gen mode 12) in TCCR1B. The register to 
  // hold the compare value is OCR1A.
  TCCR1B |= (1 << WGM12);
  OCR1A = 16000000 / (1024 * frequency) - 1; // 16MHz = 16000000
  
  // This allows for output interrupts to be fired based on 
  // a comparison.
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts();
}

// This ISR runs when TIMER1 is equal to the value in the output 
// comparison register (OCR1A). Since the output from the LED
// pin is 1 or 0, doing bitwise XOR (^) will negate it.
ISR(TIMER1_COMPA_vect) {
  digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}