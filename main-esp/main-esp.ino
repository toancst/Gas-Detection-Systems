#include <WiFi.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"

// WiFi config
const char* ssid = "UET-Wifi-Office-Free 2.4Ghz";
const char* password = "";

// ThingSpeak config
const unsigned long myChannelNumber = 2983255;
const char* myWriteAPIKey = "UDKENP5N91RRTHB5";
const unsigned long thingSpeakDelay = 30000;

const String phoneNumber = "84972334069";
const String apiKey = "4495495";

WiFiServer server(80);
WiFiClient thingSpeakClient;

int ppm = 0;
bool isRunning = true;
unsigned long lastBlinkTime = 0;
bool ledRedState = false;
unsigned long lastThingSpeakUpdate = 0;

bool alertSent800 = false;
bool alertSent1100 = false;
bool alertSent1300 = false;

const int ledGreen = 26;
const int ledYellow = 27;
const int ledRed = 25;
const int ledBlue = 32;  // NEW: blue LED
const int buzzer = 33;

void sendWhatsAppMessage(String message) {
  message.replace(" ", "%20");
  message.replace("!", "%21");
  message.replace(",", "%2C");
  message.replace(".", "%2E");

  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber +
               "&text=" + message + "&apikey=" + apiKey;

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("");
  http.end();
}

void sendToThingSpeak(float ppmValue) {
  ThingSpeak.writeField(myChannelNumber, 1, ppmValue, myWriteAPIKey);
}

void checkAlerts(int currentPpm) {
  if (currentPpm < 800) {
    alertSent800 = alertSent1100 = alertSent1300 = false;
    return;
  }

  if (currentPpm > 1300 && !alertSent1300) {
    sendWhatsAppMessage("EMERGENCY! Dangerous smoke levels detected! FIRE MAY HAVE STARTED! Evacuate immediately and call emergency services!");
    alertSent1300 = alertSent1100 = alertSent800 = true;
  } else if (currentPpm > 1100 && !alertSent1100) {
    sendWhatsAppMessage("Increasing smoke levels! Fire risk is rising. Please check heat sources and prepare safety measures.");
    alertSent1100 = alertSent800 = true;
  } else if (currentPpm > 800 && !alertSent800) {
    sendWhatsAppMessage("Smoke detected! Low level, but please check immediately. Ensure no flammable objects are near the sensor.");
    alertSent800 = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(buzzer, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected.");
  Serial.print("Local IP Address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(thingSpeakClient);
  server.begin();
}

void loop() {
  // UART Data
  static String uartBuffer = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      uartBuffer.trim();
      if (uartBuffer == "000000") {
        isRunning = false;
      } else if (uartBuffer == "111111") {
        isRunning = true;
      } else if (uartBuffer == "222222") {
        ppm = 0;
        alertSent800 = alertSent1100 = alertSent1300 = false;
      } else if (uartBuffer.startsWith("Gas:")) {
        ppm = uartBuffer.substring(4).toInt();
      }
      uartBuffer = "";
    } else {
      uartBuffer += c;
    }
  }

  // Điều khiển LED
  bool buzzerOn = false;
  bool ledGreenOn = false;
  bool ledYellowOn = false;
  bool ledRedOn = false;
  bool ledBlueOn = !isRunning;

  if (isRunning) {
    checkAlerts(ppm);
    ledGreenOn = ppm < 300;
    ledYellowOn = ppm >= 300 && ppm < 900;

    if (ppm >= 900) {
      int blinkFrequency = map(ppm, 900, 1600, 1, 10);
      blinkFrequency = constrain(blinkFrequency, 1, 10);
      if (millis() - lastBlinkTime >= 500 / blinkFrequency) {
        ledRedState = !ledRedState;
        digitalWrite(ledRed, ledRedState);
        lastBlinkTime = millis();
      }
      ledRedOn = ledRedState;
    } else {
      digitalWrite(ledRed, LOW);
      ledRedOn = false;
    }

    buzzerOn = ppm > 1200;
  } else {
    // Khi stop: chỉ bật led xanh
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, LOW);
  }

  digitalWrite(ledGreen, ledGreenOn);
  digitalWrite(ledYellow, ledYellowOn);
  if (isRunning && ppm < 900) digitalWrite(ledRed, LOW);
  digitalWrite(ledBlue, ledBlueOn);
  if (buzzerOn) tone(buzzer, 1000); else noTone(buzzer);

  // Web Giao diện
  WiFiClient client = server.available();
  if (client) {
    client.readStringUntil('\r');
    client.flush();

    String statusText = "SAFE", ppmClass = "safe";
    if (ppm > 1300) { statusText = "EMERGENCY!"; ppmClass = "emergency"; }
    else if (ppm > 1100) { statusText = "HIGH DANGER"; ppmClass = "danger"; }
    else if (ppm > 800) { statusText = "DANGER"; ppmClass = "danger"; }
    else if (ppm >= 300) { statusText = "CAUTION"; ppmClass = "caution"; }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html\n");

    client.println("<html><head>"
      "<meta http-equiv='refresh' content='1'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<title>Gas Monitor</title><style>"
      "body{font-family:sans-serif;background:#000;color:#fff;text-align:center;padding:10px}"
      "h1{color:#0ff} .status{font-size:28px;margin:10px;padding:10px;border-radius:8px}"
      ".safe{background:#006600} .caution{background:#999900} .danger{background:#cc3300} .emergency{background:#cc0000;animation:blink 1s infinite}"
      "@keyframes blink{0%{opacity:1}50%{opacity:0.2}100%{opacity:1}}"
      ".led-btn{padding:10px;margin:5px;border-radius:6px;width:120px;display:inline-block;font-weight:bold}"
      ".green{background:#00ff00;color:#000} .yellow{background:#ffff00;color:#000} .red{background:#ff0000;color:#fff}"
      ".blue{background:#00ccff;color:#000} .off{background:#444;color:#aaa}"
      ".btn{padding:10px 20px;margin:8px;border-radius:6px;font-weight:bold}"
      ".btn-primary{background:#00ff88;color:#000} .btn-danger{background:#ff4444;color:#fff} .btn-secondary{background:#888}"
      "</style></head><body>");

    client.println("<h1>Gas Monitoring System</h1>");
    client.println("<div class='status " + ppmClass + "'>" + String(ppm) + " PPM - " + statusText + "</div>");

    // LED trạng thái
    client.println("<div>");
    client.println("<div class='led-btn " + String(ledGreenOn ? "green" : "off") + "'>GREEN</div>");
    client.println("<div class='led-btn " + String(ledYellowOn ? "yellow" : "off") + "'>YELLOW</div>");
    client.println("<div class='led-btn " + String(ledRedOn ? "red" : "off") + "'>RED</div>");
    client.println("<div class='led-btn " + String(ledBlueOn ? "blue" : "off") + "'>BLUE</div>");
    client.println("</div>");

    client.println("<div><b>BUZZER:</b> " + String(buzzerOn ? "ON" : "OFF") + "</div>");
    client.println("<div class='btn " + String(isRunning ? "btn-primary" : "btn-danger") + "'>" + String(isRunning ? "RUNNING" : "STOPPED") + "</div>");
    client.println("<div class='btn btn-secondary'>RESET</div>");
    client.println("</body></html>");
    client.stop();
  }

  if (millis() - lastThingSpeakUpdate > thingSpeakDelay && isRunning) {
    sendToThingSpeak(ppm);
    lastThingSpeakUpdate = millis();
  }

  delay(100);
}
