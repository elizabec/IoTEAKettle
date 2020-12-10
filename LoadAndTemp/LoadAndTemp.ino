#include <Wire.h>
#include <OneWire.h>
#include <Notes.h>
#include <WiFiNINA.h> //library to enable use of WiFi

#define addFX29 0x28
#define MAXLEN 10
#define TEMP_ERROR 999.999
OneWire tempSensor(2); //Sensor pin = 2

float values[10]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float temp = 0;
float setTemp = 60;
byte pos = 0;
int ledGreen = 3;
int ledYellow = 4;
int buzzer = 8;
int choice = 0;

// Defining the song to play should probably be done with an array of pointers, 
// to make it possible to make the choice from the web page
//#define VICTORY
//#define SONGOFTIME
//#define MARIO

#if defined(VICTORY) 
int tempo = 115;
int tune[] = {
  C5, 12, C5, 12, C5, 12, C5, 4, GS4, 4, AS4, 4, C5, 12, REST, 12, AS4, 12, C5, -2

};
#elif defined(SONGOFTIME)
int tempo = 108;
int tune[] = {

  A4, 4, D4, 2, F4, 4, A4, 4, D4, 2,
  F4, 4, A4, 8, C4, 8, NOTE_B4, 4, G4, 4, F4, 8, G4, 8, A4, 4,
  D4, 4, C4, 8, E4, 8, D4, -1
};
#elif defined(MARIO)
int tempo = 200;
int tune[] = {E5,8, E5,8, REST,8, E5,8, REST,8, C5,8, E5,4, //1
  G5,4, REST,4, G4,8, REST,4, 
  C5,-4, G4,8, REST,4, E4,-4, // 3
  A4,4, NOTE_B4,4, AS4,8, A4,4,
  G4,-8, E5,-8, G5,-8, A5,4, F5,8, G5,8,
  REST,8, E5,4,C5,8, D5,8, NOTE_B4,-4
};
#else //This was very important to implement, trust me
int tempo = 115;
int tune[] = {D5, 2, E5, 8, FS5, 8, D5, 8, E5, 8, E5, 8, E5, 8, FS5, 8, E5, 4, A4, 4,
  REST, 2, NOTE_B4, 8, CS5, 8, D5, 8, NOTE_B4, 8, REST, 8, E5, 8, FS5, 8, E5, -4, 
  
  A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, FS5, -8, FS5, -8, E5, -4, 
  A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, E5, -8, E5, -8, D5, -8, CS5, 16, NOTE_B4, -8, 
  A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, D5, 4, E5, 8, CS5, -8, NOTE_B4, 16, A4 ,8, A4, 8, A4, 8, 
  E5, 4, D5, 2, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
  FS5, -8, FS5, -8, E5, -4, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
  A5, 4, CS5, 8, D5, -8, CS5, 16, NOTE_B4, 8, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
  D5, 4, E5, 8, CS5, -8, NOTE_B4, 16, A4, 4, A4, 8, E5, 4, D5, 2
};
#endif

float readTemp() {
  byte data[9];
  int temperature;
  float tempC;

  tempSensor.reset();
  tempSensor.write(0xCC);  //Alerts all devices to pay attention
  tempSensor.write(0x44);  //Start a temperature reading
 
  delay(800); //A temperature reading takes 750ms
 
  tempSensor.reset();
  tempSensor.write(0xCC); 
  tempSensor.write(0xBE); //Send content of scratchpad

  for (int i = 0; i < 9; i++) {
    data[i] = tempSensor.read();
  }

  temperature = (data[1] << 8) + data[0]; //Two byte binary temperature
  tempC = temperature / 16.0;

  return tempC;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
}

void loop() {

  Wire.beginTransmission(addFX29);
  Wire.write(0x51);

  int nBytes = 4;
  long bytesRead[nBytes];

  Wire.requestFrom(addFX29, nBytes);

  int cnt = 0;
  while (Wire.available()){
    bytesRead[cnt] = Wire.read();
    cnt++;
  }

  float Pdisplay, Lmax = 100, Lmin = 0;
  uint32_t Pvalue, Pspan, I2C_ERR;
  uint16_t P1 = 1300, P2 = 7000;

  if ((bytesRead[0] & 0xC0) == 0x00){
    Pvalue = (bytesRead[0] << 8) | bytesRead[1];
    I2C_ERR = 0; //All is well
  }
  else
    I2C_ERR = 1; //All is not well

  if (I2C_ERR == 0){
    Pspan = P2 - P1; //Span of values
    Pdisplay = (Pvalue - 1300) * 100 / Pspan; //Percentage

    if (Pdisplay >= 0 && Pdisplay <= 100) {
      values[pos] = Pdisplay;
      pos++;
      if (pos == MAXLEN) { //Running array of values
        pos = 0;
      }
    }
  }
  Wire.endTransmission();

  temp = readTemp();
  if (temp >= setTemp) {
    digitalWrite(ledGreen, HIGH);
    playTune();
  }
  else {
    digitalWrite(ledGreen, LOW);
    noTone(buzzer);
  }
  if (Pvalue >= 1350) {
    digitalWrite(ledYellow, HIGH);
  }
  else {
    digitalWrite(ledYellow, LOW);
  }
  Serial.print("Temperature =  ");
  Serial.print(temp, 1);
  Serial.print("  Pdisplay = ");
  Serial.print(Pdisplay);
  Serial.print("  Pvalue = ");
  Serial.print(Pvalue);
  Serial.print("  Average = ");
  Serial.println(pAvg());
  playTune();
}

float pAvg() {
  float sum = 0;
  for (int i = 0; i < MAXLEN; i++) {
    sum = sum + values[i];
  }
  return sum / MAXLEN;
}

void playTune() {
  
  int notes = sizeof(tune) / sizeof(tune[0]) / 2;
  int wholenote = (60000 * 4) / tempo; //Whole note in ms
  int divider = 0, noteDuration = 0;

  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    divider = tune[thisNote + 1]; //Calculates the duration of each note
    if (divider > 0) { //Regular note
      noteDuration = (wholenote) / divider;
    }
    else if (divider < 0) { //Dotted note = negative duration, for simplicity
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; //Dotted note = 1.5 time duration
    }
    tone(buzzer, tune[thisNote], noteDuration * 0.9); //Only play 90% of note to distinguish notes better
    delay(noteDuration);
    noTone(buzzer);
  }
  delay(800); //Delay for 800ms to correspond with sensor readings
}
