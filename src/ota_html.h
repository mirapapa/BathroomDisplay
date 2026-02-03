#include "ota_common.h"

// 換気扇システム固有のHTMLパーツ
const char SPECIFIC_PART[] PROGMEM = R"rawliteral(

)rawliteral";

// 最終的なHTMLを生成する関数
String getOtaHtml() {
String specific = String(SPECIFIC_PART);

// 固有パーツの値を埋め込む
//specific.replace("{{JUDGEONTIME}}", String(judgeOnTime));
//specific.replace("{{STARTTIME}}", String(startTime));
//specific.replace("{{CONTINUETIME}}", String(continueTime));
//specific.replace("{{HISTORYDATA}}", getHistoryData());

return specific;
}