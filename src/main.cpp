#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SGP30.h>
#include <time.h>
#include <math.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <vector>
#include <algorithm>
#include <WebSocketsClient.h>

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

// --- SGP30 基准线管理 ---
unsigned long lastBaselineSave = 0;
#define BASELINE_SAVE_INTERVAL 3600000  // 每1小时保存一次基准线

// --- LittleFS 持久化存储 ---
bool littlefs_ok = false;
#define DEFAULT_SAVE_INTERVAL 300  // 默认每300秒(5分钟)归档一次
#define MAX_DATA_AGE_DAYS 30       // 保留最30天的数据
unsigned long lastDataSave = 0;
unsigned int dataSaveInterval = DEFAULT_SAVE_INTERVAL;  // 秒，用户可配置

// --- AI 大模型配置 (MiniMax) ---
String aiApiKey = "";  // 用户在网页配置

// --- QQ Bot 配置 ---
struct QQBotConfig {
  String appId;
  String appSecret;
  String userOpenId;  // 用户私聊 openid
  bool enabled = false;
} qqbot;

String qqbotAccessToken = "";
unsigned long qqbotTokenExpiry = 0;  // Token 有效时长 ms
unsigned long qqbotTokenObtainedAt = 0;  // Token 获取时刻
unsigned long lastQQBotAlert = 0;
#define QQBOT_ALERT_INTERVAL 1800000  // 告警消息最短间隔30分钟

// --- QQ Bot WebSocket Gateway (接收用户消息) ---
WebSocketsClient qqGateway;
bool qqGwConnected = false;
bool qqGwIdentified = false;
unsigned long qqGwHeartbeatInterval = 41250;
unsigned long qqGwLastHeartbeat = 0;
int qqGwLastSeq = -1;
String qqGwSessionId = "";
unsigned long qqGwReconnectAt = 0;
#define QQ_GW_RECONNECT_DELAY 10000

// --- AI 异步请求 ---
struct AIRequest {
  uint8_t clientNum;
  String systemPrompt;
  String userMessage;
  bool pending = false;
} aiRequest;
SemaphoreHandle_t aiMutex = NULL;

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

// ===================== LED 指示灯状态机 =====================

enum LedMode {
  LED_OFF, LED_SOLID, LED_BLINK_FAST, LED_BLINK_SLOW, LED_BREATHE, LED_FLASH_EVENT
};

struct LedState {
  LedMode mode = LED_OFF;
  uint8_t r = 0, g = 0, b = 0;
  // 事件闪烁（临时覆盖）
  uint8_t evR = 0, evG = 0, evB = 0;
  unsigned long evEnd = 0;    // 事件闪烁结束时间
  // QQ Gateway 蓝闪
  unsigned long lastGwFlash = 0;
} led;

// 触发一次短暂事件闪烁（持续 duration_ms 后自动恢复）
void ledFlashEvent(uint8_t r, uint8_t g, uint8_t b, unsigned long duration_ms = 3000) {
  led.evR = r; led.evG = g; led.evB = b;
  led.evEnd = millis() + duration_ms;
}

// 根据当前系统状态更新 LED
void updateLedState() {
  // 计算告警数量
  int criticalCount = 0;
  int warnCount = 0;

  if (sensor_sht40_ok) {
    if (temp > 35 || temp < 10) criticalCount++;
    else if (temp > 28 || temp < 15) warnCount++;
  }
  if (soilPercent < 20) criticalCount++;
  else if (soilPercent < 30 || soilPercent > 80) warnCount++;
  if (sensor_sgp30_ok) {
    if (eco2 > 1500) criticalCount++;
    else if (eco2 > 1000) warnCount++;
    if (tvoc > 660) criticalCount++;
    else if (tvoc > 220) warnCount++;
  }
  if (sensor_sht40_ok && hum > 90) criticalCount++;
  else if (sensor_sht40_ok && (hum > 80 || hum < 20)) warnCount++;

  bool allOffline = !sensor_bh1750_ok && !sensor_sht40_ok && !sensor_sgp30_ok;

  // 按优先级设置 LED 模式
  if (criticalCount >= 2) {
    led.mode = LED_BLINK_FAST; led.r = 255; led.g = 0; led.b = 0;
  } else if (criticalCount == 1) {
    led.mode = LED_BLINK_SLOW; led.r = 255; led.g = 0; led.b = 0;
  } else if (warnCount > 0) {
    led.mode = LED_BREATHE; led.r = 255; led.g = 100; led.b = 0;
  } else if (allOffline) {
    led.mode = LED_SOLID; led.r = 255; led.g = 180; led.b = 0;
  } else {
    led.mode = LED_SOLID; led.r = 0; led.g = 255; led.b = 0;
  }
}

// 渲染 LED 帧（在 loop 中高频调用）
void renderLed() {
  unsigned long now = millis();

  // 事件闪烁优先（QQ消息收到、数据归档等）
  if (led.evEnd > 0 && now < led.evEnd) {
    // 事件闪烁：快速开关交替
    bool on = ((now / 300) % 2 == 0);
    setLED(on ? led.evR : 0, on ? led.evG : 0, on ? led.evB : 0);
    return;
  }
  led.evEnd = 0;

  // QQ Gateway 在线时，每 30 秒蓝闪一次
  if (qqGwIdentified && led.mode == LED_SOLID && led.r == 0 && led.g == 255 && led.b == 0) {
    if (now - led.lastGwFlash > 30000) {
      led.lastGwFlash = now;
      setLED(0, 100, 255);  // 青蓝色闪
      return;  // 下一帧恢复
    }
    // 蓝闪持续 200ms
    if (now - led.lastGwFlash < 200) {
      setLED(0, 100, 255);
      return;
    }
  }

  switch (led.mode) {
    case LED_SOLID:
      setLED(led.r, led.g, led.b);
      break;

    case LED_BLINK_FAST: {
      bool on = ((now / 200) % 2 == 0);
      setLED(on ? led.r : 0, on ? led.g : 0, on ? led.b : 0);
      break;
    }

    case LED_BLINK_SLOW: {
      bool on = ((now / 1000) % 2 == 0);
      setLED(on ? led.r : 0, on ? led.g : 0, on ? led.b : 0);
      break;
    }

    case LED_BREATHE: {
      // 正弦呼吸效果 (2秒周期)
      float phase = (float)(now % 2000) / 2000.0f;
      float brightness = (sin(phase * 2.0f * 3.14159f - 1.5708f) + 1.0f) / 2.0f;
      brightness = brightness * 0.85f + 0.15f;  // 最低亮度15%
      setLED((uint8_t)(led.r * brightness),
             (uint8_t)(led.g * brightness),
             (uint8_t)(led.b * brightness));
      break;
    }

    case LED_OFF:
    default:
      setLED(0, 0, 0);
      break;
  }
}

// ===================== LittleFS 持久化存储 =====================

String getDateStr() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[11];
    strftime(buf, sizeof(buf), "%Y%m%d", &timeinfo);
    return String(buf);
  }
  return "";
}

String getTimeStr() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[20];
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    return String(buf);
  }
  return String(millis() / 1000) + "s";
}

String getDateTimeStr() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf);
  }
  return "";
}

void ensureDir(const char* dir) {
  if (!LittleFS.exists(dir)) {
    LittleFS.mkdir(dir);
  }
}

// 校验日期字符串是否为8位纯数字
bool isValidDateStr(const String& d) {
  if (d.length() != 8) return false;
  for (unsigned int i = 0; i < d.length(); i++) {
    if (d[i] < '0' || d[i] > '9') return false;
  }
  return true;
}

// 保存传感器数据到 CSV
void saveDataToFile() {
  if (!littlefs_ok) return;
  String date = getDateStr();
  if (date.isEmpty()) return;

  ensureDir("/data");
  String path = "/data/" + date + ".csv";
  bool isNew = !LittleFS.exists(path);
  File f = LittleFS.open(path, "a");
  if (!f) return;
  if (isNew) {
    f.println("time,temp,hum,lux,soil,eco2,tvoc");
  }
  f.printf("%s,%.1f,%.1f,%.0f,%d,%u,%u\n",
    getTimeStr().c_str(), temp, hum, lux, soilPercent, eco2, tvoc);
  f.close();
}

// 记录操作日志
void logOperation(const String& action, const String& detail) {
  if (!littlefs_ok) return;
  String date = getDateStr();
  if (date.isEmpty()) return;

  ensureDir("/log");
  String path = "/log/" + date + ".log";
  File f = LittleFS.open(path, "a");
  if (!f) return;
  f.printf("[%s] %s: %s\n", getTimeStr().c_str(), action.c_str(), detail.c_str());
  f.close();
}

// 自动清理过期文件
void cleanOldFiles(const char* dir) {
  if (!littlefs_ok) return;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  // 计算截止日期（当前日期 - MAX_DATA_AGE_DAYS）
  time_t now = mktime(&timeinfo);
  time_t cutoff = now - (MAX_DATA_AGE_DAYS * 86400L);
  struct tm cutTm;
  localtime_r(&cutoff, &cutTm);
  char cutStr[9];
  strftime(cutStr, sizeof(cutStr), "%Y%m%d", &cutTm);
  String cutDate = String(cutStr);

  File root = LittleFS.open(dir);
  if (!root || !root.isDirectory()) return;
  File file = root.openNextFile();
  while (file) {
    String fname = String(file.name());
    // 文件名格式: YYYYMMDD.csv 或 YYYYMMDD.log
    if (fname.length() >= 8) {
      String fileDate = fname.substring(0, 8);
      if (fileDate < cutDate) {
        String fullPath = String(dir) + "/" + fname;
        LittleFS.remove(fullPath);
        Serial.printf("🗑️ 已清理过期文件: %s\n", fullPath.c_str());
      }
    }
    file = root.openNextFile();
  }
}

// ===================== QQ Bot 集成 =====================

// 获取 QQ Bot Access Token
bool refreshQQBotToken() {
  if (qqbot.appId.isEmpty() || qqbot.appSecret.isEmpty()) return false;

  WiFiClientSecure client;
  client.setInsecure();  // 跳过证书验证（学术项目简化处理）
  HTTPClient http;

  http.begin(client, "https://bots.qq.com/app/getAppAccessToken");
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument reqDoc(256);
  reqDoc["appId"] = qqbot.appId;
  reqDoc["clientSecret"] = qqbot.appSecret;
  String reqBody;
  serializeJson(reqDoc, reqBody);

  int code = http.POST(reqBody);
  if (code == 200) {
    DynamicJsonDocument resDoc(512);
    deserializeJson(resDoc, http.getString());
    qqbotAccessToken = resDoc["access_token"].as<String>();
    int expiresIn = resDoc["expires_in"] | 7200;
    qqbotTokenExpiry = (expiresIn - 60) * 1000UL;  // 存储有效时长（非绝对时间）
    qqbotTokenObtainedAt = millis();  // 记录获取时刻
    Serial.println("✅ QQ Bot Token 获取成功");
    http.end();
    return true;
  } else {
    Serial.printf("⚠️ QQ Bot Token 获取失败, code=%d\n", code);
    http.end();
    return false;
  }
}

// 发送私聊消息给 QQ 用户（支持被动回复 msg_id）
bool sendQQBotMsg(const String& content, const String& msgId = "", const String& targetOpenId = "") {
  String openId = targetOpenId.isEmpty() ? qqbot.userOpenId : targetOpenId;
  if (openId.isEmpty()) return false;
  if (qqbotAccessToken.isEmpty() || millis() - qqbotTokenObtainedAt > qqbotTokenExpiry) {
    if (!refreshQQBotToken()) return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String url = "https://api.sgroup.qq.com/v2/users/" + openId + "/messages";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "QQBot " + qqbotAccessToken);

  DynamicJsonDocument msgDoc(1024);
  msgDoc["content"] = content;
  msgDoc["msg_type"] = 0;  // 文本消息
  if (!msgId.isEmpty()) {
    msgDoc["msg_id"] = msgId;  // 被动回复需要携带 msg_id
  }
  String msgBody;
  serializeJson(msgDoc, msgBody);

  int code = http.POST(msgBody);
  bool ok = (code == 200 || code == 201);
  if (ok) {
    Serial.println("✅ QQ Bot 消息发送成功");
    logOperation("QQBOT", "发送告警: " + content.substring(0, 50));
  } else {
    Serial.printf("⚠️ QQ Bot 消息发送失败, code=%d, resp=%s\n", code, http.getString().c_str());
  }
  http.end();
  return ok;
}

// 构建环境告警消息
void checkAndSendQQBotAlert() {
  if (!qqbot.enabled || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastQQBotAlert < QQBOT_ALERT_INTERVAL) return;

  String alerts = "";

  if (sensor_sht40_ok) {
    if (temp > 35) alerts += "🔴 温度过高: " + String(temp, 1) + "°C\n";
    else if (temp < 10) alerts += "🔴 温度过低: " + String(temp, 1) + "°C\n";
  }
  if (soilPercent < 20) alerts += "🔴 土壤严重缺水: " + String(soilPercent) + "%\n";
  if (sensor_sgp30_ok && eco2 > 1500) alerts += "🔴 CO2严重超标: " + String(eco2) + "ppm\n";
  if (sensor_sgp30_ok && tvoc > 660) alerts += "🔴 有机挥发物超标: " + String(tvoc) + "ppb\n";

  if (alerts.isEmpty()) return;

  String msg = "🌱 【AI大棚告警】\n" + alerts;
  msg += "\n📊 当前状态: 温度" + String(temp, 1) + "°C 湿度" + String(hum, 1) + "%";
  msg += " 光照" + String(lux, 0) + "lx 土壤" + String(soilPercent) + "%";
  msg += " CO2:" + String(eco2) + "ppm TVOC:" + String(tvoc) + "ppb";

  if (sendQQBotMsg(msg)) {
    lastQQBotAlert = millis();
  }
}

// ===================== QQ Bot Gateway (被动消息接收) =====================

// 构建当前环境状态的文本摘要
String buildStatusText() {
  String s = "🌱 【AI大棚实时状态】\n";
  s += "🌡️ 温度: " + String(temp, 1) + "°C\n";
  s += "💧 湿度: " + String(hum, 1) + "%\n";
  s += "☀️ 光照: " + String(lux, 0) + " lx\n";
  s += "🌱 土壤: " + String(soilPercent) + "%\n";
  s += "☁️ CO2: " + String(eco2) + " ppm\n";
  s += "🧪 TVOC: " + String(tvoc) + " ppb\n";
  s += "\n⚙️ 设备状态:\n";
  s += "  水泵: " + String(statePump ? "开启" : "关闭") + "\n";
  s += "  补光灯: " + String(stateLight ? "开启" : "关闭") + "\n";
  s += "  加热: " + String(stateHeater ? "开启" : "关闭") + "\n";
  s += "  风扇: " + String(stateFan ? "开启" : "关闭") + "\n";
  s += "  模式: " + String(autoMode ? "自动" : "手动") + "\n";
  s += "\n⏱️ 运行: " + String((uint32_t)(millis() / 1000 / 3600)) + "h " +
       String((uint32_t)(millis() / 1000 % 3600 / 60)) + "m";
  return s;
}

// 处理用户发送的命令
void handleQQCommand(const String& content, const String& msgId, const String& userOpenId) {
  String cmd = content;
  cmd.trim();
  cmd.toLowerCase();

  String reply;
  if (cmd == "状态" || cmd == "查询" || cmd == "/status") {
    reply = buildStatusText();
  }
  else if (cmd == "告警" || cmd == "/alert") {
    String alerts = "";
    if (sensor_sht40_ok) {
      if (temp > 35) alerts += "🔴 温度过高: " + String(temp, 1) + "°C\n";
      else if (temp < 10) alerts += "🔴 温度过低: " + String(temp, 1) + "°C\n";
    }
    if (soilPercent < 20) alerts += "🔴 土壤严重缺水: " + String(soilPercent) + "%\n";
    if (sensor_sgp30_ok && eco2 > 1500) alerts += "🔴 CO2严重超标: " + String(eco2) + "ppm\n";
    if (sensor_sgp30_ok && tvoc > 660) alerts += "🔴 有机挥发物超标: " + String(tvoc) + "ppb\n";
    if (alerts.isEmpty()) {
      reply = "✅ 当前环境状态良好，无告警";
    } else {
      reply = "⚠️ 当前告警:\n" + alerts;
    }
  }
  else if (cmd == "帮助" || cmd == "help" || cmd == "/help" || cmd == "?") {
    reply = "🤖 AI大棚机器人指令:\n\n";
    reply += "📊 状态 - 查看当前环境数据\n";
    reply += "⚠️ 告警 - 查看当前告警信息\n";
    reply += "❓ 帮助 - 显示本帮助\n";
    reply += "\n💡 直接发送以上关键词即可";
  }
  else {
    reply = "🤖 你好！发送「帮助」查看可用指令";
  }

  sendQQBotMsg(reply, msgId, userOpenId);
  logOperation("QQBOT_RECV", "收到命令: " + content.substring(0, 30));
}

// 发送 Gateway Identify
void sendQQGatewayIdentify() {
  DynamicJsonDocument doc(512);
  doc["op"] = 2;
  JsonObject d = doc.createNestedObject("d");
  d["token"] = "QQBot " + qqbotAccessToken;
  d["intents"] = (1 << 25);  // GROUP_AND_C2C_EVENT → C2C_MESSAGE_CREATE
  JsonArray shard = d.createNestedArray("shard");
  shard.add(0);
  shard.add(1);
  String payload;
  serializeJson(doc, payload);
  qqGateway.sendTXT(payload);
  Serial.println("📤 QQ Gateway: Identify 已发送");
}

// 发送 Gateway Heartbeat
void sendQQGatewayHeartbeat() {
  DynamicJsonDocument doc(128);
  doc["op"] = 1;
  if (qqGwLastSeq >= 0) {
    doc["d"] = qqGwLastSeq;
  } else {
    doc["d"] = (char*)0;  // JSON null
  }
  String payload;
  serializeJson(doc, payload);
  qqGateway.sendTXT(payload);
  qqGwLastHeartbeat = millis();
}

// Gateway WebSocket 事件回调
void qqGatewayEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("✅ QQ Gateway: WebSocket 已连接");
      qqGwConnected = true;
      break;

    case WStype_DISCONNECTED:
      Serial.println("⚠️ QQ Gateway: WebSocket 断开");
      qqGwConnected = false;
      qqGwIdentified = false;
      qqGwReconnectAt = millis() + QQ_GW_RECONNECT_DELAY;
      break;

    case WStype_TEXT: {
      DynamicJsonDocument doc(4096);
      DeserializationError err = deserializeJson(doc, payload, length);
      if (err) {
        Serial.printf("⚠️ QQ Gateway: JSON 解析失败: %s\n", err.c_str());
        break;
      }

      int op = doc["op"] | -1;

      if (op == 10) {
        // Hello - 获取心跳间隔并发送 Identify
        qqGwHeartbeatInterval = doc["d"]["heartbeat_interval"] | 41250;
        Serial.printf("📬 QQ Gateway: Hello, heartbeat=%lu ms\n", qqGwHeartbeatInterval);

        // 确保 token 有效
        if (qqbotAccessToken.isEmpty() || millis() - qqbotTokenObtainedAt > qqbotTokenExpiry) {
          refreshQQBotToken();
        }
        sendQQGatewayIdentify();
      }
      else if (op == 0) {
        // Dispatch - 事件分发
        if (!doc["s"].isNull()) {
          qqGwLastSeq = doc["s"].as<int>();
        }
        String t = doc["t"].as<String>();

        if (t == "READY") {
          qqGwSessionId = doc["d"]["session_id"].as<String>();
          qqGwIdentified = true;
          Serial.printf("✅ QQ Gateway: READY, session=%s\n", qqGwSessionId.c_str());
          logOperation("QQBOT_GW", "Gateway 连接就绪");
          // 发送首次心跳
          sendQQGatewayHeartbeat();
        }
        else if (t == "C2C_MESSAGE_CREATE") {
          // 收到用户私聊消息
          JsonObject d = doc["d"];
          String msgId = d["id"].as<String>();
          String content = d["content"].as<String>();
          String userOpenId = d["author"]["user_openid"].as<String>();
          content.trim();
          Serial.printf("📩 QQ C2C消息: [%s] %s\n", userOpenId.c_str(), content.c_str());
          // 自动保存首个发消息用户的 OpenID
          if (qqbot.userOpenId.isEmpty() && !userOpenId.isEmpty()) {
            qqbot.userOpenId = userOpenId;
            preferences.begin("qqbot", false);
            preferences.putString("userId", userOpenId);
            preferences.end();
            Serial.printf("📌 已自动保存用户 OpenID: %s\n", userOpenId.c_str());
            logOperation("QQBOT", "自动保存用户OpenID: " + userOpenId);
          }
          ledFlashEvent(180, 0, 255, 3000);  // 紫色闪3秒 = 收到QQ消息
          handleQQCommand(content, msgId, userOpenId);
        }
        else if (t == "RESUMED") {
          qqGwIdentified = true;
          Serial.println("✅ QQ Gateway: RESUMED, 会话已恢复");
          logOperation("QQBOT_GW", "Gateway 会话恢复");
          sendQQGatewayHeartbeat();
        }
        else {
          Serial.printf("📬 QQ Gateway 事件: %s\n", t.c_str());
        }
      }
      else if (op == 11) {
        // Heartbeat ACK
        Serial.println("💓 QQ Gateway: Heartbeat ACK");
      }
      else if (op == 7) {
        // Reconnect - 服务端要求重连
        Serial.println("🔄 QQ Gateway: 服务端要求重连");
        qqGateway.disconnect();
      }
      else if (op == 9) {
        // Invalid Session
        Serial.println("⚠️ QQ Gateway: Invalid Session, 重新连接");
        qqGwIdentified = false;
        qqGwSessionId = "";
        qqGateway.disconnect();
      }
      break;
    }

    case WStype_ERROR:
      Serial.println("❌ QQ Gateway: WebSocket 错误");
      break;

    default:
      break;
  }
}

// 连接 QQ Gateway
void connectQQGateway() {
  if (!qqbot.enabled || qqbot.appId.isEmpty()) return;

  // 先断开已有连接
  if (qqGwConnected) {
    qqGateway.disconnect();
    qqGwConnected = false;
    qqGwIdentified = false;
  }

  // 确保有 access token
  if (qqbotAccessToken.isEmpty() || millis() - qqbotTokenObtainedAt > qqbotTokenExpiry) {
    if (!refreshQQBotToken()) {
      Serial.println("⚠️ QQ Gateway: 无法获取 Token，稍后重试");
      qqGwReconnectAt = millis() + QQ_GW_RECONNECT_DELAY;
      return;
    }
  }

  Serial.println("🔌 QQ Gateway: 正在连接...");
  qqGateway.beginSSL("api.sgroup.qq.com", 443, "/websocket/");
  qqGateway.onEvent(qqGatewayEvent);
  qqGateway.setReconnectInterval(0);  // 手动管理重连
  qqGwReconnectAt = 0;
}

// 断开 QQ Gateway
void disconnectQQGateway() {
  qqGateway.disconnect();
  qqGwConnected = false;
  qqGwIdentified = false;
  qqGwSessionId = "";
  qqGwLastSeq = -1;
  qqGwReconnectAt = 0;
  Serial.println("🔌 QQ Gateway: 已断开");
}

// HTTP API: QQ Bot 配置保存
void handleApiQQBotConfig() {
  if (server.method() == HTTP_POST) {
    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      server.send(400, "application/json", "{\"error\":\"invalid JSON\"}");
      return;
    }
    qqbot.appId = doc["appId"].as<String>();
    if (doc.containsKey("appSecret") && doc["appSecret"].as<String>().length() > 0) {
      qqbot.appSecret = doc["appSecret"].as<String>();
    }
    qqbot.userOpenId = doc["userOpenId"].as<String>();
    qqbot.enabled = doc["enabled"] | false;

    // 保存到 Preferences
    preferences.begin("qqbot", false);
    preferences.putString("appId", qqbot.appId);
    preferences.putString("secret", qqbot.appSecret);
    preferences.putString("userId", qqbot.userOpenId);
    preferences.putBool("enabled", qqbot.enabled);
    preferences.end();

    logOperation("QQBOT", qqbot.enabled ? "已启用" : "已禁用");

    // 如果启用，立即尝试获取 token 并连接 Gateway
    if (qqbot.enabled && !qqbot.appId.isEmpty()) {
      refreshQQBotToken();
      connectQQGateway();
    } else {
      disconnectQQGateway();
    }

    server.send(200, "application/json", "{\"ok\":true}");
  } else {
    // GET - 返回当前配置 (不返回 secret)
    DynamicJsonDocument doc(512);
    doc["appId"] = qqbot.appId;
    doc["userOpenId"] = qqbot.userOpenId;
    doc["enabled"] = qqbot.enabled;
    doc["hasSecret"] = !qqbot.appSecret.isEmpty();
    doc["tokenValid"] = (!qqbotAccessToken.isEmpty() && millis() - qqbotTokenObtainedAt < qqbotTokenExpiry);
    doc["gatewayConnected"] = qqGwConnected;
    doc["gatewayReady"] = qqGwIdentified;
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
  }
}

// HTTP API: 测试 QQ Bot 发送
void handleApiQQBotTest() {
  if (!qqbot.enabled) {
    server.send(400, "application/json", "{\"error\":\"QQ Bot 未启用\"}");
    return;
  }
  String msg = "🌱 【AI大棚测试消息】\n系统运行正常！\n";
  msg += "温度:" + String(temp, 1) + "°C 湿度:" + String(hum, 1) + "%";
  msg += " 光照:" + String(lux, 0) + "lx 土壤:" + String(soilPercent) + "%";
  bool ok = sendQQBotMsg(msg);
  server.send(ok ? 200 : 500, "application/json",
    ok ? "{\"ok\":true}" : "{\"error\":\"发送失败\"}");
}

// --- AI 大模型调用 (MiniMax-M2.7) ---
String callMiniMaxAI(const String& systemPrompt, const String& userMsg) {
  if (aiApiKey.isEmpty()) return "[AI 未配置] 请先在系统设置中输入 MiniMax API Key";

  HTTPClient http;
  http.begin("https://api.minimaxi.com/v1/text/chatcompletion_v2");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + aiApiKey);
  http.setTimeout(30000);

  DynamicJsonDocument reqDoc(2048);
  reqDoc["model"] = "MiniMax-M2.7";
  JsonArray msgs = reqDoc.createNestedArray("messages");

  JsonObject sysMsg = msgs.createNestedObject();
  sysMsg["role"] = "system";
  sysMsg["name"] = "AI\u5927\u68DA\u52A9\u624B";
  sysMsg["content"] = systemPrompt;

  JsonObject usrMsg = msgs.createNestedObject();
  usrMsg["role"] = "user";
  usrMsg["name"] = "\u7528\u6237";
  usrMsg["content"] = userMsg;

  String body;
  serializeJson(reqDoc, body);
  int code = http.POST(body);
  String result = "";

  if (code == 200) {
    String resp = http.getString();
    DynamicJsonDocument respDoc(4096);
    DeserializationError err = deserializeJson(respDoc, resp);
    if (!err && respDoc.containsKey("choices")) {
      result = respDoc["choices"][0]["message"]["content"].as<String>();
    } else {
      result = "[AI 响应解析失败]";
    }
  } else {
    result = "[AI 请求失败] HTTP " + String(code);
    if (code > 0) {
      String errBody = http.getString();
      if (errBody.length() > 0 && errBody.length() < 200) result += ": " + errBody;
    }
  }
  http.end();
  return result;
}

// AI 异步任务（运行在独立 FreeRTOS 核心上，不阻塞主循环）
void aiTaskFunc(void* param) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(100));
    if (xSemaphoreTake(aiMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (aiRequest.pending) {
        uint8_t clientNum = aiRequest.clientNum;
        String sysPrompt = aiRequest.systemPrompt;
        String userMsg = aiRequest.userMessage;
        aiRequest.pending = false;
        xSemaphoreGive(aiMutex);

        String aiReply = callMiniMaxAI(sysPrompt, userMsg);
        DynamicJsonDocument respDoc(4096);
        respDoc["ai_resp"] = aiReply;
        String respStr;
        serializeJson(respDoc, respStr);
        webSocket.sendTXT(clientNum, respStr);
      } else {
        xSemaphoreGive(aiMutex);
      }
    }
  }
}

// HTTP API: AI Key 配置 (GET/POST)
void handleApiAIConfig() {
  if (server.method() == HTTP_POST) {
    DynamicJsonDocument doc(256);
    deserializeJson(doc, server.arg("plain"));
    if (doc.containsKey("apiKey")) {
      String key = doc["apiKey"].as<String>();
      if (!key.isEmpty()) {
        aiApiKey = key;
        preferences.begin("ai", false);
        preferences.putString("apiKey", aiApiKey);
        preferences.end();
        Serial.println("🧠 AI API Key 已更新");
      }
    }
  }
  String resp = "{\"configured\":" + String(aiApiKey.isEmpty() ? "false" : "true") + "}";
  server.send(200, "application/json", resp);
}

// HTTP API: 记录粒度设置 (GET/POST)
void handleApiSaveInterval() {
  if (server.method() == HTTP_POST) {
    DynamicJsonDocument doc(128);
    deserializeJson(doc, server.arg("plain"));
    if (doc.containsKey("interval")) {
      unsigned int val = doc["interval"].as<unsigned int>();
      if (val < 10) val = 10;
      if (val > 3600) val = 3600;
      dataSaveInterval = val;
      preferences.begin("system", false);
      preferences.putUInt("saveIntv", dataSaveInterval);
      preferences.end();
      Serial.printf("📊 记录间隔已更新: %u 秒\n", dataSaveInterval);
      logOperation("SYSTEM", "记录间隔设为 " + String(dataSaveInterval) + " 秒");
    }
  }
  String resp = "{\"interval\":" + String(dataSaveInterval) + "}";
  server.send(200, "application/json", resp);
}

// HTTP API: 导出指定日期范围的 CSV 数据
void handleApiExport() {
  if (!littlefs_ok) {
    server.send(503, "text/plain", "storage unavailable");
    return;
  }
  String startDate = server.arg("start");
  String endDate = server.arg("end");
  if (!isValidDateStr(startDate)) startDate = getDateStr();
  if (!isValidDateStr(endDate)) endDate = startDate;
  if (startDate.isEmpty()) {
    server.send(503, "text/plain", "time not synced");
    return;
  }

  // 先收集日期范围内的文件名并排序
  std::vector<String> fileNames;
  File root = LittleFS.open("/data");
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      String fname = String(file.name());
      if (fname.endsWith(".csv") && fname.length() >= 12) {
        String fdate = fname.substring(0, 8);
        if (fdate >= startDate && fdate <= endDate) {
          fileNames.push_back(fname);
        }
      }
      file = root.openNextFile();
    }
  }
  std::sort(fileNames.begin(), fileNames.end());

  // 按日期顺序发送 CSV
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv", "");
  server.sendContent("date,time,temp,hum,lux,soil,eco2,tvoc\n");

  for (const auto& fname : fileNames) {
    String fdate = fname.substring(0, 8);
    String dateFormatted = fdate.substring(0,4) + "-" + fdate.substring(4,6) + "-" + fdate.substring(6,8);
    File file = LittleFS.open("/data/" + fname, "r");
    if (!file) continue;
    bool firstLine = true;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.isEmpty()) continue;
      if (firstLine) { firstLine = false; continue; }  // 跳过 header
      server.sendContent(dateFormatted + "," + line + "\n");
    }
    file.close();
  }
  server.sendContent("");  // 结束 chunked
}

// HTTP API: 获取指定日期的历史数据
void handleApiHistory() {
  if (!littlefs_ok) {
    server.send(503, "application/json", "{\"error\":\"storage unavailable\"}");
    return;
  }
  String date = server.arg("date");
  if (!isValidDateStr(date)) {
    // 默认返回今天
    date = getDateStr();
    if (date.isEmpty()) {
      server.send(503, "application/json", "{\"error\":\"time not synced\"}");
      return;
    }
  }
  String path = "/data/" + date + ".csv";
  if (!LittleFS.exists(path)) {
    server.send(404, "application/json", "{\"error\":\"no data for this date\"}");
    return;
  }
  File f = LittleFS.open(path, "r");
  if (!f) {
    server.send(500, "application/json", "{\"error\":\"read error\"}");
    return;
  }

  // 根据文件大小动态分配 JSON 缓冲区（约 4 倍 CSV 大小）
  size_t fileSize = f.size();
  size_t docSize = max((size_t)32768, fileSize * 4);
  if (docSize > 262144) docSize = 262144;  // 上限 256KB（PSRAM 安全范围）
  DynamicJsonDocument doc(docSize);
  JsonArray timeArr = doc.createNestedArray("time");
  JsonArray tempArr = doc.createNestedArray("temp");
  JsonArray humArr  = doc.createNestedArray("hum");
  JsonArray luxArr  = doc.createNestedArray("lux");
  JsonArray soilArr = doc.createNestedArray("soil");
  JsonArray eco2Arr = doc.createNestedArray("eco2");
  JsonArray tvocArr = doc.createNestedArray("tvoc");
  doc["date"] = date;

  String line;
  bool header = true;
  while (f.available()) {
    line = f.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;
    if (header) { header = false; continue; }  // 跳过 CSV 表头

    // 解析 CSV: time,temp,hum,lux,soil,eco2,tvoc
    int idx = 0;
    String fields[7];
    int start = 0;
    for (int i = 0; i <= (int)line.length() && idx < 7; i++) {
      if (i == (int)line.length() || line[i] == ',') {
        fields[idx++] = line.substring(start, i);
        start = i + 1;
      }
    }
    if (idx >= 7) {
      timeArr.add(fields[0]);
      tempArr.add(fields[1].toFloat());
      humArr.add(fields[2].toFloat());
      luxArr.add(fields[3].toFloat());
      soilArr.add(fields[4].toInt());
      eco2Arr.add((uint16_t)fields[5].toInt());
      tvocArr.add((uint16_t)fields[6].toInt());
    }
  }
  f.close();

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

// HTTP API: 获取指定日期的操作日志
void handleApiLog() {
  if (!littlefs_ok) {
    server.send(503, "application/json", "{\"error\":\"storage unavailable\"}");
    return;
  }
  String date = server.arg("date");
  if (!isValidDateStr(date)) date = getDateStr();
  if (date.isEmpty()) {
    server.send(503, "application/json", "{\"error\":\"time not synced\"}");
    return;
  }
  String path = "/log/" + date + ".log";
  if (!LittleFS.exists(path)) {
    server.send(404, "application/json", "{\"error\":\"no log for this date\"}");
    return;
  }
  File f = LittleFS.open(path, "r");
  if (!f) {
    server.send(500, "application/json", "{\"error\":\"read error\"}");
    return;
  }

  DynamicJsonDocument doc(16384);
  JsonArray entries = doc.createNestedArray("entries");
  doc["date"] = date;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (!line.isEmpty()) entries.add(line);
  }
  f.close();

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

// HTTP API: 获取可用日期列表
void handleApiDates() {
  if (!littlefs_ok) {
    server.send(503, "application/json", "{\"error\":\"storage unavailable\"}");
    return;
  }
  DynamicJsonDocument doc(4096);
  JsonArray dataArr = doc.createNestedArray("data_dates");
  JsonArray logArr  = doc.createNestedArray("log_dates");

  File dataDir = LittleFS.open("/data");
  if (dataDir && dataDir.isDirectory()) {
    File f = dataDir.openNextFile();
    while (f) {
      String name = String(f.name());
      if (name.endsWith(".csv")) dataArr.add(name.substring(0, name.length() - 4));
      f = dataDir.openNextFile();
    }
  }
  File logDir = LittleFS.open("/log");
  if (logDir && logDir.isDirectory()) {
    File f = logDir.openNextFile();
    while (f) {
      String name = String(f.name());
      if (name.endsWith(".log")) logArr.add(name.substring(0, name.length() - 4));
      f = logDir.openNextFile();
    }
  }

  // 存储状态
  doc["total_bytes"] = LittleFS.totalBytes();
  doc["used_bytes"]  = LittleFS.usedBytes();

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
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
  bool timeValid = false;
  if (getLocalTime(&timeinfo)) {
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
    curTime = String(buf);
    timeValid = true;
  }

  // 1. 传感器阈值逻辑（仅在传感器在线时生效）
  if (soilPercent < 30) statePump = true;
  else if (soilPercent > 60) statePump = false;

  // 2. 补光灯：(光照不足 OR 定时开启)
  bool lightBySensor = sensor_bh1750_ok && (lux < 200);
  bool lightBySched = timeValid && sched.light_en && isTimeInRange(curTime, sched.light_start, sched.light_end);
  stateLight = lightBySensor || lightBySched;
  if (sensor_bh1750_ok && lux > 800 && !lightBySched) stateLight = false;

  // 3. 加热垫（仅在 SHT40 在线时生效）
  if (sensor_sht40_ok) {
    if (temp < 18.0) stateHeater = true;
    else if (temp > 22.0) stateHeater = false;
  }

  // 4. 风扇：(高温 OR 高CO2 OR 定时通风)
  bool fanBySensor = (sensor_sht40_ok && temp > 28.0) || (sensor_sgp30_ok && eco2 > 1000);
  bool fanBySched = timeValid && sched.fan_en && isTimeInRange(curTime, sched.fan_start, sched.fan_end);
  if (fanBySensor || fanBySched) {
    stateFan = true;
  } else if ((sensor_sht40_ok && temp < 25.0) && (sensor_sgp30_ok && eco2 < 800)) {
    stateFan = false;
  }
  // 若传感器都不在线且无定时，保持当前状态

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

  int idx;
  if (history_count < HISTORY_SIZE) {
    idx = history_count;
    history_count++;
  } else {
    idx = history_head;
    history_head = (history_head + 1) % HISTORY_SIZE;
  }
  h_temp[idx] = temp; h_hum[idx] = hum;
  h_lux[idx] = lux; h_soil[idx] = soilPercent;
  h_eco2[idx] = eco2; h_tvoc[idx] = tvoc;
  h_time[idx] = tStr;
}

// 预分配 JSON 缓冲区，减少堆碎片化
static char jsonBuf[16384];

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
    char buf[24];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    doc["time"] = String(buf);
  }
  doc["saveInterval"] = dataSaveInterval;
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
  if (littlefs_ok) {
    system["fs_total"] = LittleFS.totalBytes();
    system["fs_used"]  = LittleFS.usedBytes();
  }

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

  size_t len = serializeJson(doc, jsonBuf, sizeof(jsonBuf));
  return String(jsonBuf);
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
        logOperation("MODE", autoMode ? "切换为自动模式" : "切换为手动模式");
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
        logOperation("CTRL", device + (isOn ? " 开启" : " 关闭"));
      }
      else if (action == "set_sched") {
        sched.fan_en = doc["fan_en"];
        sched.fan_start = doc["fan_start"].as<String>();
        sched.fan_end = doc["fan_end"].as<String>();
        sched.light_en = doc["light_en"];
        sched.light_start = doc["light_start"].as<String>();
        sched.light_end = doc["light_end"].as<String>();
        logOperation("SCHED", "定时任务已更新");
        
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
      else if (action == "ai_analyze") {
        // 环境分析：异步调用 AI（不阻塞主循环）
        if (xSemaphoreTake(aiMutex, 0) == pdTRUE) {
          if (!aiRequest.pending) {
            aiRequest.clientNum = num;
            aiRequest.systemPrompt = "\u4F60\u662F\u667A\u6167\u5927\u68DA\u7684 AI \u52A9\u624B\u3002\u6839\u636E\u4F20\u611F\u5668\u6570\u636E\u5206\u6790\u5F53\u524D\u73AF\u5883\u72B6\u6001\uFF0C\u8BC4\u4F30\u690D\u7269\u751F\u957F\u6761\u4EF6\uFF0C\u7ED9\u51FA\u5177\u4F53\u53EF\u64CD\u4F5C\u7684\u5EFA\u8BAE\u3002\u56DE\u7B54\u7B80\u6D01\uFF0C200\u5B57\u4EE5\u5185\u3002";
            aiRequest.userMessage = "\u5F53\u524D\u73AF\u5883:\n\u6E29\u5EA6:" + String(temp, 1) + "\u00B0C, \u6E7F\u5EA6:" + String(hum, 1) + "%, \u5149\u7167:" + String(lux, 0) + "lx, \u571F\u58E4\u6C34\u5206:" + String(soilPercent) + "%, eCO2:" + String(eco2) + "ppm, TVOC:" + String(tvoc) + "ppb\n\u8BBE\u5907:\u6C34\u6CF5=" + (statePump?"ON":"OFF") + ", \u8865\u5149=" + (stateLight?"ON":"OFF") + ", \u52A0\u70ED=" + (stateHeater?"ON":"OFF") + ", \u98CE\u6247=" + (stateFan?"ON":"OFF") + "\n\u6A21\u5F0F:" + (autoMode?"\u81EA\u52A8":"\u624B\u52A8") + "\n\u8BF7\u5206\u6790\u73AF\u5883\u72B6\u6001\u5E76\u7ED9\u51FA\u5EFA\u8BAE\u3002";
            aiRequest.pending = true;
          } else {
            webSocket.sendTXT(num, "{\"ai_resp\":\"AI \u6B63\u5728\u5904\u7406\u4E0A\u4E00\u4E2A\u8BF7\u6C42\uFF0C\u8BF7\u7A0D\u5019...\"}");
          }
          xSemaphoreGive(aiMutex);
        }
        return;  // AI 请求不需要广播状态
      }
      else if (action == "ai_chat") {
        // 自由对话：异步调用 AI
        String question = doc["question"].as<String>();
        if (!question.isEmpty()) {
          if (xSemaphoreTake(aiMutex, 0) == pdTRUE) {
            if (!aiRequest.pending) {
              aiRequest.clientNum = num;
              aiRequest.systemPrompt = "\u4F60\u662F\u667A\u6167\u5927\u68DA\u7684 AI \u52A9\u624B\u3002\u5F53\u524D\u73AF\u5883:\u6E29\u5EA6" + String(temp, 1) + "\u00B0C,\u6E7F\u5EA6" + String(hum, 1) + "%,\u5149\u7167" + String(lux, 0) + "lx,\u571F\u58E4" + String(soilPercent) + "%,eCO2:" + String(eco2) + "ppm,TVOC:" + String(tvoc) + "ppb\u3002\u56DE\u7B54\u7B80\u6D01\u5B9E\u7528\uFF0C200\u5B57\u4EE5\u5185\u3002";
              aiRequest.userMessage = question;
              aiRequest.pending = true;
            } else {
              webSocket.sendTXT(num, "{\"ai_resp\":\"AI \u6B63\u5728\u5904\u7406\u4E0A\u4E00\u4E2A\u8BF7\u6C42\uFF0C\u8BF7\u7A0D\u5019...\"}");
            }
            xSemaphoreGive(aiMutex);
          }
        }
        return;  // AI 请求不需要广播状态
      }
      
      String stateStr = buildStateJson();
      webSocket.broadcastTXT(stateStr);
    }
  }
}

void handleRootSTA() { server.send_P(200, "text/html", index_html); }

// HTML 实体转义，防止 XSS
String htmlEscape(const String& raw) {
  String out = raw;
  out.replace("&", "&amp;");
  out.replace("<", "&lt;");
  out.replace(">", "&gt;");
  out.replace("'", "&#39;");
  out.replace("\"", "&quot;");
  return out;
}

void handleRootAP() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>WiFi设置</title><style>body{font-family:sans-serif;text-align:center;padding:20px;}";
  html += "input,select{width:80%;padding:10px;margin:10px 0;border-radius:5px;border:1px solid #ccc;}";
  html += "button{width:85%;padding:12px;background:#4CAF50;color:white;border:none;border-radius:5px;font-size:16px;}</style></head><body>";
  html += "<h1>🌱 大棚WiFi配置</h1><p>请选择WiFi并输入密码</p><form action='/save' method='POST'><select name='ssid'>";
  
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String escaped = htmlEscape(WiFi.SSID(i));
    html += "<option value='" + escaped + "'>" + escaped + " (" + String(WiFi.RSSI(i)) + "dBm)</option>";
  }
  
  html += "</select><br><input type='password' name='pass' placeholder='WiFi密码'><br>";
  html += "<button type='submit'>保存并重启</button></form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  // 输入校验
  if (ssid.length() == 0 || ssid.length() > 32) {
    server.send(400, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>SSID 无效（1-32字符）</h2></body></html>");
    return;
  }
  if (pass.length() > 64) {
    server.send(400, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>密码过长（最多64字符）</h2></body></html>");
    return;
  }
  preferences.begin("wifi-config", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", pass);
  preferences.end();
  server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>配置已保存！设备正在重启...</h2></body></html>");
  delay(2000); ESP.restart();
}

void handleReset() {
  // 需要 confirm=yes 参数才执行，防止误触或未授权访问
  if (server.arg("confirm") != "yes") {
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h2>确认重置？</h2><a href='/reset?confirm=yes'>点击确认清除WiFi配置并重启</a></body></html>");
    return;
  }
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

  // 初始化 LittleFS 文件系统
  if (LittleFS.begin(true)) {
    littlefs_ok = true;
    Serial.printf("✅ LittleFS 已挂载 (总计 %u bytes, 已用 %u bytes)\n",
      LittleFS.totalBytes(), LittleFS.usedBytes());
    ensureDir("/data");
    ensureDir("/log");
  } else {
    littlefs_ok = false;
    Serial.println("⚠️ LittleFS 挂载失败，历史数据持久化不可用");
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
    // 恢复上次保存的基准线
    preferences.begin("sgp30", true);
    uint16_t bl_eco2 = preferences.getUShort("eco2", 0);
    uint16_t bl_tvoc = preferences.getUShort("tvoc", 0);
    preferences.end();
    if (bl_eco2 != 0 && bl_tvoc != 0) {
      sgp.setIAQBaseline(bl_eco2, bl_tvoc);
      Serial.printf("✅ SGP30 基准线已恢复: eCO2=%u, TVOC=%u\n", bl_eco2, bl_tvoc);
    } else {
      Serial.println("ℹ️ SGP30 无历史基准线，需要12小时校准期");
    }
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

  // 加载数据记录粒度
  preferences.begin("system", true);
  dataSaveInterval = preferences.getUInt("saveIntv", DEFAULT_SAVE_INTERVAL);
  if (dataSaveInterval < 10) dataSaveInterval = 10;  // 最小10秒
  if (dataSaveInterval > 3600) dataSaveInterval = 3600;  // 最大1小时
  preferences.end();
  Serial.printf("📊 数据记录间隔: %u 秒\n", dataSaveInterval);

  // 加载 AI API Key
  preferences.begin("ai", true);
  aiApiKey = preferences.getString("apiKey", "");
  preferences.end();
  if (!aiApiKey.isEmpty()) Serial.println("🧠 AI API Key 已配置");

  // 初始化 AI 异步任务
  aiMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(aiTaskFunc, "AI_Task", 8192, NULL, 1, NULL, 0);  // Core0，低优先级
  Serial.println("🧠 AI 异步任务已启动");

  // 加载 QQ Bot 配置
  preferences.begin("qqbot", true);
  qqbot.appId = preferences.getString("appId", "");
  qqbot.appSecret = preferences.getString("secret", "");
  qqbot.userOpenId = preferences.getString("userId", "");
  qqbot.enabled = preferences.getBool("enabled", false);
  preferences.end();
  if (qqbot.enabled) Serial.println("✅ QQ Bot 已启用");

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
    while (WiFi.status() != WL_CONNECTED && retry < 300) {
      // 蓝色呼吸效果
      float phase = (float)(millis() % 2000) / 2000.0f;
      float br = (sin(phase * 2.0f * 3.14159f - 1.5708f) + 1.0f) / 2.0f;
      setLED(0, 0, (uint8_t)(255 * (br * 0.85f + 0.15f)));
      delay(50);
      retry++;
      if (retry % 100 == 0) Serial.print(".");
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
    server.on("/api/history", handleApiHistory);
    server.on("/api/log", handleApiLog);
    server.on("/api/dates", handleApiDates);
    server.on("/api/interval", handleApiSaveInterval);
    server.on("/api/export", handleApiExport);
    server.on("/api/ai/config", handleApiAIConfig);
    server.on("/api/qqbot/config", handleApiQQBotConfig);
    server.on("/api/qqbot/test", handleApiQQBotTest);

    // 启动 QQ Bot Gateway
    if (qqbot.enabled && !qqbot.appId.isEmpty()) {
      connectQQGateway();
    }
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

    // QQ Gateway WebSocket 循环
    if (qqbot.enabled) {
      qqGateway.loop();

      // 定时心跳
      if (qqGwIdentified && millis() - qqGwLastHeartbeat > qqGwHeartbeatInterval) {
        sendQQGatewayHeartbeat();
      }

      // 断线重连
      if (!qqGwConnected && qqGwReconnectAt > 0 && millis() > qqGwReconnectAt) {
        connectQQGateway();
      }
    }

    // SGP30 要求每 1 秒读取一次以保持内部校准算法稳定
    static unsigned long lastSGP = 0;
    if (sensor_sgp30_ok && millis() - lastSGP > 1000) {
      lastSGP = millis();
      if (sgp.IAQmeasure()) {
        eco2 = sgp.eCO2;
        tvoc = sgp.TVOC;
      }
    }

    // 定期保存 SGP30 基准线（每1小时）
    if (sensor_sgp30_ok && millis() - lastBaselineSave > BASELINE_SAVE_INTERVAL) {
      lastBaselineSave = millis();
      uint16_t bl_eco2, bl_tvoc;
      if (sgp.getIAQBaseline(&bl_eco2, &bl_tvoc)) {
        preferences.begin("sgp30", false);
        preferences.putUShort("eco2", bl_eco2);
        preferences.putUShort("tvoc", bl_tvoc);
        preferences.end();
        Serial.printf("💾 SGP30 基准线已保存: eCO2=%u, TVOC=%u\n", bl_eco2, bl_tvoc);
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

      // 更新 LED 状态（基于告警等级）
      updateLedState();
    }

    // 渲染 LED 帧（支持闪烁/呼吸动画）
    static unsigned long lastLedRender = 0;
    if (millis() - lastLedRender > 30) {  // ~33fps
      lastLedRender = millis();
      renderLed();
    }

    // 每5分钟持久化保存一次传感器数据
    if (millis() - lastDataSave > (unsigned long)dataSaveInterval * 1000UL) {
      lastDataSave = millis();
      saveDataToFile();
      ledFlashEvent(0, 200, 255, 500);  // 青色短闪500ms = 数据归档
      checkAndSendQQBotAlert();  // 顺便检查是否需要发告警
    }

    // 每天凌晨清理过期文件（通过检查小时:分钟实现）
    static uint8_t lastCleanDay = 255;
    struct tm tInfo;
    if (getLocalTime(&tInfo) && tInfo.tm_mday != lastCleanDay && tInfo.tm_hour == 3) {
      lastCleanDay = tInfo.tm_mday;
      cleanOldFiles("/data");
      cleanOldFiles("/log");
      logOperation("SYSTEM", "自动清理过期数据");
    }
  }
}