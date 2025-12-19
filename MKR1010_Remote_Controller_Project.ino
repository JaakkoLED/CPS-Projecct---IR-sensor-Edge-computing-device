#define IR_TRANSMITTER_PIN 3
#define BUTTON_PIN 4

void setup()
{
  pinMode(IR_TRANSMITTER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);   
}

void sendBurst()
{
  tone(IR_TRANSMITTER_PIN, 38000);
  delayMicroseconds(600);  
  noTone(IR_TRANSMITTER_PIN);
  delayMicroseconds(600);  
}


void sendCommand()
{
  for (int i = 0; i < 50; i++)
  {  
    sendBurst();
  }
}

void loop()
{
  static int lastButtonState = HIGH;

  int buttonState = digitalRead(BUTTON_PIN);

  //High -> LOW (Falling): Do something
  if (lastButtonState == HIGH && buttonState == LOW)
  {
    delay(20);
    //Chehcking
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      //send 38 kHz signal
      //38 kHz ON ja OFF 0.0012 ms * 50 = 60 ms(60 ms "Command") (duty 50% = 6 µs)   
      sendCommand();
    }
  }

  lastButtonState = buttonState;
}
