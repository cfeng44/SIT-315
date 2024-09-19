// ----------------------------------------------------------------------------
// File:        Task1.3C.ino
// Author:      Codey Funston
// Version:     1.0.0
//
// Description: 
//             
//              Using Arduino Uno R3 board, LEDs show level of fire danger. 
//              Levels are white < yellow < orange < red. Updated to use an
//              interrupt from a digital signal push button that causes all 
//              LEDs to flash to evacuate. Updated again to add an tilt 
//              sensor to show that the hardware has fallen over and isn't 
//              operating properly.
//
// ----------------------------------------------------------------------------

const uint8_t TEMP_PIN = A0;
const uint8_t INTERRUPT_PIN_A = 2; // Push button.
const uint8_t INTERRUPT_PIN_B = 3; // For tilt sensor.

const uint8_t WHITE_LED_PIN = 12;
const uint8_t YELLOW_LED_PIN = 11;
const uint8_t ORANGE_LED_PIN = 10;
const uint8_t RED_LED_PIN = 9;

const uint8_t FALLEN_PIN_1 = 7;
const uint8_t FALLEN_PIN_2 = 6;
const uint8_t FALLEN_PIN_3 = 5;
const uint8_t FALLEN_PIN_4 = 4;

const uint16_t read_temp_delay = 1500; // Time between temperature serial output.
const uint16_t ISR_delay = 200; // Time between evacuation blinks.

volatile uint8_t do_evacuate = LOW; // Interrupt flag for push button.
volatile uint8_t fallen = LOW;    // Interrupt flag for tilt sensor.

float voltageToCelcius(uint16_t volt_in) {
  // The ADC sends 5v as 1023, therefore:
  // ADC / voltage = 1023 / 5
  float ADC_to_volt = volt_in * (5.0 / 1023.0);
  return (1000 * ADC_to_volt - 500) / 10.0;
}

void ledData(uint8_t pin, char* colour_name, float temp) {
  Serial.print("TEMPERATURE: ");
  Serial.print(temp);
  digitalWrite(pin, HIGH);
  Serial.print(" COLOUR: ");
  Serial.println(colour_name);
}

void evacuate() {
  do_evacuate = !do_evacuate;
}

void fall() {
  fallen = !fallen;
}

void setup() {
  pinMode(TEMP_PIN, INPUT);
  pinMode(INTERRUPT_PIN_A, INPUT_PULLUP);
  pinMode(INTERRUPT_PIN_B, INPUT_PULLUP);

  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  pinMode(FALLEN_PIN_1, OUTPUT);
  pinMode(FALLEN_PIN_2, OUTPUT);
  pinMode(FALLEN_PIN_3, OUTPUT);
  pinMode(FALLEN_PIN_4, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_A), evacuate, FALLING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_B), fall, CHANGE);

  Serial.begin(9600);
}

void loop() { 
  static unsigned long prev_millis = 0;
  unsigned long current_millis = millis();
  unsigned long delay_record = current_millis - prev_millis;
  
  if (do_evacuate) {
    if (delay_record >= ISR_delay) {
      prev_millis = current_millis;
      static bool led_state = false;
      led_state = !led_state;
      
      // Blinks all LEDs for evacuation.
      digitalWrite(WHITE_LED_PIN, led_state);
      digitalWrite(YELLOW_LED_PIN, led_state);
      digitalWrite(ORANGE_LED_PIN, led_state);
      digitalWrite(RED_LED_PIN, led_state);

      Serial.println("EVACUATE");
    }
  }

  else if (fallen) {
    if (delay_record >= ISR_delay) {
      prev_millis = current_millis;
      static bool led_state = false;
      led_state = !led_state;
      
      // Blinks all LEDs for evacuation.
      digitalWrite(FALLEN_PIN_1, led_state);
      digitalWrite(FALLEN_PIN_2, led_state);
      digitalWrite(FALLEN_PIN_3, led_state);
      digitalWrite(FALLEN_PIN_4, led_state);

      Serial.println("PLEASE STAND HARDWARE UPRIGHT FOR ACCURACY.");
    }
  }

  else {
    if (delay_record >= read_temp_delay) {
      prev_millis = current_millis;
    
      // Shows fire danger level as a colour.
      digitalWrite(WHITE_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(ORANGE_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, LOW);

      float temp = voltageToCelcius(analogRead(TEMP_PIN));

      if (temp >= 15 && temp < 20) {
        ledData(WHITE_LED_PIN, "White", temp);
      }
      
      else if (temp >= 20 && temp < 30) {
        ledData(YELLOW_LED_PIN, "Yellow", temp);
      }
      
      else if (temp >= 30 && temp < 35) {
        ledData(ORANGE_LED_PIN, "Orange", temp);
      }
      
      else if (temp >= 35) {
        ledData(RED_LED_PIN, "Red", temp);
      }
      
      else {
        Serial.print("TEMPERATURE: ");
        Serial.print(temp);
       	Serial.println(" No fire danger.");
      }
    }
  }
}