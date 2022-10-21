/**The MIT License (MIT)
 
 Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 *********************************************************************************************
 improved and translate 2022 by Mario Ammerschuber - github "ammerscm68" - www.intermario.de 
 *********************************************************************************************
 */

/***********************************************************
 * Important: see settings.h to configure your settings!!!
 * *********************************************************/
#include "settings.h"
#include "time.h"
#include "LittleFS.h"       // Little Filesystem - über Bibliothek installieren
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>    // über Bibliothek installieren

#include <XPT2046_Touchscreen.h>
#include "TouchControllerWS.h"
#include "SunMoonCalc.h"

#include <ESP8266WebServer.h>
ESP8266WebServer  server(80);

/***
   Install the following libraries through Arduino Library Manager
   - Mini Grafx by Daniel Eichhorn
   - ESP8266 WeatherStation by Daniel Eichhorn
   - Json Streaming Parser by Daniel Eichhorn
 ***/

#include <JsonListener.h>
#include <OpenWeatherMapCurrent.h>
#include <OpenWeatherMapForecast.h>
#include <MiniGrafx.h>
#include <Carousel.h>
#include <ILI9341_SPI.h>

#include "ArialRounded.h"
#include "moonphases.h"
#include "weathericons.h"


#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_YELLOW 2
#define MINI_BLUE 3

#define MAX_FORECASTS 12

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C
                     }; //3

// Limited to 4 colors due to memory constraints
int BITS_PER_PIXEL = 2; // 2^2 =  4 colors

ADC_MODE(ADC_VCC);

ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);
Carousel carousel(&gfx, 0, 0, 240, 100);


XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
TouchControllerWS touchController(&ts);

void calibrationCallback(int16_t x, int16_t y);
CalibrationCallback calibration = &calibrationCallback;

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];

SunMoonCalc::Moon moonData;

char determineMoonIcon();
String getTime(time_t *timestamp);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);

FrameCallback frames[] = { drawForecast1, drawForecast2, drawForecast3 };
int frameCount = 3;

// how many different screens do we have?
int screenCount = 6;
long lastDownloadUpdate = millis();

uint8_t screen = 0;
// divide screen into 4 quadrants "< top", "> bottom", " < middle "," > middle "
uint16_t dividerTop, dividerBottom, dividerMiddle;
uint8_t changeScreen(TS_Point p, uint8_t screen);

long timerPress;
bool canBtnPress;
bool sleep_mode();
char* make12_24(int hour);

bool isFSMounted = false;
int WiFiConnectLoop = 0;
int MaxWiFiConnectLoop = 350;  // Maximale Anzahl Loops bei Verbindung mit dem WLAN

int waitloop = 0; 
int WSReset = 0;
String UWD = "10";
String LastWeatherUpdateTime = "";

//flag for saving data WiFiManager
bool shouldSaveConfig = false;

long lastDrew = 0;
bool btnClick;
uint8_t MAX_TOUCHPOINTS = 10;
TS_Point points[10];
uint8_t currentTouchPoint = 0;

// *****************************************************************************************************************************************************

void setup() {
  Serial.begin(115200);

  clearscreen(); // Serial Monitor Clear

  // ***First Start***
     // Little-Filesystem formatieren
     // LittleFS.format(); // alle Dateien löschen danch wieder neu erstellen
  
  Serial.println("Mounting file system...");
  isFSMounted = LittleFS.begin();
  if (!isFSMounted) {
    Serial.println("Formatting file system...");
    drawProgress(50, "Formatierung Dateisystem");
    LittleFS.format();
    drawProgress(100, "Formatierung abgeschlossen");
    MyWaitLoop(10000);
    ESP.restart();
  }

  loadPropertiesFromLittlefs(); // Load Adjustments

  // The LED pin needs to set HIGH
  // Use this pin to save energy
  // Turn on the background LED
  Serial.println(TFT_LED);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);    // HIGH to Turn on;

  gfx.init();
  gfx.fillBuffer(MINI_BLACK);
  gfx.commit();

  if (WIFI_SSID == "" | WIFI_PASS == "" | OPEN_WEATHER_MAP_API_KEY == "" | OPEN_WEATHER_MAP_LOCATION_ID == "") {
  Serial.println("Starte Access Point ... [Wetterstation]"); 
  Serial.println("");
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(20, 5, ThingPulseLogo);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 90, "Wetterstation");
  gfx.drawString(120, 160, F("WiFi-Manager gestartet\n (IP-Adresse: 192.168.10.1)"));
  gfx.commit();
  CaptivePortal();  // Start Captive Portal (Access Point)
  } else
  {  
  // Normal Start  
  Serial.println("Initializing touch screen...");
  ts.begin();
  /* Allow user to force a screen re-calibration  */
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(20, 5, ThingPulseLogo);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 90, "Wetterstation");
  gfx.drawString(120, 160, F("Drücken und Halten\n Initialisierung Touch Screen\nKalibrierung"));
  gfx.commit();
  delay(3000);
  yield();
  boolean isCalibrationAvailable = touchController.loadCalibration();
  if(ts.touched()) {
    isCalibrationAvailable = false;
    gfx.fillBuffer(MINI_YELLOW);
    gfx.drawString(120, 160, F("Kalibrierung initialisiert\nFreigabe Bildschirm"));
    gfx.commit();

    // Wait for release otherwise touch becomes first calibration point
    while(ts.touched()) { 
      delay(10);
      yield();
    }
    delay(100); // debounce
    touchController.getPoint(); // throw away last point
  }

  if (!isCalibrationAvailable) {
    Serial.println("Calibration data not available or force calibration initiated");
    touchController.startCalibration(&calibration);
    while (!touchController.isCalibrationFinished()) {
      gfx.fillBuffer(0);
      gfx.setColor(MINI_YELLOW);
      gfx.setTextAlignment(TEXT_ALIGN_CENTER);
      gfx.drawString(120, 160, "Bitte Touch Screen Kalibrieren\n(Punkt berühren...)");
      touchController.continueCalibration();
      gfx.commit();
      yield();
    }
    touchController.saveCalibration();
    ESP.restart(); 
  } else 
      {
      if (isFSMounted == true) {  
      // Erase Calibration Data 
      /*if (LittleFS.exists("/calibration.txt")){
      LittleFS.remove("/calibration.txt");
      ESP.restart();}*/
      }
      }
  dividerTop = 0; // Default 64
  dividerBottom = gfx.getHeight() - dividerTop;
  dividerMiddle = gfx.getWidth() / 2;
  Serial.println("dividerTop = "+ (String)dividerTop);
  Serial.println("dividerBottom = "+ (String)dividerBottom);
  Serial.println("dividerMiddle = "+ (String)dividerMiddle);

  connectWifi(); // mit WLAN verbinden

  // #######################################################################################
  // Arduino OTA/DNS
  Serial.println("Arduino OTA/DNS-Server starten ... - URL: http://wetterstation.local");
  ArduinoOTA.setHostname(WIFI_HOSTNAME);
  ArduinoOTA.setPassword("74656"); // default Passwort for OTA
  ArduinoOTA.onEnd([]() {Serial.println("\nEnd");});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));});
  ArduinoOTA.onError([](ota_error_t error) {(void)error;ESP.restart();}); // Restart bei OTA Fehler
  // Arduino jetzt OTA/DNS starten
  ArduinoOTA.begin();
  // #####################################################################################

  carousel.setFrames(frames, frameCount);
  carousel.disableAllIndicators();

  initTime();

  // update the weather information when Access data correct
  int OWMupdateData = 0;
  updateData();
  while (currentWeather.cityName == "") {
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(20, 5, ThingPulseLogo);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 90, "Wetterstation");
  gfx.drawString(120, 150, F("Die Wetterdaten konnten\n nicht geladen werden !"));
  gfx.drawString(120, 200, F("(Eventuell falsche Zugangsdaten)"));
  OWMupdateData += 1; 
  gfx.drawString(120, 260, "Versuch: " +(String)OWMupdateData);
  gfx.commit();   
  MyWaitLoop(5000);
  updateData(); // Try Next Update
  if (OWMupdateData > 4) {ResetWeatherStation();}}

  timerPress = millis();
  canBtnPress = true;

// Get all information of your LittleFS
    if (isFSMounted == true)
    {
    Serial.println(F("Little Filesystem Init - done."));
    FSInfo fs_info;
    LittleFS.info(fs_info);
    Serial.println("File sistem info.");
    Serial.print("Total space:      ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Total space used: ");
    Serial.print(fs_info.usedBytes);
    Serial.println("byte");
    Serial.print("Block size:       ");
    Serial.print(fs_info.blockSize);
    Serial.println("byte");
    Serial.print("Page size:        ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Max open files:   ");
    Serial.println(fs_info.maxOpenFiles);
    Serial.print("Max path length:  ");
    Serial.println(fs_info.maxPathLength);
    Serial.println();
    // Open dir folder
    Dir dir = LittleFS.openDir("/");
    // Cycle all the content
    while (dir.next()) {
        // get filename
        Serial.print(dir.fileName());
        Serial.print(" - ");
        // If element have a size display It else write 0
        if(dir.fileSize()) {
            File f = dir.openFile("r");
            Serial.println(f.size());
            f.close();
        }else{
            Serial.println("0");
        }   
    }
   Serial.println("_______________________________");
   Serial.println("");
   }
   else
     {
     Serial.println("Little Filesystem Init - Fehler.");      
     }

  Serial.println("Start Wetterstation ...."); 
  }   
}

// *****************************************************************************************************************************************************

void loop() {
if (WIFI_SSID == "" | WIFI_PASS == "" | OPEN_WEATHER_MAP_API_KEY == "" | OPEN_WEATHER_MAP_LOCATION_ID == "") {server.handleClient();} else
{  
   WiFi.mode(WIFI_STA);
  if (WiFi.status() == WL_CONNECTED) 
  {
  ArduinoOTA.handle();  
  static bool asleep = false;	//  asleep used to stop screen change after touch for wake-up
  gfx.fillBuffer(MINI_BLACK);

  /* Break up the screen into 4 sections a touch in section:
   * - Top changes the time format
   * - Left back one page
   * - Right forward one page
   * - Bottom jump to page 0
   */
  if (touchController.isTouched(500)) {
    TS_Point p = touchController.getPoint();
    timerPress = millis();
    if (!asleep) { 				// no need to update or change screens;
    	screen = changeScreen(p, screen);
    }
  } // isTouched()

  if (!(asleep = sleep_mode())) {
    if (screen == 0) {
      drawTime();

      drawWifiQuality();
      int remainingTimeBudget = carousel.update();
      if (remainingTimeBudget > 0) {
        // You can do some work here
        // Don't do stuff if you are below your
        // time budget.
        delay(remainingTimeBudget);
      }
      drawCurrentWeather();
      drawAstronomy();
    } else if (screen == 1) {
      WSReset = MaxWiFiConnectLoop-100; // for Weatherstation Reset
      drawCurrentWeatherDetail();
    } else if (screen == 2) {
      WSReset = MaxWiFiConnectLoop-100; // for Weatherstation Reset
      drawForecastTable(0);
    } else if (screen == 3) {
      WSReset = MaxWiFiConnectLoop-100; // for Weatherstation Reset
      drawForecastTable(4);
    } else if (screen == 4) {
      WSReset = MaxWiFiConnectLoop-100; // for Weatherstation Reset
      drawAbout();
    } else if (screen == 5) {
    ResetWeatherStationScreen();  
    }
    gfx.commit();

    // Check if we should update weather information
    if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) {
      updateData();
      lastDownloadUpdate = millis();
    }
  } //!asleep
  } else {connectWifi();}
} 
}
// *****************************************************************************************************************************************************
// *****************************************************************************************************************************************************
