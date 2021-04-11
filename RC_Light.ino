
int LED_Head = 6;
int LED_Left = 7;
int LED_Right = 4;
int LED_Break = 5;
int LED_Back = 3;

int Break_Brightness = 0;

bool SignalOnOff = false;

boolean Forward;
boolean Backward = true;
boolean Break = false;
boolean Left = true;
boolean Right = true;
boolean After_Forward;

boolean LightOn = true;


int ch1_value = 0;
int ch2_value = 0;
int ch3_value = 0;
int ch4_value = 0;

void setupPortBPinChangeInterrupt() {

  PCICR |= (1 << PCIE0);    //enable PCMSK0 scan
  //PCMSK0 |= (1 << PCINT0);  //Set PCINT0 (digital input 8) to trigger an interrupt on state change.
  //PCMSK0 |= (1 << PCINT1);  //Set PCINT0 (digital input 9) to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT2);  //Set PCINT0 (digital input 10) to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT3);  //Set PCINT0 (digital input 11) to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT4);  //Set PCINT0 (digital input 12) to trigger an interrupt on state change.
  //PCMSK0 |= (1 << PCINT5);  //Set PCINT0 (digital input 13) to trigger an interrupt on state change.
}

void setupTimer1() {
  noInterrupts();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 2 Hz (8000000/((15624+1)*256))
  OCR1A = 15624;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 256
  TCCR1B |= (1 << CS12);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void PinSetup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_Head, OUTPUT);
  pinMode(LED_Left, OUTPUT);
  pinMode(LED_Right, OUTPUT);
  pinMode(LED_Break, OUTPUT);
  pinMode(LED_Back, OUTPUT);
}

// the setup function runs once when you press reset or power the board
void setup() {
  PinSetup();
  setupPortBPinChangeInterrupt();
  setupTimer1();
  Serial.begin(9600);
}

void updateLED() {
  //digitalWrite(LED_BUILTIN, LED_Status);
  digitalWrite(LED_Head, LightOn);

  digitalWrite(LED_Left, Left && SignalOnOff);
  digitalWrite(LED_Right, Right && SignalOnOff);
  digitalWrite(LED_Back, Backward);

  Break_Brightness = LightOn ? 50 : 0;
  if (Break) {
    Break_Brightness = 255;
  }
  analogWrite(LED_Break, Break_Brightness);
}

int Thottle_Center = 1500;
int Thottle_Buffer = 30;
int Steering_Center = 1500;
int Steering_Buffer = 10;
int Last_Thottle;

void TottleCheck() {
  // Check Thottle
  int Thottle =  ch2_value - Thottle_Center;

  if (abs(Thottle) < Thottle_Buffer) {
    {
      Forward = false;
      Backward = false;
      Break = false;
      After_Forward = false;
    }
  } else if (Thottle > 0 ) {
    Forward = true;
    Backward = false;
    After_Forward = true;

    if ((Last_Thottle - Thottle - Thottle_Buffer) > 0) {
      Break = true;
    } else {
      Break = false;
    }

  } else if (Thottle < 0 ) {
    Forward = false;

    if (After_Forward) {
      Backward = false;
      Break = true;
      After_Forward = true;
    } else {
      Backward = true;
      Break = false;
      After_Forward = false;
    }

  }
  Last_Thottle = Thottle;
}

void TrunCheck() {
  //Check Trun
  int Steering =  ch1_value - Steering_Center;
  if (abs(Steering) < Steering_Buffer) {
    {
      Left = false;
      Right = false;
    }
  } else if (Steering > 0 ) {
    {
      Left = false;
      Right = true;
    }
  } else {
    {
      Left = true;
      Right = false;
    }
  }
}


// the loop function runs over and over again forever
void loop() {

  LightOn = (ch3_value > 1700);

  TrunCheck();
  TottleCheck();
  if (!Break && !Backward) {
    Break = (ch2_value < 1700);
  }

  Serial.print("CH1:");
  Serial.print(ch1_value);
  Serial.print("\tCH2:");
  Serial.print(ch2_value);
  Serial.print("\tCH3:");
  Serial.print(ch3_value);
  Serial.println("");
}


ISR(TIMER1_COMPA_vect) {
  SignalOnOff = !SignalOnOff;
  updateLED();
}


//We create variables for the time width values of each PWM input signal
unsigned long counter_1, counter_2, counter_3, counter_4, current_count;

//We create 4 variables to stopre the previous value of the input signal (if LOW or HIGH)
byte last_CH1_state, last_CH2_state, last_CH3_state, last_CH4_state;

ISR(PCINT0_vect) {
  current_count = micros();
  ///////////////////////////////////////Channel 1
  if (PINB & B00000100) {                            //We make an AND with the pin state register, We verify if pin 10 is HIGH???
    if (last_CH1_state == 0) {                       //If the last state was 0, then we have a state change...
      last_CH1_state = 1;                            //Store the current state into the last state for the next loop
      counter_1 = current_count;                     //Set counter_1 to current value.
    }
  }
  else if (last_CH1_state == 1) {                    //If pin 8 is LOW and the last state was HIGH then we have a state change
    last_CH1_state = 0;                              //Store the current state into the last state for the next loop
    ch1_value = current_count - counter_1;   //We make the time difference. Channel 1 is current_time - timer_1.
  }

  ///////////////////////////////////////Channel 2
  if (PINB & B00001000 ) {                           //pin D11 -- B00001000
    if (last_CH2_state == 0) {
      last_CH2_state = 1;
      counter_2 = current_count;
    }
  }
  else if (last_CH2_state == 1) {
    last_CH2_state = 0;
    ch2_value = current_count - counter_2;
  }

  ///////////////////////////////////////Channel 3
  if (PINB & B00010000 ) {                           //pin D12 -- B00010000
    if (last_CH3_state == 0) {
      last_CH3_state = 1;
      counter_3 = current_count;
    }
  }
  else if (last_CH3_state == 1) {
    last_CH3_state = 0;
    ch3_value = current_count - counter_3;
  }
}
