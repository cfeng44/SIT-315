//
// File: 		Task1.1P.ino
// Author: 		Codey Funston
// Description: 
//              Using Arduino Uno R3 board, LEDs show level
//              of fire danger. Levels are white < yellow <
//              orange < red.
//

const int TEMP_PIN = A0;

const int WHITE_LED_PIN = 12;
const int YELLOW_LED_PIN = 11;
const int ORANGE_LED_PIN = 10;
const int RED_LED_PIN = 9;

double voltageToCelcius(int volt_in)
{
  // The ADC sends 5v as 1023, therefore:
  // ADC / voltage = 1023 / 5
  double ADC_to_volt = volt_in * (5.0 / 1023.0);
  return (1000*ADC_to_volt - 500) / 10.0;
}

void setup()
{
  pinMode(TEMP_PIN, INPUT);
  
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  
  Serial.begin(9600);
}

void loop()
{
  digitalWrite(WHITE_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  double temp = voltageToCelcius(analogRead(TEMP_PIN));
  Serial.println(temp);
  
  if (temp >= 15 && temp < 20) 
  {
    digitalWrite(WHITE_LED_PIN, HIGH);
  }
  
  else if (temp >= 20 && temp < 30) 
  {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  }
  
  else if (temp >= 30 && temp < 35) 
  {
    digitalWrite(ORANGE_LED_PIN, HIGH);
  }
  
  else if (temp >= 35)
  {
    digitalWrite(RED_LED_PIN, HIGH);
  }

  delay(5000);
}