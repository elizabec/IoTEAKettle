#include <Helper_JW.h>

OneWire tempSensor(2); //Sensor pin = 2
WiFiClient wifi;
PubSubClient client(wifi);
char buffer[15];
uint8_t song_choice = 0;
int32_t temp = 0, load = 0, setTemp = 0;
char state[] = "0";

void updateTemp()
{
  tempSensor.reset();
  tempSensor.write(0xCC); //Alerts all devices to pay attention
  tempSensor.write(0x44); //Start a temperature reading

  delay(800); //A temperature reading takes 750ms

  tempSensor.reset();
  tempSensor.write(0xCC);
  tempSensor.write(0xBE); //Send content of scratchpad

  byte data[9];
  for (int i = 0; i < 9; i++)
  {
    data[i] = tempSensor.read();
  }

  temp = ((data[1] << 8) + data[0]) / 16.0;

  Serial.print("Temperature =  ");
  Serial.println(temp, 1);

  itoa(temp, buffer, 10);
  client.publish("Kettle/Temperature", buffer);

  if (temp >= setTemp && state[0] == '1')
  {
    digitalWrite(KETTLE, LOW);
    state[0] = '0';
    client.publish("Kettle/State", state);
    digitalWrite(LED_GREEN, HIGH);
    playTune(song_choice);
  }
  else
  {
    digitalWrite(LED_GREEN, LOW);
    noTone(BUZZER);
  }
}

void updateLoad()
{
  Wire.beginTransmission(addFX29);
  Wire.write(0x51);
  int const nBytes = 4;
  long bytesRead[nBytes];

  Wire.requestFrom(addFX29, nBytes); //Request 4 bytes of data from device
  for (int i = 0; Wire.available(); i++)
    bytesRead[i] = Wire.read();
  Wire.endTransmission();

  if ((bytesRead[0] & 0xC0) == 0x00)
  {
    load = (bytesRead[0] << 8) | bytesRead[1];
    if (load > KETTLE_LOAD)
      digitalWrite(LED_YELLOW, HIGH);
    else
      digitalWrite(LED_YELLOW, LOW);

    Serial.print("Load = ");
    Serial.println(load);
    Serial.print("kilos (maybe) = ");
    Serial.println((load - KETTLE_LOAD) * TO_KG);
  }
  else
  {
    Serial.println("Error!");
  }

  static uint8_t load_counter = 0;
  if (++load_counter >= 30)
  {
    sprintf(buffer, "%d", load);
    client.publish("Kettle/Weight", buffer);
    load_counter = 0;
  }
}

void connect_WIFI()
{
  Serial.print("Connecting WiFi...");
  for (int status = WL_IDLE_STATUS; status != WL_CONNECTED; )
  {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  Serial.print("signal strength (RSSI):");
  Serial.println(WiFi.RSSI());

  // print the encryption type:
  Serial.print("Encryption Type:");
  Serial.println(WiFi.encryptionType(), HEX);
}

void connect_MQTT()
{
  client.setServer("broker.emqx.io", 1883);
  client.setCallback([](char *topic, byte *payload, unsigned int length) {
    Serial.print("Topic: ");
    Serial.println(topic);
    if (strncmp(topic, "Kettle/HeatTemp", 15) == 0)
    {
      setTemp = atof((char *)payload); // Not sure if the compiler will like this typecast. Possibly not null terminated?
      Serial.print("  Set Temperature = ");
      Serial.println(setTemp);
      if (setTemp > temp)
      {
        if (load > MINWATER)
        {
          // START BOILING
          digitalWrite(KETTLE, HIGH);
          digitalWrite(LED_RED, LOW);
          state[0] = '1';
          client.publish("Kettle/State", state);
        }
        else
        {
          digitalWrite(LED_RED, HIGH);
          state[0] = '0';
          client.publish("Kettle/State", state);
        }
      }
    }
    else if (strncmp(topic, "Kettle/Song", 11) == 0)
    {
      song_choice = atoi((char *)payload);
      Serial.print("  Song Choice = ");
      Serial.println(song_choice);
    }
  });

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT....");
    if (client.connect("mqttjs_13cc164d", NULL, NULL, "Kettle/Connected", 2, true, "0"))
    {
      Serial.println("MQTT Connected!");
      client.publish("Kettle/Connected", "1", true);
      client.subscribe("Kettle/HeatTemp");
      client.subscribe("Kettle/Song");
    }
    else
    {
      Serial.print("Failed, state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  connect_WIFI();
  connect_MQTT();
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(KETTLE, OUTPUT);
}

void loop()
{
  client.loop();
  updateLoad();
  updateTemp();
}
void playTune(int c)
{
  int victory[] = {C5, 12, C5, 12, C5, 12, C5, 4, GS4, 4, AS4, 4, C5, 12, REST, 12, AS4, 12, C5, -2, REST, 4, REST, 4,
                   REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4,
                   REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4};

  int songoftime[] = {A4, 4, D4, 2, F4, 4, A4, 4, D4, 2,
                      F4, 4, A4, 8, C5, 8, NOTE_B4, 4, G4, 4, F4, 8, G4, 8, A4, 4,
                      D4, 4, C4, 8, E4, 8, D4, -1, REST, 4, REST, 4, REST, 4, REST, 4,
                      REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4, REST, 4};

  int mario[] = {E5, 8, E5, 8, REST, 8, E5, 8, REST, 8, C5, 8, E5, 8,
                 G5, 4, REST, 4, G4, 8, REST, 4,
                 C5, -4, G4, 8, REST, 4, E4, -4,
                 A4, 4, NOTE_B4, 4, AS4, 8, A4, 4,
                 G4, -8, E5, -8, G5, -8, A5, 4, F5, 8, G5, 8,
                 REST, 8, E5, 4, C5, 8, D5, 8, NOTE_B4, -4};

  int deftune[] = {A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, FS5, -8, FS5, -8, E5, -4,
                   A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, E5, -8, E5, -8, D5, -8, CS5, 16, NOTE_B4, -8,
                   A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, D5, 4, E5, 8, CS5, -8, NOTE_B4, 16, A4, 8, A4, 8, A4, 8,
                   E5, 4, D5, 2, REST, 4};
  int tempo = 100;
  int *tune = NULL;
  int const notes = 60;

  if (c == 1)
  {
    tune = victory;
    tempo = 115;
  }
  else if (c == 2)
  {
    tune = songoftime;
    tempo = 108;
  }
  else if (c == 3)
  {
    tune = mario;
    tempo = 200;
  }
  else
  {
    tune = deftune;
    tempo = 115;
  }

  int const wholenote = (60000 * 4) / tempo; //Whole note in ms
  for (int i = 0; i < notes; i += 2)
  {
    int const duration = (tune[i + 1] > 0) ? wholenote / tune[i + 1] : wholenote * 1.5 / abs(tune[i + 1]);
    tone(BUZZER, tune[i], duration * 0.9); //Only play 90% of note to distinguish notes better
    delay(duration);
    noTone(BUZZER);
  }
  delay(800); //Delay for 800ms to correspond with sensor readings
}
