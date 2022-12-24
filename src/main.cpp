#include <WiFi.h>
#include <PubSubClient.h>
#include <M5StickCPlus.h>
#include "M5_ENV.h"

SHT3X sht30;
QMP6988 qmp6988;

float tmp = 0.0;
float hum = 0.0;
float pressure = 0.0;

#define BTN_A_PIN  37
#define BTN_ON  LOW
#define BTN_OFF HIGH
uint8_t prev_btn_a = BTN_OFF;
uint8_t btn_a      = BTN_OFF;

#define DISP_BRIGHTNESS_MIN  7 // 0〜6もセット可能だが、何も見えない
#define DISP_BRIGHTNESS_MAX 12 //
uint8_t disp_brightness = DISP_BRIGHTNESS_MIN;

#define PWR_BTN_STABLEL     0 // 押していない or 1秒以上押し続ける
#define PWR_BTN_LONG_PRESS  1 // 1秒の長押し発生
#define PWR_BTN_SHORT_PRESS 2 // 1秒以下のボタンクリック

boolean is_sleep = false;

long messageSentAt = 0;
int led,red,green,blue;

double vbat = 0.0;
int8_t bat_charge_p = 0;

void setup() {
    Serial.begin(115200);
    M5.begin(); //Init M5Stack
    Wire.begin(); //Wire init, adding the I2C bus.
    qmp6988.init();
 
    // M5.Axp.ScreenBreath(7);
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("START");

}

void showBatteryInfo(){
  // バッテリー電圧表示
  vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("Volt: %.2fV", vbat);

  // バッテリー残量表示
  // 簡易的に、線形で4.2Vで100%、3.0Vで0%とする
  bat_charge_p = int8_t((vbat - 3.0) / 1.2 * 100);
  if(bat_charge_p > 100){
    bat_charge_p = 100;
  }else if(bat_charge_p < 0){
    bat_charge_p = 0;
  }
  M5.Lcd.setCursor(0, 90);
  M5.Lcd.printf("Charge: %3d%%", bat_charge_p);

  // 液晶の明るさ表示
  M5.Lcd.setCursor(0, 110);
  M5.Lcd.printf("Brightness:%2d", disp_brightness);
}

void getEnvData() {
    pressure = qmp6988.calcPressure();
    if(sht30.get()==0){ //Obtain the data of shT30. 
      tmp = sht30.cTemp;  //Store the temperature obtained from shT30.
      hum = sht30.humidity; //Store the humidity obtained from the SHT30.
    }else{
      tmp=0,hum=0;
    }
}

void loop() {     
    btn_a = digitalRead(BTN_A_PIN);

    if(prev_btn_a == BTN_OFF && btn_a == BTN_ON){
        // ボタンAが押されたとき。1回ごとにディスプレイの明るさを上げる。
        // 最小7、最大12。12を超えたら0に戻す
        disp_brightness += 1;
        if(disp_brightness > DISP_BRIGHTNESS_MAX){
          disp_brightness = DISP_BRIGHTNESS_MIN;
        }
        M5.Axp.ScreenBreath(disp_brightness);
        M5.Lcd.setCursor(0, 45);
        M5.Lcd.printf("Brightness:%2d", disp_brightness);
    }

    if(M5.Axp.GetBtnPress() == PWR_BTN_SHORT_PRESS) {
        // 電源ボタンの短押し
        if(is_sleep){
          ; // 復帰するだけで、何もしない
        }else{
          M5.Axp.SetSleep(); // 画面が消えるだけのスリープに入る
        }
        is_sleep = !is_sleep;
    }

    long now = millis();
    if (now - messageSentAt > 5000) {
        messageSentAt = now;
        
        getEnvData();
        
        M5.Lcd.fillRect(0,0,200,200,BLACK); //Fill the screen with black (to clear the screen).
        M5.Lcd.setCursor(0,20);
        M5.Lcd.printf("Temp: %2.1f  \r\nHumi: %2.0f%%  \r\nPressure:%2.0fPa\r\n", tmp, hum, pressure);
        showBatteryInfo();
    }
}