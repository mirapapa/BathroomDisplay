#include "common.h"
#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
// #include "lgfx_ESP32_2432S028.h"
#include "image.c"

static LGFX lcd;

// 画像の座標とサイズ
enum enum_IMG
{
  IMG_X,
  IMG_Y,
  IMG_W,
  IMG_H,
  IMG_MAX
};
//                         { IMG_X                              , IMG_Y           , IMG_W        , IMG_H        };
const int32_t T[IMG_MAX] = {0, 10, bathroom_W, bathroom_H};
const int32_t L[IMG_MAX] = {30, 60, lightoff_W, lightoff_H};
const int32_t V[IMG_MAX] = {120, 60, vfanoff_W, vfanoff_H};
const int32_t M[IMG_MAX] = {30 + lightoff_W / 2 - speakeroff_W / 2, 60 + lightoff_H, speakeroff_W, speakeroff_H};
const int32_t B[IMG_MAX] = {0, 320 - minion_H, minion_W, minion_H};

// 画像データ
enum num_MOTION
{
  MOTION_OFF,
  MOTION_ON1,
  MOTION_ON2,
  MOTION_MAX
};
const unsigned short *T_IMAGE[MOTION_MAX] = {bathroom, 0, 0};
const unsigned short *L_IMAGE[MOTION_MAX] = {lightoff, lighton, 0};
const unsigned short *V_IMAGE[MOTION_MAX] = {vfanoff, vfanon1, vfanon2};
const unsigned short *M_IMAGE[MOTION_MAX] = {speakeroff, speakeron1, speakeron2};
const unsigned short *B_IMAGE[MOTION_MAX] = {minion, 0, 0};

// 制御状態
enum num_TYPE
{
  TYPE_L,
  TYPE_V,
  TYPE_M,
  TYPE_MAX
};
unsigned char mState[TYPE_MAX] = {MOTION_ON1, MOTION_ON1, MOTION_ON1}; // 動作状態
unsigned char mCount[TYPE_MAX] = {0};                                  // 動作カウンタ
char prev_ymd[24];
char prev_hms[24];

DState pre_dState;
SRData r_dState;
SRData s_dState;

void lcd_setup(void)
{
  memset(&pre_dState, 0x01, sizeof(DState));
  memset(&r_dState, 0x00, sizeof(SRData));
  memset(&s_dState, 0x00, sizeof(SRData));
  // バージョン情報設定
  s_dState.version.id = 'V';
  sscanf(SYSTEM_VERSION, "%hhu.%hhu.%hhu",
         &s_dState.version.verMejor,
         &s_dState.version.verMinor,
         &s_dState.version.verPatch);

  lcd.init();
  lcd.setRotation(2);
  lcd.setColorDepth(24);
  lcd.setBrightness(200);
  lcd.fillScreen(TFT_BLACK);

  drawT();
  drawB(); // 画像描画
  drawL();
  drawV();
  drawM(); // アイコン描画
  drawVMode();
  drawMMode(); // モード描画
}

void lcd_task(void *pvParameters)
{
  logprintln("lcd_task START!!");
  delay(100); // 各タスク起動待ち

  while (1)
  {
    // 画面タッチ有無確認
    uint16_t tX, tY;
    bool touched = lcd.getTouch(&tX, &tY);
    if (touched)
    {
      logprintln("touched (" + String(tX) + "," + String(tY) + ")");
      // 電気アイコンエリアタッチ確認
      if ((L[IMG_X] < tX) && (tX < (L[IMG_W] + L[IMG_X])) &&
          (L[IMG_Y] < tY) && (tY < (L[IMG_H] + L[IMG_Y])))
      {
        delay(500);
      }
      // 換気扇アイコンエリアタッチ確認
      if ((V[IMG_X] < tX) && (tX < (V[IMG_W] + V[IMG_X])) &&
          (V[IMG_Y] < tY) && (tY < (V[IMG_H] + V[IMG_Y])))
      {
        increment(&r_dState.dState.vForce, 0, 1);
        drawVMode();
        memcpy(&s_dState.dState, &r_dState.dState, sizeof(DState));
        sendDMode();
        delay(500); // この間の連続タッチを無効化する
      }
      // 音楽アイコンエリアタッチ確認
      if ((M[IMG_X] < tX) && (tX < (M[IMG_W] + M[IMG_X])) &&
          (M[IMG_Y] < tY) && (tY < (M[IMG_H] + M[IMG_Y])))
      {
        increment(&r_dState.dState.mForce, 0, 1);
        drawMMode();
        memcpy(&s_dState.dState, &r_dState.dState, sizeof(DState));
        sendDMode();
        delay(500); // この間の連続タッチを無効化する
      }
    }

    drawL();
    drawV();
    drawM(); // アイコン描画更新
    drawVMode();
    drawMMode(); // モード描画更新
    drawClock(); // 時計描画更新

    delay(50);
  }

  vTaskDelete(NULL);
}

// タイトル描画
void drawT(void)
{
  lcd.pushImage(T[IMG_X], T[IMG_Y], T[IMG_W], T[IMG_H], T_IMAGE[MOTION_OFF]);
  lcd.setTextColor(0xFFFFFFU);
  lcd.setFont(&fonts::Font2);
  lcd.setCursor(lcd.width() - 80, T[IMG_Y] + T[IMG_H] + 2);
  lcd.print("Ver. 1.1.0");
}

// 電気アイコン描画
void drawL(void)
{
  if (r_dState.dState.lState != pre_dState.lState)
  {
    if (r_dState.dState.lState)
    {
      lcd.pushImage(L[IMG_X], L[IMG_Y], L[IMG_W], L[IMG_H], L_IMAGE[MOTION_ON1]);
    }
    else
    {
      lcd.pushImage(L[IMG_X], L[IMG_Y], L[IMG_W], L[IMG_H], L_IMAGE[MOTION_OFF]);
    }
    pre_dState.lState = r_dState.dState.lState;
  }
}

// 換気扇アイコン描画
void drawV(void)
{
  if (r_dState.dState.vState)
  {
    mCount[TYPE_V]++;
    if (mCount[TYPE_V] > 10)
    {
      increment(&mState[TYPE_V], MOTION_ON1, MOTION_ON2);
      lcd.pushImage(V[IMG_X], V[IMG_Y], V[IMG_W], V[IMG_H], V_IMAGE[mState[TYPE_V]]);
      pre_dState.vState = r_dState.dState.vState;
      mCount[TYPE_V] = 0;
    }
  }
  else
  {
    if (r_dState.dState.vState != pre_dState.vState)
    {
      lcd.pushImage(V[IMG_X], V[IMG_Y], V[IMG_W], V[IMG_H], V_IMAGE[MOTION_OFF]);
      pre_dState.vState = r_dState.dState.vState;
      mCount[TYPE_V] = 10;
    }
  }
}

// 音楽アイコン描画
void drawM(void)
{
  if (r_dState.dState.mState)
  {
    mCount[TYPE_M]++;
    if (mCount[TYPE_M] > 10)
    {
      increment(&mState[TYPE_M], MOTION_ON1, MOTION_ON2);
      lcd.pushImage(M[IMG_X], M[IMG_Y], M[IMG_W], M[IMG_H], M_IMAGE[mState[TYPE_M]]);
      pre_dState.mState = r_dState.dState.mState;
      mCount[TYPE_M] = 0;
    }
  }
  else
  {
    if (r_dState.dState.mState != pre_dState.mState)
    {
      lcd.pushImage(M[IMG_X], M[IMG_Y], M[IMG_W], M[IMG_H], M_IMAGE[MOTION_OFF]);
      pre_dState.mState = r_dState.dState.mState;
      mCount[TYPE_M] = 10;
    }
  }
}

// ボトム描画
void drawB(void)
{
  lcd.pushImage(B[IMG_X], B[IMG_Y], B[IMG_W], B[IMG_H], B_IMAGE[MOTION_OFF]);
}

// 換気扇モード描画
void drawVMode(void)
{
  if (r_dState.dState.vForce != pre_dState.vForce)
  {
    lcd.setFont(&fonts::Font2);
    lcd.setCursor(V[IMG_X] + 7, V[IMG_Y] + V[IMG_H] + 3);
    if (r_dState.dState.vForce)
    {
      lcd.setTextColor(0xFF0000U, 0xFFC0CBU);
      lcd.print(" Force Mode ");
    }
    else
    {
      lcd.setTextColor(0xFFFFFFU, 0x000000U);
      lcd.print(" Auto  Mode  ");
    }
    pre_dState.vForce = r_dState.dState.vForce;
  }
}

// 音楽モード描画
void drawMMode(void)
{
  if (r_dState.dState.mForce != pre_dState.mForce)
  {
    lcd.setFont(&fonts::Font2);
    lcd.setCursor(M[IMG_X] + 15, M[IMG_Y] + M[IMG_H] + 3);
    if (r_dState.dState.mForce)
    {
      lcd.setTextColor(0x0000FFU, 0x87ceebU);
      lcd.print(" Stop Music ");
    }
    else
    {
      lcd.setTextColor(0xFFFFFFU, 0x000000U);
      lcd.print(" Auto  Mode  ");
    }
    pre_dState.mForce = r_dState.dState.mForce;
  }
}

// 時計表示
void drawClock(void)
{
  char ymd[24];
  char hms[24];
  getNowTime(ymd, hms);

  if (strcmp(ymd, prev_ymd) != 0)
  {
    strcpy(prev_ymd, ymd);

    lcd.setTextColor(0xFFFFFFU, 0x000000U);
    lcd.setFont(&fonts::Font2);
    lcd.setCursor(125, 225);
    lcd.println(ymd);
  }
  else if (strcmp(hms, prev_hms) != 0)
  {
    strcpy(prev_hms, hms);

    lcd.setTextColor(0xFFFFFFU, 0x000000U);
    lcd.setFont(&fonts::Font4);
    lcd.setCursor(125, 240);
    lcd.println(hms);
  }
}

// 時刻取得
void getNowTime(char *ymd, char *hms)
{
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  sprintf(ymd, "%04d/%02d/%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
  sprintf(hms, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

// インクリメント関数
void increment(unsigned char *num, unsigned char min, unsigned char max)
{
  if (*num >= max)
    *num = min;
  else
    (*num)++;
}