#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SGP30.h> // 👇 新增 SGP30 对象
#include <time.h>

#include "webpage.h"

// --- 硬件与引脚定义 ---
#define RGB_PIN        48
#define NUMPIXELS      1
#define SDA_PIN        4
#define SCL_PIN        5
#define SOIL_PIN       6

#define PUMP_PIN       8
#define LIGHT_PIN      9
#define HEATER_PIN     10
#define FAN_PIN        11

#define RELAY_ON       LOW
#define RELAY_OFF      HIGH

#define SHT40_ADDR     0x44 
#define SOIL_AIR_VAL   4095
#define SOIL_WATER_VAL 1200

// --- 全局对象 ---
Adafruit_NeoPixel pixel(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);
WebSocketsServer webSocket(81);
Preferences preferences;
BH1750 lightMeter;
Adafruit_SGP30 sgp; // 👇 新增 SGP30 对象

// --- 环境数据变量 ---
float temp = 0.0, hum = 0.0, lux = 0.0;
int soilPercent = 0;
uint16_t eco2 = 400, tvoc = 0;

// --- 传感器在线状态 ---
bool sensor_bh1750_ok = false;
bool sensor_sht40_ok = false;
bool sensor_sgp30_ok = false;
bool has_psram = false;

// --- 硬件控制状态变量 ---
bool autoMode = true;
bool statePump = false;
bool stateLight = false;
bool stateHeater = false;
bool stateFan = false;

// --- 任务调度结构体 ---
struct Schedule {
  bool fan_en = false;
  String fan_start = "00:00";
  String fan_end = "00:00";
  bool light_en = false;
  String light_start = "00:00";
  String light_end = "00:00";
} sched;

// --- 历史数据记录 ---
#define HISTORY_SIZE 60   
float h_temp[HISTORY_SIZE];
float h_hum[HISTORY_SIZE];
float h_lux[HISTORY_SIZE];
int h_soil[HISTORY_SIZE];
uint16_t h_eco2[HISTORY_SIZE]; // 👇 新增 eCO2 历史
uint16_t h_tvoc[HISTORY_SIZE]; // 👇 新增 TVOC 历史
String h_time[HISTORY_SIZE];
int history_head = 0;     
int history_count = 0;    

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void readSHT40() {
  Wire.beginTransmission(SHT40_ADDR);
  Wire.write(0xFD);
  if (Wire.endTransmission() == 0) {
    delay(10);
    Wire.requestFrom(SHT40_ADDR, 6);
    if (Wire.available() == 6) {
      uint16_t t_ticks = (Wire.read() << 8) | Wire.read(); Wire.read();
      uint16_t h_ticks = (Wire.read() << 8) | Wire.read(); Wire.read();
      temp = -45.0 + (175.0 * t_ticks / 65535.0);
      hum = -6.0 + (125.0 * h_ticks / 65535.0);
      sensor_sht40_ok = true;
      return;
    }
  }
  sensor_sht40_ok = false;
}

void updateRelays() {
  digitalWrite(PUMP_PIN,   statePump   ? RELAY_ON : RELAY_OFF);
  digitalWrite(LIGHT_PIN,  stateLight  ? RELAY_ON : RELAY_OFF);
  digitalWrite(HEATER_PIN, stateHeater ? RELAY_ON : RELAY_OFF);
  digitalWrite(FAN_PIN,    stateFan    ? RELAY_ON : RELAY_OFF);
}

// --- 调度检查函数 ---
bool isTimeInRange(String current, String start, String end) {
  if (start == end) return false;
  if (start < end) return (current >= start && current < end);
  return (current >= start || current < end); // 跨零点处理
}

void handleAutoLogic() {
  if (!autoMode) return;

  struct tm timeinfo;
  String curTime = "";
  if (getLocalTime(&timeinfo)) {
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
    curTime = String(buf);
  }

  // 1. 传感器阈值逻辑
  if (soilPercent < 30) statePump = true;
  else if (soilPercent > 60) statePump = false;

  // 2. 定时调度逻辑 (会覆盖传感器逻辑，或者与传感器逻辑取 OR/AND)
  // 补光灯：(光照不足 OR 定时开启)
  bool lightBySensor = (lux < 200);
  bool lightBySched = (sched.light_en && isTimeInRange(curTime, sched.light_start, sched.light_end));
  stateLight = lightBySensor || lightBySched;
  if (lux > 800 && !lightBySched) stateLight = false;

  if (temp < 18.0) stateHeater = true;
  else if (temp > 22.0) stateHeater = false;

  // 风扇：(高温 OR 高CO2 OR 定时通风)
  bool fanBySensor = (temp > 28.0 || eco2 > 1000);
  bool fanBySched = (sched.fan_en && isTimeInRange(curTime, sched.fan_start, sched.fan_end));
  stateFan = fanBySensor || fanBySched;
  if (temp < 25.0 && eco2 < 800 && !fanBySched) stateFan = false;

  updateRelays();
}

void recordHistory() {
  struct tm timeinfo;
  String tStr = "";
  if (getLocalTime(&timeinfo, 10)) {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
    tStr = String(timeStringBuff);
  } else {
    tStr = String(millis() / 1000) + "s"; 
  }

  int idx = (history_head + history_count) % HISTORY_SIZE;
  if (history_count < HISTORY_SIZE) {
    h_temp[history_count] = temp; h_hum[history_count] = hum;
    h_lux[history_count] = lux; h_soil[history_count] = soilPercent;
    h_eco2[history_count] = eco2; h_tvoc[history_count] = tvoc; // 👇 记录气象
    h_time[history_count] = tStr;
    history_count++;
  } else {
    h_temp[history_head] = temp; h_hum[history_head] = hum;
    h_lux[history_head] = lux; h_soil[history_head] = soilPercent;
    h_eco2[history_head] = eco2; h_tvoc[history_head] = tvoc;
    h_time[history_head] = tStr;
    history_head = (history_head + 1) % HISTORY_SIZE;
  }
}

String buildJson() {
  DynamicJsonDocument doc(16384); 

  JsonObject current = doc.createNestedObject("current");
  current["temp"] = temp; current["hum"] = hum;
  current["lux"] = lux; current["soil"] = soilPercent;
  current["eco2"] = eco2; current["tvoc"] = tvoc; // 👇 下发气体数据

  JsonObject state = doc.createNestedObject("state");
  state["mode"]   = autoMode ? "auto" : "manual";
  state["pump"]   = statePump ? 1 : 0;
  state["light"]  = stateLight ? 1 : 0;
  state["heater"] = stateHeater ? 1 : 0;
  state["fan"]    = stateFan ? 1 : 0;

  // --- 发送当前系统时间与调度配置 ---
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    doc["time"] = String(buf);
  }
  JsonObject s_obj = doc.createNestedObject("sched");
  s_obj["fan_en"] = sched.fan_en;
  s_obj["fan_start"] = sched.fan_start;
  s_obj["fan_end"] = sched.fan_end;
  s_obj["light_en"] = sched.light_en;
  s_obj["light_start"] = sched.light_start;
  s_obj["light_end"] = sched.light_end;

  JsonObject system = doc.createNestedObject("system");
  system["heap_total"] = has_psram ? ESP.getPsramSize() : ESP.getHeapSize();
  system["heap_free"] = has_psram ? ESP.getFreePsram() : ESP.getFreeHeap();
  system["chip_rev"] = ESP.getChipRevision();
  system["cpu_cores"] = ESP.getChipCores();
  system["mac"] = WiFi.macAddress();
  system["sdk_ver"] = ESP.getSdkVersion();
  system["rssi"]      = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  system["ip"]        = WiFi.localIP().toString();
  system["cpu_freq"]  = ESP.getCpuFreqMHz();
  system["uptime"]    = (uint32_t)(millis() / 1000);
  system["psram"]     = has_psram;

  // 传感器在线状态
  JsonObject sensors_status = doc.createNestedObject("sensors");
  sensors_status["bh1750"] = sensor_bh1750_ok;
  sensors_status["sht40"]  = sensor_sht40_ok;
  sensors_status["sgp30"]  = sensor_sgp30_ok;
  sensors_status["soil"]   = true;  // ADC 总是可用

  JsonObject history = doc.createNestedObject("history");
  JsonArray t_arr = history.createNestedArray("time");
  JsonArray temp_arr = history.createNestedArray("temp");
  JsonArray hum_arr = history.createNestedArray("hum");
  JsonArray lux_arr = history.createNestedArray("lux");
  JsonArray soil_arr = history.createNestedArray("soil");
  JsonArray eco2_arr = history.createNestedArray("eco2"); // 👇 气体历史
  JsonArray tvoc_arr = history.createNestedArray("tvoc"); // 👇 气体历史

  for (int i = 0; i < history_count; i++) {
    int idx = (history_head + i) % HISTORY_SIZE; 
    t_arr.add(h_time[idx]);
    temp_arr.add(h_temp[idx]);
    hum_arr.add(h_hum[idx]);
    lux_arr.add(h_lux[idx]);
    soil_arr.add(h_soil[idx]);
    eco2_arr.add(h_eco2[idx]);
    tvoc_arr.add(h_tvoc[idx]);
  }

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String buildStateJson() {
  DynamicJsonDocument doc(512);
  JsonObject state = doc.createNestedObject("state");
  state["mode"]   = autoMode ? "auto" : "manual";
  state["pump"]   = statePump ? 1 : 0;
  state["light"]  = stateLight ? 1 : 0;
  state["heater"] = stateHeater ? 1 : 0;
  state["fan"]    = stateFan ? 1 : 0;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    String jsonStr = buildJson();
    webSocket.sendTXT(num, jsonStr);
  } 
  else if (type == WStype_TEXT) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (!error && doc.containsKey("action")) {
      String action = doc["action"];

      if (action == "set_mode") {
        autoMode = (doc["mode"] == "auto");
        handleAutoLogic();
      } 
      else if (action == "set_device" && !autoMode) {
        String device = doc["device"];
        bool isOn = (doc["state"] == 1);
        if (device == "pump") statePump = isOn;
        else if (device == "light") stateLight = isOn;
        else if (device == "heater") stateHeater = isOn;
        else if (device == "fan") stateFan = isOn;
        updateRelays();
      }
      else if (action == "set_sched") {
        sched.fan_en = doc["fan_en"];
        sched.fan_start = doc["fan_start"].as<String>();
        sched.fan_end = doc["fan_end"].as<String>();
        sched.light_en = doc["light_en"];
        sched.light_start = doc["light_start"].as<String>();
        sched.light_end = doc["light_end"].as<String>();
        
        // 保存到 Preferences 持久化
        preferences.begin("sched", false);
        preferences.putBool("f_en", sched.fan_en);
        preferences.putString("f_s", sched.fan_start);
        preferences.putString("f_e", sched.fan_end);
        preferences.putBool("l_en", sched.light_en);
        preferences.putString("l_s", sched.light_start);
        preferences.putString("l_e", sched.light_end);
        preferences.end();

        handleAutoLogic();
      }
      
      String stateStr = buildStateJson();
      webSocket.broadcastTXT(stateStr);
    }
  }
}

void handleRootSTA() { server.send_P(200, "text/html", index_html); }

void handleRootAP() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>WiFi设置</title><style>body{font-family:sans-serif;text-align:center;padding:20px;}";
  html += "input,select{width:80%;padding:10px;margin:10px 0;border-radius:5px;border:1px solid #ccc;}";
  html += "button{width:85%;padding:12px;background:#4CAF50;color:white;border:none;border-radius:5px;font-size:16px;}</style></head><body>";
  html += "<h1>🌱 大棚WiFi配置</h1><p>请选择WiFi并输入密码</p><form action='/save' method='POST'><select name='ssid'>";
  
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) { html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + "dBm)</option>"; }
  
  html += "</select><br><input type='password' name='pass' placeholder='WiFi密码'><br>";
  html += "<button type='submit'>保存并重启</button></form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  preferences.begin("wifi-config", false);
  preferences.putString("ssid", server.arg("ssid"));
  preferences.putString("password", server.arg("pass"));
  preferences.end();
  server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>配置已保存！设备正在重启...</h2></body></html>");
  delay(2000); ESP.restart();
}

void handleReset() {
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>WiFi配置已清除！重启中...</h2></body></html>");
  delay(2000); ESP.restart();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== AI-Biodome 启动 ===");

  pinMode(PUMP_PIN, OUTPUT);   digitalWrite(PUMP_PIN, RELAY_OFF);
  pinMode(LIGHT_PIN, OUTPUT);  digitalWrite(LIGHT_PIN, RELAY_OFF);
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, RELAY_OFF);
  pinMode(FAN_PIN, OUTPUT);    digitalWrite(FAN_PIN, RELAY_OFF);

  pixel.begin();
  pixel.setBrightness(50);
  setLED(0, 0, 0);

  // 检查 PSRAM
  has_psram = psramFound();
  if (has_psram) {
    Serial.println("✅ PSRAM 可用");
  } else {
    Serial.println("⚠️ PSRAM 不可用，使用内部 SRAM");
  }

  // 初始化总线和传感器（全部带保护）
  Wire.begin(SDA_PIN, SCL_PIN);

  // BH1750 光照传感器
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    sensor_bh1750_ok = true;
    Serial.println("✅ BH1750 初始化成功");
  } else {
    sensor_bh1750_ok = false;
    Serial.println("⚠️ BH1750 未连接，光照数据将不可用");
  }

  pinMode(SOIL_PIN, INPUT);

  // SGP30 空气质量传感器
  if (sgp.begin()) {
    sensor_sgp30_ok = true;
    Serial.println("✅ SGP30 传感器初始化成功");
  } else {
    sensor_sgp30_ok = false;
    Serial.println("⚠️ SGP30 未连接，空气质量数据将不可用");
  }

  // 加载调度配置
  preferences.begin("sched", true);
  sched.fan_en = preferences.getBool("f_en", false);
  sched.fan_start = preferences.getString("f_s", "00:00");
  sched.fan_end = preferences.getString("f_e", "00:00");
  sched.light_en = preferences.getBool("l_en", false);
  sched.light_start = preferences.getString("l_s", "00:00");
  sched.light_end = preferences.getString("l_e", "00:00");
  preferences.end();

  preferences.begin("wifi-config", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("password", "");
  preferences.end();

  if (ssid != "") {
    Serial.print("尝试连接 WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 30) {
      setLED(0, 0, 255); delay(250); setLED(0, 0, 0); delay(250); 
      retry++;
      if (retry % 5 == 0) Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    setLED(0, 255, 0);
    Serial.println("WiFi 连接成功！");
    Serial.println(WiFi.localIP());
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    server.on("/", handleRootSTA);
    server.on("/reset", handleReset);
  } else {
    setLED(255, 100, 0);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("AI-Biodome-Config");
    server.on("/", handleRootAP);
    server.on("/save", handleSave);
  }
  
  server.begin();
}

void loop() {
  server.handleClient();
  
  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop(); 

    // SGP30 要求每 1 秒读取一次以保持内部校准算法稳定
    static unsigned long lastSGP = 0;
    if (sensor_sgp30_ok && millis() - lastSGP > 1000) {
      lastSGP = millis();
      if (sgp.IAQmeasure()) {
        eco2 = sgp.eCO2;
        tvoc = sgp.TVOC;
      }
    }

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 5000) {
      lastUpdate = millis();
      
      readSHT40();
      lux = sensor_bh1750_ok ? lightMeter.readLightLevel() : 0.0;
      int rawSoil = analogRead(SOIL_PIN);
      soilPercent = constrain(map(rawSoil, SOIL_AIR_VAL, SOIL_WATER_VAL, 0, 100), 0, 100);

      handleAutoLogic();
      recordHistory();
      
      String broadcastStr = buildJson();
      webSocket.broadcastTXT(broadcastStr);

      // LED 状态指示
      bool allOffline = !sensor_bh1750_ok && !sensor_sht40_ok && !sensor_sgp30_ok;
      if (allOffline) {
        setLED(255, 180, 0);  // 黄色 = 空板/无传感器
      } else if (soilPercent < 20 || temp > 35 || eco2 > 1500) {
        setLED(255, 0, 0);    // 红色 = 告警
      } else {
        setLED(0, 255, 0);    // 绿色 = 正常
      } 
    }
  }
}