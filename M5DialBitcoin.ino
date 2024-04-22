#include <M5Dial.h>

#define WIFI_SSID     "xxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"

#include <WiFi.h>
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ビットコイン値段を取得
const char *apiServer = "https://api.coindesk.com/v1/bpi/currentprice/JPY.json";

char buf[100];
int sx, sy;
int nowyen = 0;  // 最新のBTC
int oldyen;      // ひとつ前のBTC
int redraw = 0;  // BTC更新で表示更新
int bhour = 0;   // BTC取得時間
int bmin = 0;    // BTC取得時間
int bsec = 0;    // BTC取得時間
char date[100];
int bup = 0;     // 値上がり
int bdown = 0;   // 値下がり

// 初期設定
void setup() {
	// put your setup code here, to run once:
	
	auto cfg = M5.config();
	M5Dial.begin(cfg, true, true);
	
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.setTextColor(GREEN, BLACK);
	M5Dial.Display.setTextDatum(middle_center);
	M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
	M5Dial.Display.setTextSize(0.5);
	
	sx = M5Dial.Display.width() / 2;
	sy = M5Dial.Display.height() / 2;
	
	// WiFi接続
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.drawString("WiFi connecting...", sx, sy);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	int tout = 0;
	while(WiFi.status() != WL_CONNECTED){
		delay(500);
		tout++;
		if(tout > 10){  // 5秒でタイムアウト、接続リトライ
			WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
			tout = 0;
		}
	}
	Serial.println("WiFi Connected.");
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.drawString("Connected.", sx, sy);
	
	Serial.printf("sx=%d, sy=%d\n", sx, sy);
	
	redraw = 0;
	// ビットコインチェックタスク起動
	xTaskCreatePinnedToCore(BitcoinTask, "BitcoinTask", 4096, NULL, 1, NULL, 1);
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.drawString("Getting BTC...", sx, sy);
	
	// ビットコイン価格取得待ち
	while(redraw == 0){
		delay(500);
	}
	Serial.print("\n");
	redraw = 0;
	
	M5Dial.Display.fillScreen(BLACK);
	// BTC
	M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
	M5Dial.Display.setTextSize(1);
	M5Dial.Display.setTextColor(WHITE, BLACK);
	M5Dial.Display.drawString("BTC", sx, sy-50);
	// 取得時間
	sprintf(buf, "%02d:%02d:%02d", bhour, bmin, bsec);
	M5Dial.Display.setTextFont(7);
	M5Dial.Display.setTextSize(0.5);
	M5Dial.Display.setTextColor(OLIVE, BLACK);
	M5Dial.Display.drawString(buf, sx, sy+50);
	// 円価値
	sprintf(buf, "%02d.%03d.%03d", (nowyen / 1000000), ((nowyen / 1000) % 1000), (nowyen % 1000));
	M5Dial.Display.setTextFont(7);
	M5Dial.Display.setTextSize(0.65);
	M5Dial.Display.setTextColor(WHITE, BLACK);
	M5Dial.Display.drawString(buf, sx, sy);
	
}

// メインループ
void loop() {
	// put your main code here, to run repeatedly:
	M5Dial.update();
	if(M5Dial.BtnA.wasPressed()){
		bup = 0;
		bdown = 0;
		M5Dial.Display.fillArc(sx, sy, 100, 120, 0, 360, BLACK);
	}
	
	if(redraw != 0){
		// BTC
		M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
		M5Dial.Display.setTextSize(1);
		M5Dial.Display.setTextColor(WHITE, BLACK);
		M5Dial.Display.drawString("BTC", sx, sy-50);
		// 取得時間
		sprintf(buf, "%02d:%02d:%02d", bhour, bmin, bsec);
		M5Dial.Display.setTextFont(7);
		M5Dial.Display.setTextSize(0.5);
		M5Dial.Display.setTextColor(OLIVE, BLACK);
		M5Dial.Display.drawString(buf, sx, sy+50);
		// 円価値
		sprintf(buf, "%02d.%03d.%03d", (nowyen / 1000000), ((nowyen / 1000) % 1000), (nowyen % 1000));
		M5Dial.Display.setTextFont(7);
		M5Dial.Display.setTextSize(0.65);
		if(redraw > 0){
			M5Dial.Display.setTextColor(RED, BLACK);
		}
		else if(redraw < 0){
			M5Dial.Display.setTextColor(GREEN, BLACK);
		}
		else{
			M5Dial.Display.setTextColor(WHITE, BLACK);
		}
		M5Dial.Display.drawString(buf, sx, sy);
		
		Serial.printf("BTC = %d (%02d:%02d:%02d)\n", nowyen, bhour, bmin, bsec);
		
		if(redraw > 0){
			if(bup < 180){
				bup++;
				if(bup < 90){
					M5Dial.Display.fillArc(sx, sy, 100, 120, 270, 270+bup, RED);
					M5Dial.Display.fillArc(sx, sy, 100, 120, 270+bup, 360, BLACK);
					M5Dial.Display.fillArc(sx, sy, 100, 120, 0, 90, BLACK);
				}
				else{
					M5Dial.Display.fillArc(sx, sy, 100, 120, 270, 360, RED);
					M5Dial.Display.fillArc(sx, sy, 100, 120, 0, bup-90, RED);
					M5Dial.Display.fillArc(sx, sy, 100, 120, bup-90, 90, BLACK);
				}
			}
		}
		else{
			if(bdown < 180){
				bdown++;
				M5Dial.Display.fillArc(sx, sy, 100, 120, 90, 270-bdown, BLACK);
				M5Dial.Display.fillArc(sx, sy, 100, 120, 270-bdown, 270, GREEN);
			}
		}
		
		redraw = 0;
	}
	else{
		delay(20);
	}
}


// ビットコインチェック用タスク
void BitcoinTask(void* arg) 
{
	int httpCode;
	double rate;
	JsonObject obj;
	JsonObject result;
	char *str;
	char ss[10];
	// json用メモリ確保
	DynamicJsonDocument doc(1024);
	
	Serial.println("Start BitcoinTask.");
	
	while(1){
		if((WiFi.status() == WL_CONNECTED)){
			HTTPClient http;
			
			// HTTP接続開始
			http.begin(apiServer);
			
			// リクエスト作成
			httpCode = http.GET();
			
			// 返信
			if(httpCode > 0){
				// 応答取得
				String payload = http.getString();
				// ペイロードをjson変換
				deserializeJson(doc, payload);
				obj = doc.as<JsonObject>();
				
				// bpi.JPY
				result = obj[String("bpi")][String("JPY")];
				// ビットコインレート
				rate = result[String("rate_float")];
				oldyen = nowyen;
				nowyen = (int)(rate);
				
				// time
				result = obj[String("time")];
				// updated
				strcpy(date, result[String("updated")]);
				// 3個目のスペースから時間
				str = strchr(date, ' ');
				str++;
				str = strchr(str, ' ');
				str++;
				str = strchr(str, ' ');
				str++;
				strncpy(ss, str, 2);
				bhour = atoi(ss);
				bhour = ((bhour + 9) % 24);
				str += 3;
				strncpy(ss, str, 2);
				bmin = atoi(ss);
				str += 3;
				strncpy(ss, str, 2);
				bsec = atoi(ss);
				
				Serial.printf("%d : %d : %s\n", nowyen, oldyen, date);
				
				if(nowyen > oldyen){
					redraw = 1;
				}
				else if(nowyen < oldyen){
					redraw = -1;
				}
				if(redraw != 0){
					Serial.printf("Bitcoin %d yen\n", nowyen);
					Serial.printf("%02d:%02d:%02d\n", bhour, bmin, bsec);
				}
			}
			else{
				Serial.print("x");
			}
			http.end();
		}
		else{
			Serial.print("WiFi Error!");
			// 再接続
			WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		}
		vTaskDelay(5000);
	}
	
	//Serial.println("STOP BitcoinTask.");
	vTaskDelay(10);
	// タスク削除
	vTaskDelete(NULL);
}
