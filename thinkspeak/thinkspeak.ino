#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "P519";
const char* password = "deocomatkhau";

WiFiClient client;
const unsigned long myChannelNumber = 2983255;
const char* myWriteAPIKey = "UDKENP5N91RRTHB5";
const unsigned long timerDelay = 30000;

unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {
  if (millis() - lastTime > timerDelay) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to WiFi");
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(5000);
        Serial.print(".");
      }
      Serial.println("\nConnected.");
    }

    float mq2 = random(100, 1001);  
    Serial.println("MQ2: " + String(mq2));

    int x = ThingSpeak.writeField(myChannelNumber, 1, mq2, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Error updating channel. HTTP code: " + String(x));
    }

    lastTime = millis();
  }
}
