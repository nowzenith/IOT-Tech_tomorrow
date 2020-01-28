#include <time.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <IOXhop_FirebaseESP32.h>

// Set these to run example.
#define FIREBASE_HOST "Web Firebase"
#define FIREBASE_AUTH "Firebase Auth"
int n = 0;

String readString;
const char* ssid = "ชื่อ WiFi";
const char* password = "รหัส WiFi";
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
String GG_SHEET_ID = "https://script.google.com/macros/s/เอาตรงนี้มาใส่/exec?";
int timezone = 7 * 3600; //ตั้งค่า TimeZone ตามเวลาประเทศไทย
int dst = 0; //กำหนดค่า Date Swing Time

String NowString() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String tmpNow = "";
  tmpNow += String(p_tm->tm_year+1900);
  tmpNow += "-";
  tmpNow += String(p_tm->tm_mon+1);
  tmpNow += "-";
  tmpNow += String(p_tm->tm_mday);
  tmpNow += " ";
  tmpNow += String(p_tm->tm_hour);
  tmpNow += ":";
  tmpNow += String(p_tm->tm_min);
  tmpNow += ":";
  tmpNow += String(p_tm->tm_sec);
  return tmpNow;
}
void setup() {
  Serial.begin(115200);
  // connect to wifi.
  WiFi.begin(ssid, password);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov"); //ดึงเวลาจาก Server
}
void loop() {
  float t = (float)random(2000, 4000) / 100; // อ่านค่า อุณหภูมิ (สมมติใช้ค่า random แทน)
  float h = (float)random(1000, 9000) / 100; // อ่านค่า ความชื้น (สมมติใช้ค่า random แทน)
  float l = (float)random(0, 5000); // อ่านค่า แสง (สมมติใช้ค่า random แทน)
 //-----------------------------------------------------------------------------------------------ส่งเข้าGG------------------------------------------------------------------------------------
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String temp =  String(t);
  String hump =  String(h);
  String light =  String(l);
  String url = "/macros/s/" + GG_SHEET_ID + "/exec?Temp=" + temp + "&Hump=" + hump + "&Light=" + light;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp32/Arduino CI successfull!");
  } else {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.println("Response...");
    while (!client.available()) {
      delay(100);
      Serial.print(".");
    }
    Serial.println();
    Serial.println("-----------");
    while (client.available()) {
      Serial.write(client.read());
    }
    Serial.println();
    Serial.println("-----------");
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  client.stop();
//-----------------------------------------------------------------------------------------------ส่งแล้ววว------------------------------------------------------------------------------------
  Serial.print(NowString());
  Serial.println(", Firebase connected");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["time"] = NowString();
  root["temperature"] = t;
  root["humidity"] = h;
  root["ldr"] = l;

  String name = Firebase.push("Sensor", root);
  if (Firebase.failed()) {
    Serial.print(NowString());
    Serial.print(", Firebase pushing /Sensor failed:");
    Serial.println(Firebase.error());
    return;
  }
  Serial.print(NowString());
  Serial.print(", Firebase pushed: /Sensor/");
  Serial.println(name);
}
