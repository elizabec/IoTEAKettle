#include <Wire.h>
#include <OneWire.h>
#include <Notes.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <arduino_secrets.h>
#include <stdlib.h>

#define addFX29 0x28
#define MAXLEN 10
#define TEMP_ERROR 999.999
#define KETTLE_LOAD 1300
#define MINWATER 1550
#define LED_GREEN 3
#define LED_YELLOW 4
#define LED_RED 5
#define BUZZER 11
#define KETTLE 8
#define TO_KG 10 * 10 * 4.44822 / 14000 / 9.81 // lb Span * lb %age * lbf -> N / Pspan / g

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

