
#include <Wire.h>
#include <OneWire.h>
#include <Notes.h>
#include <WiFiNINA.h> //library to enable use of WiFi
#include <SPI.h>
#include <PubSubClient.h>
#include <arduino_secrets.h>
#include <stdlib.h>
#include <avr/dtostrf.h>

int victory[] = { C5, 12, C5, 12, C5, 12, C5, 4, GS4, 4, AS4, 4, C5, 12, REST, 12, AS4, 12, C5, -2 };

int songoftime[] = { A4, 4, D4, 2, F4, 4, A4, 4, D4, 2,
                    F4, 4, A4, 8, C4, 8, NOTE_B4, 4, G4, 4, F4, 8, G4, 8, A4, 4,
                    D4, 4, C4, 8, E4, 8, D4, -1
};

int mario[] = { E5, 8, E5, 8, REST, 8, E5, 8, REST, 8, C5, 8, E5, 4, //1
               G5, 4, REST, 4, G4, 8, REST, 4,
               C5, -4, G4, 8, REST, 4, E4, -4, // 3
               A4, 4, NOTE_B4, 4, AS4, 8, A4, 4,
               G4, -8, E5, -8, G5, -8, A5, 4, F5, 8, G5, 8,
               REST, 8, E5, 4, C5, 8, D5, 8, NOTE_B4, -4
};

int deftune[] = { D5, 2, E5, 8, FS5, 8, D5, 8, E5, 8, E5, 8, E5, 8, FS5, 8, E5, 4, A4, 4,
                 REST, 2, NOTE_B4, 8, CS5, 8, D5, 8, NOTE_B4, 8, REST, 8, E5, 8, FS5, 8, E5, -4,

                 A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, FS5, -8, FS5, -8, E5, -4,
                 A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, E5, -8, E5, -8, D5, -8, CS5, 16, NOTE_B4, -8,
                 A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16, D5, 4, E5, 8, CS5, -8, NOTE_B4, 16, A4 , 8, A4, 8, A4, 8,
                 E5, 4, D5, 2, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
                 FS5, -8, FS5, -8, E5, -4, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
                 A5, 4, CS5, 8, D5, -8, CS5, 16, NOTE_B4, 8, A4, 16, NOTE_B4, 16, D5, 16, NOTE_B4, 16,
                 D5, 4, E5, 8, CS5, -8, NOTE_B4, 16, A4, 4, A4, 8, E5, 4, D5, 2
};

char mqtt_broker[] = "broker.emqx.io";
char mqtt_client[] = "mqttjs_13cc164d";
const int mqtt_port = 1883;
float values[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
float temp = 0;
float setTemp = 60;
byte pos = 0;
int ledGreen = 3;
int ledYellow = 4;
int ledRed = 5;
int buzzer = 8;
int tempo = 100;
int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char sendTemp[15];
char sendP[10];
char buf[10];