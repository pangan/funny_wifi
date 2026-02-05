#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <vector>

#include "my_wifi_manager.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define SDA_PIN 6 // Changed from 8 to avoid onboard LED conflict
#define SCL_PIN 7 // Changed from 9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int x, minX;
int currentCons = 0;
int totalCons = 0;
int previousStationCount = 0;

std::vector<String> emailList = {
 
};
String scrollText = "";

void updateScrollText() {
  scrollText = "";
  for (const auto& entry : emailList) {
    scrollText += entry;
    scrollText += "   ";
  }
  minX = -6 * (int)scrollText.length();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  updateScrollText();

  x = display.width();
  
  setAddUserCallback([](String email, String name) {
    emailList.push_back(email + "(" + name + ")");
    updateScrollText();
  });
  
  startPortal(1, 2, 3, 4);
}

void loop() {
  wifiLoop();
  
  int stationNum = WiFi.softAPgetStationNum();
  if (stationNum > previousStationCount) {
    totalCons += (stationNum - previousStationCount);
  }
  previousStationCount = stationNum;
  currentCons = stationNum;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Current cons: "));
  display.println(currentCons);
  display.setCursor(0, 12);
  display.print(F("Total cons: "));
  display.println(totalCons);
  display.setCursor(x, 24);
  display.println(scrollText);
  display.display();

  x = x - 1; // Adjust scrolling speed here
  if(x < minX) x = display.width();
  //delay(1);
}
