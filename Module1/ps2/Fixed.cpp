
// Defining the pin numbers for the button (BTN) and 
// the light emitting diode (LED). The pin numbers 
// correspond to the physical IDs of the pins on the
// board.
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;
const size_t DELAY = 50;

// These variables hold the state of the button and LED.
// ie the button can pressed or depressed. We care about
// the previous state not just the current state because
// that allows for comparison.
uint8_t buttonPrevState = LOW;
uint8_t ledState = LOW;

size_t previous_time = 0;

// Initialises pins and the serial output.
void setup()
{
  // Sets the button pin to INPUT_PULLUP mode.
  // This means that internally a pull-up resistor
  // is used for that pin so that when the button
  // is open the input will be HIGH.
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Setting a pin to OUTPUT means that it is using
  // low-impedence which essentially means a high
  // current is sent out to the circuit it corresponds 
  // to. This is what we want to power an LED.
  pinMode(LED_PIN, OUTPUT);
  
  // This opens the serial port to enable communication 
  // to the serial monitor. The passed in number sets the 
  // bit transfer rate. 9600 is commmonly used as it has 
  // a good balance between properties like speed, compatibility,
  // and loss.
  Serial.begin(9600);
}

void loop()
{
  // Sets the button state to the current input from
  // the button pin via digitalRead() to read the value
  // from a digital pin.
  uint8_t buttonState = digitalRead(BTN_PIN);
  
  // Sending information to the serial monitor, in text
  // form by using the encapsulated logic from the print()
  // function. println() simply adds a new line, analogous
  // to WriteLine() in .NET.
  Serial.print(buttonState);
  Serial.print(buttonPrevState);
  Serial.print(ledState);
  Serial.println("");
  
  
  // If the button state has changed, change the state of the
  // LED, ie switch it on or off.
  if(buttonState != buttonPrevState)
  { 
    if ((millis() - previous_time) > DELAY)
    {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);

      previous_time = millis();
    }
  }

  buttonPrevState = buttonState;
  
  // Adds a 500 millisecond delay so that the program
  // functionality can be physically observed.
  delay(50);
}
