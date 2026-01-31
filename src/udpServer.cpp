#include "common.h"
#include <WiFiUdp.h>

WiFiUDP udp;
char pBuffer[256];
static const int localPort = 5555;                // 受信ポート番号
static const int rmoteUdpPort = 5555;             // 送信先のポート番号
static const char *remoteIpadr = "192.168.11.51"; // 送信先のIPアドレス

void udpServer_setup(void) { /* NONE */ }

void udpServer_task(void *pvParameters)
{
  logprintln("udpServer_task START!!");
  delay(100); // 各タスク起動待ち

  watchdog_subscribe_task("UDPSERVER_TASK"); // 名前を修正

  bool isUdpRunning = false;

  while (1)
  {
    watchdog_reset();

    if (WiFi.status() == WL_CONNECTED)
    {
      if (!isUdpRunning)
      {
        // 接続された、かつUDPがまだ動いていない時だけbeginする
        if (udp.begin(localPort))
        {
          logprintln("UDP Server started on port " + String(localPort));
          isUdpRunning = true;
        }
      }

      // UDP受信処理
      int packetSize = udp.parsePacket();
      if (packetSize)
      {
        memset(pBuffer, 0x00, 256);
        udp.read(pBuffer, 256);
        memcpy(&r_dState, &pBuffer[0], sizeof(SRData));
      }
    }
    else
    {
      // WiFiが切れたら、次に繋がった時に再開できるようにフラグを下ろす
      if (isUdpRunning)
      {
        logprintln("WiFi disconnected, UDP Server stopped.");
        udp.stop(); // 明示的に止める
        isUdpRunning = false;
      }
    }

    delay(100);
  }
}

// 動作モード送信処理
void sendDMode(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    udp.beginPacket(remoteIpadr, rmoteUdpPort);
    udp.write((uint8_t *)&s_dState, sizeof(SRData));
    udp.endPacket();
  }
}