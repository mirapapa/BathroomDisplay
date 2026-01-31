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

  while (1)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      udp.begin(localPort);
      break;
    }
    else
    {
      delay(100);
      continue;
    }
  }

  while (1)
  {
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
      memset(pBuffer, 256, 0x00);
      udp.read(pBuffer, 256);
      memcpy(&r_dState, &pBuffer[0], sizeof(SRData));
    }
    delay(100);
  }

  vTaskDelete(NULL);
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