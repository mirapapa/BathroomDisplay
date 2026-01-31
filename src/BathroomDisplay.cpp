#include "common.h"

// #define DEBUG
TaskHandle_t thp[3];

void setup()
{
  Serial.begin(115200);

  // logServerのセットアップ
  logServersetup();

  // lcdのセットアップ
  lcd_setup();

  logprintln(F(""));
  logprintln("***********************************");
  logprintln("** " SYSTEM_NAME "      **");
  logprintln("**   (ver" SYSTEM_VERSION ")                  **");
  logprintln("***********************************");
#ifdef CONFIG_APP_ROLLBACK_ENABLE
  logprintln(F("CONFIG_APP_ROLLBACK_ENABLE"));
#endif // CONFIG_APP_ROLLBACK_ENABLE
  logprintln(F(""));

  // Watchdog Timerのセットアップ
  watchdog_setup();

  // NTPクライアントのセットアップ
  ntp_setup();

  // wifiのセットアップ
  wifisetup();

  // sensorタスク起動
  xTaskCreatePinnedToCore(lcd_task, "LCD_TASK", 4096, NULL, 10, &thp[0], APP_CPU_NUM);
  // UDPサーバータスク起動
  xTaskCreatePinnedToCore(udpServer_task, "UDPSERVER_TASK", 4096, NULL, 9, &thp[1], APP_CPU_NUM);
  // logServerタスク起動
  xTaskCreatePinnedToCore(logServer_task, "LOGSERVER_TASK", 4096, NULL, 1, &thp[2], APP_CPU_NUM);

  // otaのセットアップ
  ota_setup();

  logprintln(F("<<浴室自動換気表示システム再起動>>"), 1);
}

void loop() // メインCPU(Core1)で実行するプログラム
{
  // wifi接続判定
  wificheck();

  // OTA処理（ArduinoOTA + WEB OTA）
  ota_handle();

  delay(100);
}