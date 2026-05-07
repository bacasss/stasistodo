#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>

#define TFT_CS    1
#define TFT_RST   2
#define TFT_DC    3
#define TFT_SCLK  4
#define TFT_MOSI  5

const int btnDown     = 6; 
const int btnComplete = 7; 
const int btnRefresh  = 10; 

const char* SSID = "YOUR WIFI SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";
const char* apiToken = "todoist token";

String taskNames[5]; 
String taskIDs[5]; 
int selected = 0;
bool update = true; 

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
    Serial.begin(115200);

    pinMode(btnDown, INPUT_PULLUP);
    pinMode(btnComplete, INPUT_PULLUP);
    pinMode(btnRefresh, INPUT_PULLUP);

    tft.initR(INITR_BLACKTAB); // the type of screen
    tft.setRotation(1); // this makes the screen landscape! remove this line for portrait
    Serial.println("TFT Initialized!");
    tft.fillScreen(ST77XX_BLACK); // make sure there is nothing in the buffer


    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    tft.setCursor(0,0); // make the cursor at the top left
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.println("Connecting to WiFi...");


    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected!\n");
    tft.fillScreen(ST77XX_BLACK);
    tft.println("Connected!");
      delay(1000);

    HTTPClient http;
    http.begin("https://api.todoist.com/rest/v2/tasks");
    http.addHeader("Authorization", "Bearer " + String(apiToken));
    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        deserializeJson(doc, payload);
        JsonArray array = doc.as<JsonArray>();
        for (int i = 0; i < 5; i++) {
            if (i < array.size()) {
                taskNames[i] = array[i]["content"].as<String>();
                taskIDs[i]   = array[i]["id"].as<String>();
            } else {
                taskNames[i] = "no task!!";
                taskIDs[i]   = "";
            }
        }
    }
    http.end();
}

void loop() {
    if (digitalRead(btnDown) == LOW) {
        selected++;
        if (selected > 4) {
            selected = 0;
        }
        update = true;
        delay(250);
        //scrolling select thingy button
    }

    if (digitalRead(btnComplete) == LOW) {
        String idToClose = taskIDs[selected];
        if (idToClose != "") {
            tft.fillScreen(ST77XX_RED);
            tft.setCursor(10, 50);
            tft.println("COMPLETING...");

            HTTPClient http;
            String url = "https://api.todoist.com/rest/v2/tasks/" + idToClose + "/close";
            http.begin(url);
            http.addHeader("Authorization", "Bearer " + String(apiToken));
            http.POST("");
            http.end();
            delay(500);
            update = true;
            //complete button
        }
    }

    if (digitalRead(btnRefresh) == LOW || update == true) {
        if (digitalRead(btnRefresh) == LOW) {
            HTTPClient http;
            http.begin("https://api.todoist.com/rest/v2/tasks");
            http.addHeader("Authorization", "Bearer " + String(apiToken));
            int httpCode = http.GET();
            if (httpCode == 200) {
                String payload = http.getString();
                DynamicJsonDocument doc(4096);
                deserializeJson(doc, payload);
                JsonArray array = doc.as<JsonArray>();
                for (int i = 0; i < 5; i++) {
                    if (i < array.size()) {
                        taskNames[i] = array[i]["content"].as<String>();
                        taskIDs[i]   = array[i]["id"].as<String>();
                    } else {
                        taskNames[i] = "no task here!!";
                        taskIDs[i]   = "";
                    }
                }
            }
            http.end();
            //refresh list buttonnn
        }

        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(ST77XX_CYAN);
        tft.println("taskssssssssssssssss");
        tft.println(" ");

        for (int i = 0; i < 5; i++) {
            tft.setCursor(0, 25 + (i * 20));
            if (i == selected) {
                tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
                tft.print("> ");
                tft.println(taskNames[i]);
            } else {
                tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
                tft.print("  ");
                tft.println(taskNames[i]);
            }
        }
        update = false;
        delay(200);
    }
}