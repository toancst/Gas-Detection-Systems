#include <WiFi.h>
#include <HTTPClient.h>

// Thông tin WiFi
const char* ssid = "P519";
const char* password = "deocomatkhau";

// Thông tin API CallMeBot
const String phoneNumber = "84972334069";
const String apiKey = "4495495";

void sendMessage(String message){
  // Mã hóa URL cho tin nhắn
  String encodedMessage = "hello%20test%20test"; // Thay thế khoảng trắng bằng "%20"

  // Tạo URL đúng cách
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber +
               "&text=" + encodedMessage + "&apikey=" + apiKey;

  HTTPClient http;
  http.begin(url);

  // Đặt tiêu đề HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Gửi yêu cầu HTTP POST
  int httpResponseCode = http.POST("");

  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Giải phóng tài nguyên
  http.end();
}

void setup() {
  Serial.begin(115200);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Gửi tin nhắn WhatsApp
  sendMessage("Hello from ESP32!");
}

void loop() {
  // Không cần thực hiện gì trong loop()
}
