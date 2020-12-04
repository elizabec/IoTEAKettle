#include <Wire.h>
#include <OneWire.h> // library download
#include <TempSensor.h> // custom header!
#include <Notes.h>

#define ONE_WIRE_BUS 2
#define addFX29 0x28
#define MAXLEN 10
OneWire oneWire(ONE_WIRE_BUS);

float values[10]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float temp = 0;
float setTemp = 60;
byte pos = 0;
int ledGreen = 3;
int ledYellow = 4;
int ledWhite = 5;
int buzzer = 8;

DallasTemperature sensors(&oneWire);

int victory[] = {
  C5, 12, C5, 12, C5, 12, C5, 4, GS4, 4, AS4, 4, C5, 12, REST, 12, AS4, 12, C5, -2

};
int songOfTime[] = {

  A5, 4, D4, 2, F4, 4, A4, 4, D4, 2,
  F4, 4, A5, 8, C5, 8, NOTE_B5, 4, G4, 4, F4, 8, G4, 8, A5, 4,
  D4, 4, C4, 8, E4, 4, D4, -1
};

void setup() {
  Wire.begin();
  Serial.begin(9600);
  sensors.begin();
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledWhite, OUTPUT);
}

void loop() {

  Wire.beginTransmission(addFX29);
  Wire.write(0x51);

  int nBytes = 4;
  long bytesRead[nBytes];

  Wire.requestFrom(addFX29, nBytes);

  int cnt = 0;
  while (Wire.available())
  {
    bytesRead[cnt] = Wire.read();
    cnt++;
  }

  float Pdisplay, Lmax = 100, Lmin = 0;
  uint32_t Pvalue, Pspan, I2C_ERR;
  uint16_t P1 = 1000, P2 = 10000;

  if ((bytesRead[0] & 0xc0) == 0x00)
  {
    Pvalue = (bytesRead[0] << 8)   |   bytesRead[1];
    I2C_ERR = 0;
  }
  else
    I2C_ERR = 1;

  if (I2C_ERR == 0)
  {
    Pspan = P2 - P1;
    Pdisplay = (Pvalue - 1000) * (Lmax - Lmin) / Pspan;

    if (Pdisplay >= 0 && Pdisplay <= 100) {
      values[pos] = Pdisplay;
      pos++;
      if (pos == MAXLEN) {
        pos = 0;
      }
    }
  }
  Wire.endTransmission();

  sensors.requestTemperatures(); // Send the command to get temperatures
  temp = sensors.getTempCByIndex(0);
  if (temp >= setTemp) {
    digitalWrite(ledGreen, HIGH);
    playTune(buzzer, 108);
  }
  else {
    digitalWrite(ledGreen, LOW);
    noTone(buzzer);
  }
  if (Pdisplay >= 4) {
    digitalWrite(ledYellow, HIGH);
  }
  else {
    digitalWrite(ledYellow, LOW);
  }
  Serial.print("Temperature =  ");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print("  Pdisplay = ");
  Serial.print(Pdisplay);
  Serial.print("  Pvalue = ");
  Serial.print(Pvalue);
  Serial.print("  Average = ");
  Serial.println(pAvg());
  digitalWrite(5, HIGH);
  delay(200);
}

float pAvg() {
  float sum = 0;
  for (int i = 0; i < MAXLEN; i++) {
    sum = sum + values[i];
  }
  return sum / MAXLEN;
}

void playTune(int pin, int tempo) {
  int notes = sizeof(victory) / sizeof(victory[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = victory[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    }
    else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(pin, victory[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(pin);
  }
}
