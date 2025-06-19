#include <WiFi.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"

// Cấu hình WiFi
const char* ssid = "P519";
const char* password = "deocomatkhau";

// Cấu hình ThingSpeak
const unsigned long myChannelNumber = 2983255;
const char* myWriteAPIKey = "UDKENP5N91RRTHB5";
const unsigned long thingSpeakDelay = 30000; // 30 giây

// Cấu hình WhatsApp API
const String phoneNumber = "84972334069";
const String apiKey = "4495495";

WiFiServer server(80);
WiFiClient thingSpeakClient;

int ppm = 0;
bool isRunning = true;
unsigned long lastBlinkTime = 0;
bool ledRedState = false;
unsigned long lastThingSpeakUpdate = 0;

// Biến để theo dõi cảnh báo đã gửi
bool alertSent800 = false;
bool alertSent1100 = false;
bool alertSent1300 = false;

// GPIO Pins
const int ledGreen = 26;
const int ledYellow = 27;
const int ledRed = 25;
const int buzzer = 33;

void sendWhatsAppMessage(String message) {
  // URL encode the message
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

  if (httpResponseCode == 200) {
    Serial.println("WhatsApp message sent successfully");
  } else {
    Serial.println("Error sending WhatsApp message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void sendToThingSpeak(float ppmValue) {
  int responseCode = ThingSpeak.writeField(myChannelNumber, 1, ppmValue, myWriteAPIKey);
  
  if (responseCode == 200) {
    Serial.println("ThingSpeak update successful. PPM: " + String(ppmValue));
  } else {
    Serial.println("Error updating ThingSpeak. HTTP code: " + String(responseCode));
  }
}

void checkAlerts(int currentPpm) {
  // Reset alerts nếu ppm giảm xuống dưới 800
  if (currentPpm < 800) {
    alertSent800 = false;
    alertSent1100 = false;
    alertSent1300 = false;
    return;
  }
  
  // Cảnh báo mức 1300 (Khẩn cấp)
  if (currentPpm > 1300 && !alertSent1300) {
    sendWhatsAppMessage("EMERGENCY! Dangerous smoke levels detected! FIRE MAY HAVE STARTED! Evacuate immediately and call emergency services!");
    alertSent1300 = true;
    alertSent1100 = true; // Đánh dấu các mức thấp hơn cũng đã gửi
    alertSent800 = true;
  }
  // Cảnh báo mức 1100 (Nguy hiểm cao)
  else if (currentPpm > 1100 && !alertSent1100) {
    sendWhatsAppMessage("Increasing smoke levels! Fire risk is rising. Please check heat sources and prepare safety measures.");
    alertSent1100 = true;
    alertSent800 = true; // Đánh dấu mức thấp hơn cũng đã gửi
  }
  // Cảnh báo mức 800 (Nguy hiểm thấp)
  else if (currentPpm > 800 && !alertSent800) {
    sendWhatsAppMessage("Smoke detected! Low level, but please check immediately. Ensure no flammable objects are near the sensor.");
    alertSent800 = true;
  }
}

// Hàm tạo CSS tối ưu (lưu trong PROGMEM để tiết kiệm RAM)
String getCSS() {
  return F("*{box-sizing:border-box;margin:0;padding:0}"
           "body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0a0a0a;color:#e0e0e0;padding:10px;line-height:1.4}"
           ".container{max-width:500px;margin:0 auto;background:#1a1a1a;border-radius:12px;padding:20px;box-shadow:0 4px 20px rgba(0,0,0,0.3)}"
           "h1{color:#00ff88;font-size:24px;margin-bottom:5px;text-align:center}"
           "h3{color:#888;font-size:14px;margin-bottom:20px;text-align:center;font-weight:400}"
           ".ppm-display{background:linear-gradient(135deg,#2a2a2a,#1f1f1f);border-radius:8px;padding:15px;margin:15px 0;text-align:center;border:1px solid #333}"
           ".ppm-value{font-size:32px;font-weight:bold;margin-bottom:5px}"
           ".ppm-status{font-size:16px;font-weight:600;text-transform:uppercase;letter-spacing:1px}"
           ".safe{color:#00ff88;border-color:#00ff88}"
           ".caution{color:#ffaa00;border-color:#ffaa00}"
           ".danger{color:#ff4444;border-color:#ff4444}"
           ".emergency{color:#ff0000;border-color:#ff0000;animation:pulse 1s infinite}"
           "@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.7}}"
           ".status-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:15px 0}"
           ".status-item{background:#2a2a2a;padding:10px;border-radius:6px;text-align:center;border:1px solid #333}"
           ".status-label{font-size:12px;color:#888;margin-bottom:3px}"
           ".status-value{font-size:14px;font-weight:bold}"
           ".led-on{color:#00ff88}"
           ".led-blink{color:#ff4444;animation:blink 0.5s infinite}"
           ".buzzer-on{color:#ff0000}"
           "@keyframes blink{0%,50%{opacity:1}51%,100%{opacity:0.3}}"
           ".alert-section{margin:15px 0;padding:15px;background:#2a2a2a;border-radius:8px;border:1px solid #333}"
           ".alert-title{color:#ffaa00;font-size:16px;margin-bottom:10px;font-weight:600}"
           ".alert-item{display:flex;justify-content:space-between;align-items:center;padding:5px 0;border-bottom:1px solid #333}"
           ".alert-item:last-child{border-bottom:none}"
           ".alert-sent{color:#00ff88;font-weight:bold}"
           ".alert-ready{color:#888}"
           ".controls{display:flex;gap:10px;margin:20px 0;justify-content:center}"
           ".btn{padding:12px 20px;border:none;border-radius:8px;font-size:14px;font-weight:600;cursor:pointer;text-decoration:none;display:inline-block;text-align:center;transition:all 0.3s;min-width:100px}"
           ".btn-primary{background:linear-gradient(135deg,#00ff88,#00cc6a);color:#000}"
           ".btn-primary:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(0,255,136,0.3)}"
           ".btn-danger{background:linear-gradient(135deg,#ff4444,#cc0000);color:#fff}"
           ".btn-danger:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(255,68,68,0.3)}"
           ".btn-secondary{background:linear-gradient(135deg,#666,#444);color:#fff}"
           ".btn-secondary:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(102,102,102,0.3)}"
           ".footer{margin-top:20px;padding-top:15px;border-top:1px solid #333;text-align:center}"
           ".footer-text{font-size:12px;color:#666;margin:3px 0}");
}

void setup() {
  Serial.begin(115200);

  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo ThingSpeak
  ThingSpeak.begin(thingSpeakClient);
  
  server.begin();
  
  Serial.println("Gas Monitoring System Started");
  Serial.println("Features: Web Interface + ThingSpeak Logging + WhatsApp Alerts");
}

void loop() {
  // Xử lý kết nối WiFi nếu bị mất
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      delay(5000);
      Serial.print(".");
    }
    Serial.println("\nReconnected to WiFi");
  }

  // Xử lý web server
  WiFiClient client = server.available();
  
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/stop") != -1) isRunning = !isRunning;
    if (request.indexOf("/reset") != -1) {
      ppm = 0;
      // Reset các cảnh báo khi reset
      alertSent800 = false;
      alertSent1100 = false;
      alertSent1300 = false;
    }

    if (isRunning) ppm = random(0, 1601);

    // Kiểm tra và gửi cảnh báo WhatsApp
    checkAlerts(ppm);

    // Điều khiển LED
    digitalWrite(ledGreen, ppm < 300);
    digitalWrite(ledYellow, ppm >= 300 && ppm < 900);

    // Tính toán tần số nhấp nháy LED đỏ nếu ppm >= 900
    int blinkFrequency = 0;
    if (ppm >= 900) {
      blinkFrequency = map(ppm, 900, 1600, 1, 10);
      blinkFrequency = constrain(blinkFrequency, 1, 10);

      unsigned long blinkInterval = 500 / blinkFrequency;
      if (millis() - lastBlinkTime >= blinkInterval) {
        ledRedState = !ledRedState;
        digitalWrite(ledRed, ledRedState);
        lastBlinkTime = millis();
      }
    } else {
      digitalWrite(ledRed, LOW);
    }

    // Điều khiển buzzer nếu ppm > 1200
    bool buzzerOn = false;
    if (ppm > 1200) {
      tone(buzzer, 1000);
      buzzerOn = true;
    } else {
      noTone(buzzer);
    }

    // Xác định trạng thái và class CSS
    String statusClass = "safe";
    String statusText = "SAFE";
    String ppmClass = "safe";
    
    if (ppm > 1300) {
      statusClass = "emergency";
      statusText = "EMERGENCY!";
      ppmClass = "emergency";
    } else if (ppm > 1100) {
      statusClass = "danger";
      statusText = "HIGH DANGER";
      ppmClass = "danger";
    } else if (ppm > 800) {
      statusClass = "danger";
      statusText = "DANGER";
      ppmClass = "danger";
    } else if (ppm >= 300) {
      statusClass = "caution";
      statusText = "CAUTION";
      ppmClass = "caution";
    }

    // Tạo giao diện HTML tối ưu
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-type:text/html"));
    client.println();

    client.println(F("<!DOCTYPE html><html><head>"));
    client.println(F("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"));
    client.println(F("<title>Gas Monitor Pro</title>"));
    client.println(F("<style>"));
    client.println(getCSS());
    client.println(F("</style></head><body>"));
    
    client.println(F("<div class=\"container\">"));
    client.println(F("<h1>GAS MONITORING SYSTEMS</h1>"));
    client.println(F("<h3>Real-time - ThingSpeak - WhatsApp</h3>"));

    // Hiển thị PPM với animation
    client.println("<div class=\"ppm-display " + ppmClass + "\">");
    client.println("<div class=\"ppm-value\">" + String(ppm) + " PPM</div>");
    client.println("<div class=\"ppm-status\">" + statusText + "</div>");
    client.println(F("</div>"));

    // Status Grid
    client.println(F("<div class=\"status-grid\">"));
    
    client.println(F("<div class=\"status-item\">"));
    client.println(F("<div class=\"status-label\">GREEN LED</div>"));
    client.println("<div class=\"status-value " + String(ppm < 300 ? "led-on" : "") + "\">" + String(ppm < 300 ? "ON" : "OFF") + "</div>");
    client.println(F("</div>"));
    
    client.println(F("<div class=\"status-item\">"));
    client.println(F("<div class=\"status-label\">YELLOW LED</div>"));
    client.println("<div class=\"status-value " + String((ppm >= 300 && ppm < 900) ? "led-on" : "") + "\">" + String((ppm >= 300 && ppm < 900) ? "ON" : "OFF") + "</div>");
    client.println(F("</div>"));
    
    client.println(F("<div class=\"status-item\">"));
    client.println(F("<div class=\"status-label\">RED LED</div>"));
    client.println("<div class=\"status-value " + String(ppm >= 900 ? "led-blink" : "") + "\">" + String(ppm >= 900 ? "BLINK " + String(blinkFrequency) + "Hz" : "OFF") + "</div>");
    client.println(F("</div>"));
    
    client.println(F("<div class=\"status-item\">"));
    client.println(F("<div class=\"status-label\">BUZZER</div>"));
    client.println("<div class=\"status-value " + String(buzzerOn ? "buzzer-on" : "") + "\">" + String(buzzerOn ? "ACTIVE" : "OFF") + "</div>");
    client.println(F("</div>"));
    
    client.println(F("</div>"));

    // WhatsApp Alert Status
    client.println(F("<div class=\"alert-section\">"));
    client.println(F("<div class=\"alert-title\">WHATSAPP ALERTS</div>"));
    
    client.println(F("<div class=\"alert-item\">"));
    client.println(F("<span>Level 800+ PPM</span>"));
    client.println("<span class=\"" + String(alertSent800 ? "alert-sent\">SENT" : "alert-ready\">READY") + "</span>");
    client.println(F("</div>"));
    
    client.println(F("<div class=\"alert-item\">"));
    client.println(F("<span>Level 1100+ PPM</span>"));
    client.println("<span class=\"" + String(alertSent1100 ? "alert-sent\">SENT" : "alert-ready\">READY") + "</span>");
    client.println(F("</div>"));
    
    client.println(F("<div class=\"alert-item\">"));
    client.println(F("<span>Level 1300+ PPM</span>"));
    client.println("<span class=\"" + String(alertSent1300 ? "alert-sent\">SENT" : "alert-ready\">READY") + "</span>");
    client.println(F("</div>"));
    
    client.println(F("</div>"));

    // Control Buttons
    client.println(F("<div class=\"controls\">"));
    if (isRunning) {
      client.println(F("<a href=\"/stop\" class=\"btn btn-danger\">STOP</a>"));
    } else {
      client.println(F("<a href=\"/stop\" class=\"btn btn-primary\">START</a>"));
    }
    client.println(F("<a href=\"/reset\" class=\"btn btn-secondary\">RESET</a>"));
    client.println(F("</div>"));

    // Footer
    client.println(F("<div class=\"footer\">"));
    client.println(F("<div class=\"footer-text\">ThingSpeak sync: 30s</div>"));
    client.println("<div class=\"footer-text\">IP: " + WiFi.localIP().toString() + "</div>");
    client.println(F("</div>"));

    client.println(F("</div></body></html>"));

    delay(1);
    client.stop();
  }

  // Gửi dữ liệu lên ThingSpeak mỗi 30 giây
  if (millis() - lastThingSpeakUpdate > thingSpeakDelay && isRunning) {
    sendToThingSpeak(ppm);
    lastThingSpeakUpdate = millis();
  }

  delay(100);
}