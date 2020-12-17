#include <Helper.h>


OneWire tempSensor(2); //Sensor pin = 2
WiFiClient wifi;
PubSubClient client(wifi);

char *dtostrf (double value, signed char width, unsigned char prec, char *sout) {
  char sBuf[20];
  sprintf(sBuf, "%%%d.%df", width, prec);
  sprintf(sout, sBuf, value);
  return sout;
}

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  for (int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i])); //TODO: topic determines which value should be used (temp vs song)
    buf[i] = payload[i];

  }
  Serial.println();
}
void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
}

void connSetup() {
  Serial.print("Connecting WiFi...");
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
  printCurrentNet();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT....");
    if (client.connect(mqtt_client, NULL, NULL, "Kettle/connected", 2, true, "false")) {
      Serial.println("MQTT Connected!");
    }
    else {
      Serial.print("failed, state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.publish("Kettle/connected", "true");
  client.subscribe("Kettle/HeatTemp");
}

void setup() {
  Wire.begin();
  connSetup();
  Serial.begin(9600);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
}

void loop() {

  client.loop();

  Wire.beginTransmission(addFX29);
  Wire.write(0x51);

  int nBytes = 4;
  long bytesRead[nBytes];

  Wire.requestFrom(addFX29, nBytes); //Request 4 bytes of data from device

  int index = 0;
  while (Wire.available()) {
    bytesRead[index] = Wire.read();
    index++;
  }

  float Pdisplay, Lmax = 100, Lmin = 0;
  uint32_t Pvalue, Pspan, I2C_ERR;
  uint16_t P1 = 1000, P2 = 15000;

  if ((bytesRead[0] & 0xC0) == 0x00) {
    Pvalue = (bytesRead[0] << 8) | bytesRead[1];
    I2C_ERR = 0; //All is well
  }
  
  else
    I2C_ERR = 1; //All is not well
    
  if (I2C_ERR == 0) {
    Pspan = P2 - P1; //Span of values
    Pdisplay = (Pvalue - 1000) * 50 / Pspan; //Percentage

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
  itoa(temp, sendTemp, 10);
  sprintf(sendP, "%d", Pvalue);
  setTemp = atof(buf);

  Serial.print("Send Temp = ");
  Serial.println(sendTemp);
  client.publish("Kettle/Temperature", sendTemp);
  client.publish("Kettle/Weight", sendP);

  if (setTemp > temp) {
    
    if(Pvalue > MINWATER){
    //digitalWrite(kettle, HIGH);
    client.publish("Kettle/State", "1");
    }
    
    else{
      digitalWrite(ledRed, HIGH);
      client.publish("Kettle/State", "0");
    }
  }
  
  if (temp >= setTemp) {
    //digitalWrite(kettle, LOW);
    client.publish("Kettle/State", "0");
    digitalWrite(ledGreen, HIGH);
    //playTune(0);
  }
  
  else {
    digitalWrite(ledGreen, LOW);
    noTone(buzzer);
  }
  
  if (Pvalue >= EMPTYKETTLE) {
    digitalWrite(ledYellow, HIGH);
  }
  
  else {
    digitalWrite(ledYellow, LOW);
  }
  // Values for debugging
  Serial.print("Temperature =  ");
  Serial.print(temp, 1);
  Serial.print("  Pvalue = ");
  
  if (I2C_ERR == 0) {
    Serial.println(Pvalue);
  }
  
  else if (I2C_ERR == 1) {
    Serial.println("Error!");
  }
}

void playTune(int choice) {
  
  int tune[arrlen];
  
  if (choice == 1) {
    for(int i = 0; i < arrlen; i++){
      tune[i] = victory[i];
    }
    tempo = 115;
  }
  
  else if (choice == 2) {
    for(int i = 0; i < arrlen; i++){
      tune[i] = songoftime[i];
    }
    tempo = 108;
  }
  
  else if (choice == 3) {
    for(int i = 0; i < arrlen; i++){
      tune[i] = mario[i];
    }
    tempo = 200;
  }
  
  else {
    for(int i = 0; i < arrlen; i++){
      tune[i] = deftune[i];
    }
    tempo = 115;
  }

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
