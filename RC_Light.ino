bool isDebug = false;

const long signalInterval = 500;

int pin_Steering_Channel = 2; //Steering Channel
int pin_Thottle_Channel = 3; //Thottle Channel

int Head_LED = 10;
int Back_LED = 11;
int Break_LED = 9;

int Turn_L_LED = 7;
int Turn_R_LED = 8;

int Steering_Buffer = 50;
int Thottle_Buffer = 50;
int Speed_Buffer = 0;//5;
int Last_Thottle = 0;

// read channel values ch1 ch2 ch3 1000~2000 ns
int Thottle_Value;
int Steering_Value;

bool isCenter = false;
int Steering_Center = 0;
int Thottle_Center = 0;

boolean Forward;
boolean Backward;
boolean Break;
boolean Left;
boolean Right;
boolean After_Forward;

void setup() {
  // put your setup code here, to run once:
  pinMode(pin_Steering_Channel, INPUT); // //Left Right Channel
  pinMode(pin_Thottle_Channel, INPUT); // Channel 2

  attachInterrupt(digitalPinToInterrupt(pin_Steering_Channel), CH1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_Thottle_Channel), CH2, CHANGE);

  pinMode(Head_LED, OUTPUT);
  pinMode(Turn_L_LED, OUTPUT);
  pinMode(Turn_R_LED, OUTPUT);
  pinMode(Break_LED, OUTPUT);
  pinMode(Back_LED, OUTPUT);

  if (isDebug) {
    Serial.begin(9600);
  }
}

void loop() {

  if (!isCenter) {
    CenterCalc();
  }

  calcSignalOnOff();
  TrunCheck();
  TottleCheck();

  if (isDebug) {
    printChannelValue();
    printCarAction();
    delay(200);
  }

  updateLED();

}

bool signalOFF = true;
unsigned long previousMillis = 0;

void calcSignalOnOff() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= signalInterval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    if (signalOFF) {
      signalOFF = false;
    } else {
      signalOFF = true;
    }
  }
}

int CH1_Start;
int CH2_Start;

void CH1() {
  int time = micros();
  int stat = digitalRead(pin_Steering_Channel);

  if (stat == HIGH) {
    CH1_Start = time;
  } else {
    Steering_Value = time - CH1_Start;
  }
}

void CH2() {
  int time = micros();
  int stat = digitalRead(pin_Thottle_Channel);

  if (stat == HIGH) {
    CH2_Start = time;
  } else {
    Thottle_Value = time - CH2_Start;
  }
}

void TrunCheck() {
  //Check Trun
  int Steering =  Steering_Value - Steering_Center;
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
void TottleCheck() {
  // Check Thottle
  int Thottle =  Thottle_Value - Thottle_Center;

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

    if ((Last_Thottle - Thottle - Speed_Buffer) > 0) {
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


void CenterCalc() {
  if (Steering_Value > 0) {
    isCenter = true;
    Thottle_Center = Thottle_Value;
    Steering_Center = Steering_Value;
  }
}

void printChannelValue() {

  Serial.print("Receiver Value; Steering:"); // Print the value of
  Serial.print(Steering_Value);        // each channel
  Serial.print("\tThottle:");
  Serial.print(Thottle_Value);
  Serial.println();

  Serial.print("Center Value; Steering:");
  Serial.print(Steering_Center);
  Serial.print("\tThottle:");
  Serial.print(Thottle_Center);
  Serial.println();
}

void printCarAction() {

  Serial.print("Car Dir: Forward:");
  Serial.print(Forward);
  Serial.print("\tBackward:");
  Serial.print(Backward);
  Serial.print("\tBreak:");
  Serial.print(Break);
  Serial.print("\tLeft:");
  Serial.print(Left);
  Serial.print("\tRight:");
  Serial.print(Right);
  Serial.println();

}

void updateLED() {
  int Head_LED_Brightness =   0;
  int Break_LED_Brightness = 0;

  if (Forward) {
    Head_LED_Brightness = Head_LED_Brightness + 127;
  }
  if (Break) {
    Break_LED_Brightness = Break_LED_Brightness + 127;
  }
  analogWrite(Head_LED, Head_LED_Brightness);
  analogWrite(Break_LED, Break_LED_Brightness);

  if (Backward) {
    digitalWrite(Back_LED, HIGH);
  } else {
    digitalWrite(Back_LED, LOW);
  }



  if (Left) {
    if (signalOFF) {
      digitalWrite(Turn_L_LED, LOW);
    } else {
      digitalWrite(Turn_L_LED, HIGH);
    }
  } else {
    digitalWrite(Turn_L_LED, LOW);
  }
  if (Right) {
    if (signalOFF) {
      digitalWrite(Turn_R_LED, LOW);
    } else {
      digitalWrite(Turn_R_LED, HIGH);
    }
  } else {
    digitalWrite(Turn_R_LED, LOW);
  }
}
