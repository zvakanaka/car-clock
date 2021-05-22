#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdbool.h>
#include "uRTCLib.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int btnPin1 = 2; // d4
const int btnPin2 = 13; // d7
const int btnPin3 = 14;

uRTCLib rtc;

unsigned long previousMillis = 0;
unsigned long clockMillis = 0;
const long interval = 100;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool dArr[SCREEN_HEIGHT][SCREEN_WIDTH];

void checkButtons();
void restoreDisplay();
void storeDisplay();
void detail(Adafruit_SSD1306 *display, int fahrenheit, int hour, int minute, int second);
void temperature(Adafruit_SSD1306 *display, int fahrenheit, int hour, int minute, int second);

int btn1State = HIGH;
int btn2State = HIGH;
int btn3State = HIGH;

float getFahrenheit(float rtcTemp) {
  return rtcTemp / 100 * 9 / 5 + 32;
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Ready!");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println(F("SSD1306 allocation failed."));
    for (;;);
  }
  Serial.println("SSD1306 allocated!");
  pinMode(btnPin1, INPUT_PULLUP);
  pinMode(btnPin2, INPUT_PULLUP);
  pinMode(btnPin3, INPUT_PULLUP);

#ifdef ARDUINO_ARCH_ESP8266
  URTCLIB_WIRE.begin(4, 5); // D3 and D4 on ESP8266
#else
  URTCLIB_WIRE.begin();
#endif
  rtc.set_rtc_address(0x68);
  //  rtc.set_model(URTCLIB_MODEL_DS3232);
  rtc.set_model(URTCLIB_MODEL_DS3231);

  if (rtc.enableBattery()) {
    Serial.println("Battery activated correctly.");
  } else {
    Serial.println("ERROR activating battery.");
  }
  Serial.print("Lost power status: ");
  if (rtc.lostPower()) {
    Serial.print("POWER FAILED. Clearing flag...");
    rtc.lostPowerClear();
    Serial.println(" done.");
  } else {
    Serial.println("POWER OK");
  }

  // uncomment if you want to manually set date/time - otherwise use buttons
  //  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
  //    rtc.set(30, 45, 16, 1, 9, 2, 21);


  // fill the stored screen 2d-array with BLACK
  memset(dArr, false, sizeof(dArr[0][0]) * SCREEN_WIDTH * SCREEN_HEIGHT);

  // flip upside-down
  display.setRotation(2);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 20);
  display.println("\"Let God Prevail\"");
  display.setCursor(11, 35);
  display.println("- President Nelson");
  display.display();
  storeDisplay();
  delay(500);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - clockMillis >= 1000) {
    clockMillis = currentMillis;
    rtc.refresh();
  }
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    rtc.refresh();

    int hour = rtc.hour();
    int minute = rtc.minute();
    int second = rtc.second();

    float fahrenheit = getFahrenheit((float)rtc.temp());
    int fahrenheitDisplay = fahrenheit;

    display.clearDisplay();
    detail(&display, fahrenheitDisplay, hour, minute, second);
    //    temperature(&display, fahrenheitDisplay, hour, minute, second);
    display.display();

    checkButtons();
  }
}
bool resetMode = false;
void checkButtons() {
  int hour = rtc.hour();
  int minute = rtc.minute();
  int second = rtc.second();

  if (digitalRead(btnPin1) == LOW) {
    // btn1State is used to avoid sequential toggles
    if (resetMode && btn1State != LOW) {
      Serial.println("Button 1 pressed");
      //  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
      rtc.set(second, (minute + 1) % 60, hour, 1, 9, 2, 21);
    }
    btn1State = LOW;
  } else {
    btn1State = HIGH;
  }

  if (digitalRead(btnPin2) == LOW) {
    // btnState is used to avoid sequential toggles
    if (resetMode && btn2State != LOW) {
      Serial.println("Button 2 pressed");
      //  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
      rtc.set(second, minute, (hour + 1) % 24, 1, 9, 2, 21);
    }
    btn2State = LOW;
  } else {
    btn2State = HIGH;
  }

  if (digitalRead(btnPin3) == LOW) {
    // btnState is used to avoid sequential toggles
    if (btn3State != LOW) {
      Serial.println("Button 3 pressed");
      resetMode = !resetMode;
    }
    btn3State = LOW;
  } else {
    btn3State = HIGH;
  }
}


void detail(Adafruit_SSD1306 *display, int fahrenheit, int hour, int minute, int second) {
  display->setTextSize(2);
  display->setCursor(128 / 2 - (30 / 2), 0);
  display->print(fahrenheit);
  display->print(" F");

  display->setTextSize(3);
  
  display->setCursor(0, 22);
  if (hour > 12) display->print(hour - 12);
  else if (hour == 0) display->print(12);
  else display->print(hour);
  
  const bool longHour = hour > 12 ? hour - 12 >= 10 : hour >= 10;
  if (longHour) display->setCursor(32, 22);
  display->print(':');
  if (longHour) display->setCursor(44, 22);

  if (minute < 10) display->print('0');
  display->print(minute);
  if (longHour) display->setCursor(74, 22);
  display->print(':');
  if (longHour) display->setCursor(88, 22);

  if (second < 10) display->print('0');
  display->print(second);

  display->setTextSize(1);
  display->setCursor(110, 52);
  if (hour >= 12) display->print("PM");
  else display->print("AM");

  // Serial.print(hour);
  // Serial.print(':');
  // Serial.print(minute);
  // Serial.print(':');
  // Serial.println(second);

  if (resetMode) {
    display->drawPixel(0, 0, WHITE);
    display->drawPixel(127, 0, WHITE);
    display->drawPixel(0, 63, WHITE);
    display->drawPixel(127, 63, WHITE);
  }
}

void temperature(Adafruit_SSD1306 *display, int fahrenheit, int hour, int minute, int second) {
  display->setTextSize(4);
  display->setCursor(20, 20);
  display->print(fahrenheit);
  display->print(" F");

  display->setTextSize(1);
  display->setCursor(5, 55);
  if (hour >= 13) display->print(hour - 12);
  else display->print(hour);
  display->print(':');

  if (minute < 10) display->print('0');
  display->print(minute);
  display->print(':');

  if (second < 10) display->print('0');
  display->print(second);

  if (hour >= 12) display->print("PM");
  else display->print("AM");
}

void restoreDisplay() {
  display.clearDisplay();
  for (int y = 0; y < SCREEN_HEIGHT; y++ ) {
    for (int x = 0; x < SCREEN_WIDTH; x++ ) {
      display.drawPixel(x, y, dArr[y][x] == true ? WHITE : BLACK);
    }
  }
  display.display();
}
void storeDisplay() {
  for (int y = 0; y < SCREEN_HEIGHT; y++ ) {
    for (int x = 0; x < SCREEN_WIDTH; x++ ) {
      dArr[y][x] = display.getPixel(x, y) == WHITE ? true : false;
    }
  }
}
