#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <dht.h>
LiquidCrystal_I2C lcd(0x27,16,2);
dht DHT;

byte fadeDirection = 0, fadeIncrement = 5;
bool greenState = LOW, yellowState = LOW, sleepState = false;
float distanceInCm, distanceInM;
int switch1 = 0, switch2 = 1;
int greenLed = 9, yellowLed = 10, redLed = 11, buzzer = 5, trigPin = 2, echoPin = 3, fadeValue = 0;
long duration;
unsigned long startTimeGlowFade = 0, startGreenTimeBuzzer = 0, startYellowTimeBuzzer = 0, startTimeGreenLED = 0, startTimeYellowLED = 0;
unsigned long startMicroHigh = 0, startMicroLow = 0, startTimeMeters = 0;
unsigned int fadeInterval = 50, farBuzzInterval = 900, farGreenInterval = 450, aveBuzzInterval = 200, aveYellowInterval = 100;
unsigned int clAveBuzzInterval = 100, clAveYlwInterval = 50, trigMicroInterval1 = 2, trigMicroInterval2 = 10, distanceInterval = 10;

void setup() {
   lcd.begin();
   lcd.backlight();
   lcd.clear(); 
   pinMode(echoPin, INPUT);
   pinMode(trigPin, OUTPUT);
   pinMode(switch1, INPUT);
   pinMode(switch2, INPUT);
   pinMode(greenLed, OUTPUT);
   pinMode(yellowLed, OUTPUT);
   pinMode(redLed, OUTPUT);
}

void loop() {
   unsigned currentMillis = millis();
   unsigned currentMicros = micros();
   deviceState();
   startFeedback(currentMillis, currentMicros); 
}

void deviceState(){
   if(digitalRead(switch1) == LOW && digitalRead(switch2) == HIGH) setState(true);
   else if(digitalRead(switch1) == HIGH && digitalRead(switch2) == LOW) setState(false);
}

void setState(bool isSleep){
   lcd.clear();
   sleepState = isSleep;
}

void startFeedback(unsigned long currentMillis, unsigned long currentMicros){
   if(!sleepState)verifyDistance(currentMillis, currentMicros);
   else deviceIdle(currentMillis);
}

void verifyDistance(unsigned long currentMillis, unsigned long currentMicros){
   scanDistance(currentMicros);
   
   if(distanceInM > 1){
     twoLedOff(yellowLed, redLed);
     highDistanceFeedback(currentMillis);
     printData(currentMillis);
   }else if(distanceInM <= 1 && distanceInM > 0.7){
     twoLedOff(greenLed, redLed);
     averageDistance(currentMillis);
     printData(currentMillis);
   }else if(distanceInM <= 0.7 && distanceInM > 0.4){
     twoLedOff(greenLed, redLed);
     closeAverageDistance(currentMillis);
     printData(currentMillis);
   }else{
     twoLedOff(greenLed, yellowLed);
     closeDistance();
     printData(currentMillis);
   }
}

void scanDistance(unsigned long currentMicros){
   digitalWrite(trigPin, LOW);
   
   if(currentMicros - startMicroHigh >= trigMicroInterval1){
     startMicroHigh = currentMicros;
     digitalWrite(trigPin, HIGH);
   }
   
   if(currentMicros - startMicroLow >= trigMicroInterval2){
     startMicroLow = currentMicros;
     digitalWrite(trigPin, LOW);
   }
 
   duration = pulseIn(echoPin, HIGH);
   distanceInCm = duration * 0.034 / 2;
   distanceInM = distanceInCm / 100;
}

void twoLedOff(int ledPin1, int ledPin2){
   digitalWrite(ledPin1, LOW);
   digitalWrite(ledPin2, LOW);
}
  
void highDistanceFeedback(unsigned long currentMillis){
   if(currentMillis - startGreenTimeBuzzer >= farBuzzInterval){
     startGreenTimeBuzzer = currentMillis;
     tone(buzzer, 3000, 100);
   } 
   if(currentMillis - startTimeGreenLED >= farGreenInterval){
     startTimeGreenLED = currentMillis;
     if(greenState) greenState = LOW;
     else greenState = HIGH; 
     digitalWrite(greenLed, greenState); 
   }
}

void averageDistance(unsigned long currentMillis){
   if(currentMillis - startYellowTimeBuzzer >= aveBuzzInterval){
     startYellowTimeBuzzer = currentMillis;
     tone(buzzer, 3000, 150);
   } 
   
   if(currentMillis - startTimeYellowLED >= aveYellowInterval){
     startTimeYellowLED = currentMillis;
     if(yellowState) yellowState = LOW;
     else yellowState = HIGH;
     digitalWrite(yellowLed, yellowState); 
   }
}

void closeAverageDistance(unsigned long currentMillis){
   if(currentMillis - startYellowTimeBuzzer >= clAveBuzzInterval){
     startYellowTimeBuzzer = currentMillis;
     tone(buzzer, 3000, 50);
   } 
   if(currentMillis - startTimeYellowLED >= clAveYlwInterval){
     startTimeYellowLED = currentMillis;
     if(yellowState) yellowState = LOW;
     else yellowState = HIGH;
     digitalWrite(yellowLed, yellowState); 
   }
}

void closeDistance(){
   tone(buzzer, 3000);
   digitalWrite(redLed, HIGH);
}

void printData(unsigned long currentMillis){
   if(currentMillis - startTimeMeters >= distanceInterval){
     startTimeMeters = currentMillis; 
     lcd.setCursor(0, 0);
     lcd.print("Distance: ");
     lcd.print(distanceInM);
     lcd.print("m ");
   }
}

void deviceIdle(unsigned long currentTime){
  DHT.read11(A0);
  noTone(buzzer);
  ledGlowFade(currentTime);
  lcd.setCursor(0,0);
  lcd.print("Welcome Home!");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(DHT.temperature, 1);
  lcd.print(" C");
}

void ledGlowFade(unsigned long currentMillis){
   if (currentMillis - startTimeGlowFade >= fadeInterval) {
     if (fadeDirection == 0){
      fadeValue = fadeValue + fadeIncrement;
      if (fadeValue >= 255) {
        fadeValue = 255;
        fadeDirection = 1;
      }
   }else {
     fadeValue = fadeValue - fadeIncrement;
     if (fadeValue <= 0){
       fadeValue = 0;
       fadeDirection = 0;
     }
   }
     analogWrite(greenLed, fadeValue); analogWrite(yellowLed, fadeValue); analogWrite(redLed, fadeValue); 
     startTimeGlowFade = currentMillis;
   }
}
