#include "Ultrasonic.h"
#include "rgb_lcd.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

rgb_lcd lcd;

#define IR_PIN 2           // IR receiver
#define INDICATION 3       // MKR1010:lle - HIGH=OFF, LOW=ON

#define LED_1 4
#define LED_2 5
#define LED_3 6
#define LED_4 7
#define LED_5 8
#define LED_6 9
#define LED_7 10
#define LED_8 11

#define buzzer 13

Ultrasonic ultrasonic(12);

int f_offset = 0;
volatile bool systemActive = false;
volatile unsigned long lastInterrupt = 0;
const unsigned long DEBOUNCE_TIME = 250;  

void setup()
{
  Serial.begin(9600);
  
  // LCD alustetaan
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);
  
  attachInterrupt(digitalPinToInterrupt(IR_PIN), toggleSystem, FALLING);
  
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(LED_6, OUTPUT);
  pinMode(LED_7, OUTPUT);
  pinMode(LED_8, OUTPUT);
  pinMode(INDICATION, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(buzzer, OUTPUT);
  
  digitalWrite(INDICATION, HIGH);
  systemActive = false;
  
  lcd.clear();
  lcd.print("System Ready");
  delay(800);
}


void toggleSystem()
{
  unsigned long currentTime = millis();
  
  if(currentTime - lastInterrupt > DEBOUNCE_TIME)
  {
    
    systemActive = !systemActive;
    lastInterrupt = currentTime;
    
    
    if(systemActive)     
    {
      
      digitalWrite(INDICATION, LOW);
      tone(buzzer, 800, 200); 
      Serial.println("✅ SYSTEM ON");
    } 
    else                 
    {
      digitalWrite(INDICATION, HIGH);
      tone(buzzer, 500, 100);  
      Serial.println("❌ SYSTEM OFF");
    }              
  }
}

//SYSTEM OFF
void system_off()
{
  // LCD Screen
  lcd.setCursor(0, 0);
  lcd.print("System: OFF ");
  lcd.setCursor(0, 1);
  lcd.print("Sleeping...    ");
  
  noTone(buzzer);
  
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  digitalWrite(LED_5, LOW);
  digitalWrite(LED_6, LOW);
  digitalWrite(LED_7, LOW);
  digitalWrite(LED_8, LOW);
  
  
  digitalWrite(INDICATION, HIGH);
  
 
  while(!systemActive)
  {
    delay(100);  
  }
}

// Main Loop
void loop()
{
  // If system OFF -> Sleep
  if(!systemActive)
  {
    system_off();
    return;
  }
  
  // SYSTEM ON
  
  // Measure Distance
  float t = ultrasonic.MeasureInCentimeters();
  
  // Potentiometri offset
  f_offset = analogRead(A0);
  
  // LCD - Rivi 1: Status
  lcd.setCursor(0, 0);
  lcd.print("System: ON  ");
  
  // LCD - rode 2: Distance
  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(t);
  lcd.print(" cm  ");
  
  
  if(t >= 2.5 && t <= 5) 
  {
    tone(buzzer, 50 + f_offset);
  } 
  else if (t > 5 && t <= 7.5) 
  {
    tone(buzzer, 100 + f_offset);
  }
  else if (t > 7.5 && t <= 10) 
  {
    tone(buzzer, 150 + f_offset);
  }  
  else if (t > 10 && t <= 12.5) 
  {
    tone(buzzer, 200 + f_offset);
  }
  else if (t > 12.5 && t <= 15) 
  {
    tone(buzzer, 250 + f_offset);
  }
  else if (t > 15 && t <= 17.5) 
  {
    tone(buzzer, 300 + f_offset);
  }   
  else if (t > 17.5 && t <= 20) 
  {
    tone(buzzer, 350 + f_offset);
  }
  else
  {
    noTone(buzzer);  
  }
  

  digitalWrite(LED_1, (t >= 2.5)  ? HIGH : LOW);
  digitalWrite(LED_2, (t >= 5)    ? HIGH : LOW);
  digitalWrite(LED_3, (t >= 7.5)  ? HIGH : LOW);
  digitalWrite(LED_4, (t >= 10)   ? HIGH : LOW);
  digitalWrite(LED_5, (t >= 12.5) ? HIGH : LOW);
  digitalWrite(LED_6, (t >= 15)   ? HIGH : LOW);
  digitalWrite(LED_7, (t >= 17.5) ? HIGH : LOW);
  digitalWrite(LED_8, (t >= 20)   ? HIGH : LOW);
  
  delay(120);
}