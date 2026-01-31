#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "esp_sntp.h"
#include <ArduinoOTA.h>
#include "esp_ota_ops.h"
#include <LiquidCrystal_I2C.h>
#include <ESPmDNS.h>

// --- 構造体の定義 ---
typedef struct version
{
    unsigned char id;
    unsigned char verMejor;
    unsigned char verMinor;
    unsigned char verPatch;
} VERSION;

typedef struct
{
    bool send_flg;
    String data;
    int last_http_code;
} SENDSSDATATOSS;

// 再起動記録の構造体
typedef struct
{
    time_t timestamp;     // 再起動時刻
    uint8_t rebootReason; // 再起動理由（esp_reset_reason_t）
    char message[64];     // メッセージ
} RebootRecord;

// リングバッファ形式の再起動ログ
typedef struct
{
    RebootRecord records[10];  // 最大10件
    uint8_t writeIndex;        // 次に書き込む位置
    uint8_t recordCount;       // 現在の記録数
    uint32_t totalRebootCount; // 総再起動回数
} RebootLog;

// 機器状態データ
typedef struct deviceState
{
    unsigned char lState;
    unsigned char lForce;
    unsigned char vState;
    unsigned char vForce;
    unsigned char mState;
    unsigned char mForce;
} DState;
// 送受信データ
typedef struct sendrecvData
{
    VERSION version;
    DState dState;
} SRData;

// --- 全ての自作関数のプロトタイプ宣言 ---

// ログ関連
void logServer_task(void *pvParameters);
void logprintln(String log);
void logprintln(String log, bool historyFlg);
void logServersetup();
uint nextnum(uint num);
uint prevnum(uint num);
String getHistoryData();
String getSystemTimeStr();

// ネットワーク関連
void ntp_setup();
int wifisetup();
void wificheck();
void mdnssetup();
void timeavailable(struct timeval *t);
bool isWiFiReallyConnected(); // 新規追加

// OTA関連
void ota_setup();
void verifyFirmware();
void ota_handle(); // 新規追加

// Watchdog関連
void watchdog_setup();
void watchdog_subscribe_task(const char *taskName);
void watchdog_reset();
void watchdog_unsubscribe_task(const char *taskName);

// 再起動ログ関連
void rebootLog_setup();
void loadRebootLog();
void saveRebootLog();
void addRebootRecord(esp_reset_reason_t reason, const char *message);
String getRebootLogJson();
String getRebootLogHtml();
void clearRebootLog();
String getRebootReasonString(esp_reset_reason_t reason);
esp_reset_reason_t getCurrentRebootReason();

// セマフォ関連
void takeSemaphore(SemaphoreHandle_t xSemaphore);
void giveSemaphore(SemaphoreHandle_t xSemaphore);

// lcd関連
void lcd_setup(void);
void lcd_task(void *pvParameters);
void drawT();
void drawB();
void drawL();
void drawV();
void drawM();
void drawVMode();
void drawMMode();
void drawClock();
void increment(unsigned char *value, unsigned char min, unsigned char max);
void getNowTime(char *ymd, char *hms);

// UDPサーバ関連
void udpServer_setup(void);
void udpServer_task(void *pvParameters);
void sendDMode(void);

// --- 全ての外部変数の宣言 ---
extern const VERSION version;
extern SENDSSDATATOSS sendHDatatoSS;
// extern SemaphoreHandle_t xHistorySemaphore; // 履歴データ(Web表示)用
extern SemaphoreHandle_t xDataSemaphore; // 送信データ(SS/Ambient)用
extern bool firstTimeNtpFlg;
extern RebootLog rebootLog; // 再起動ログ
extern DState pre_dState;
extern SRData r_dState;
extern SRData s_dState;

#endif // COMMON_H