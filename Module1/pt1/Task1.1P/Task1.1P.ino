// ----------------------------------------------------------------------------
// File:        Task1.1P.ino
// Author:      Codey Funston
// Version:     1.0.0 
//
// Description: 
//             
//              Using Arduino Uno R3 board, LEDs show level
//              of fire danger. Levels are white < yellow <
//              orange < red.
//
// ----------------------------------------------------------------------------

const uint8_t TEMP_PIN = A0;

const uint8_t WHITE_LED_PIN = 12;
const uint8_t YELLOW_LED_PIN = 11;
const uint8_t ORANGE_LED_PIN = 10;
const uint8_t RED_LED_PIN = 9;

float voltageToCelcius(uint16_t volt_in) {
  // The ADC sends 5v as 1023, therefore:
  // ADC / voltage = 1023 / 5
  float ADC_to_volt = volt_in * (5.0 / 1023.0);
  return (1000 * ADC_to_volt - 500) / 10.0;
}

void setup() {
  pinMode(TEMP_PIN, INPUT); 
  
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  digitalWrite(WHITE_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  float temp = voltageToCelcius(analogRead(TEMP_PIN));
  Serial.println(temp);
  
  // If temp is less than 15 no lights show as there is no fire danger.

  if (temp >= 15 && temp < 20) {
    digitalWrite(WHITE_LED_PIN, HIGH);
  }

  else if (temp >= 20 && temp < 30) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  }
  
  else if (temp >= 30 && temp < 35) {
    digitalWrite(ORANGE_LED_PIN, HIGH);
  }
  
  else if (temp >= 35) {
    digitalWrite(RED_LED_PIN, HIGH);
  }

  delay(5000);
}