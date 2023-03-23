/**********************************************************************
    本程序为网络时钟，通过互联网获取当地时间和天气预报
    自行填写WIFI名称、密码及和风天气私钥 
    程序设计：Sunnysucd Email:bizman2000@foxmail.com
***********************************************************************/
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>              
#include <ESP8266_Heweather.h>

//定义显示模块
#define NUM_MAX 4

#define DIN_PIN D7
#define CS_PIN  D8
#define CLK_PIN D5
#include "max7219.h"
#include "fonts.h" 

// =============================定义变量==============================
#define MAX_DIGITS 20
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
int Lval=0;
long localEpoc = 0;
long localMillisAtUpdate = 0;
String today_day,today;
String UserKey = "XXXXXXX";   // 私钥 https://dev.heweather.com/docs/start/get-api-key
String Location = "102270101"; // 城市代码 https://github.com/heweather/LocationList,表中的 Location_ID
String Unit = "m";             // 公制-m/英制-i
String Lang = "en";            // 语言 英文-en/中文-zh
int err = 0;


// NTP时间服务器请求所需信息
WiFiUDP ntpUDP;
const PROGMEM char *ntpServer = "ntp3.aliyun.com"; //ntp2.aliyun.com,ntp3.aliyun.com,ntp4.aliyun.com,ntp5.aliyun.com,ntp6.aliyun.com,cn.pool.ntp.org
NTPClient timeClient(ntpUDP, ntpServer, 3600*8, 60000); //此处可更换时区
//开启天气服务
WeatherForecast WeatherForecast;
AirQuality AirQuality;
WeatherNow WeatherNow;  

void setup(){
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,10);          
  Serial.println("");
//连接WIFI
 delay(100);
  if (!AutoConfig())
  {
      SmartConfig();
  }
//
// 开启NTP时间服务器请求信息
  timeClient.begin();
// 开启天气配置请求信息
WeatherForecast.config(UserKey, Location, Unit, Lang); 
AirQuality.config(UserKey, Location, Unit, Lang);
WeatherNow.config(UserKey, Location, Unit, Lang); 
printStringWithShift("  Setting Time And Loading Weather Data...",15);
}
 
void loop(){
  if(updCnt<=0) { // every 10 scrolls, ~450s=7.5m
    updCnt = 60;
    Serial.println("Getting data ...");
    getTime();
    getWeather();
    Serial.println("Data loaded");
    clkTime = millis();
  }

    if(millis()-clkTime > 60000 && !del && dots) { // clock for 50s, then scrolls for about 50s
    if (err == 1){ for (int j = 0; j < 3; j++){getWeather();if (err == 0){ break; }}}
    if (err == 0){today ="  " +timeClient.getFormattedDate() +"  " + today_day +"  "+WeatherNow.getWeatherText()+" ("+WeatherNow.getTemp()+")"+"  T:"+WeatherForecast.getTempMin(0)+"/"+WeatherForecast.getTempMax(0)+"  H:"+int(WeatherNow.getHumidity())+"%"+"  AQI:"+AirQuality.getAqi()+"  "+AirQuality.getCategory()+" ";}
    else{today ="  " +timeClient.getFormattedDate() +"  " + today_day +"  "+"  No Weather Data!";}
    printStringWithShift(today.c_str(),40);
    delay(1000);
    updCnt--;
    clkTime = millis();
    }
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
  
  LightControl();
  updateTime();
  showAnimClock();
}

void getWeather(){
      if(WeatherForecast.get()){ // 获取天气更新
 
      Serial.print(F("==========Day "));
      Serial.print(0);
      Serial.println(F("=========="));
      Serial.print("Server Response: ");
      Serial.println(WeatherForecast.getServerCode());    // 获取API状态码
      Serial.print(F("Last Update: "));
      Serial.println(WeatherForecast.getLastUpdate());    // 获取服务器更新天气信息时间
      Serial.print(F("TempMax: "));
      Serial.println(WeatherForecast.getTempMax(0));      // 获取最高温度
      Serial.print(F("TempMin: "));
      Serial.println(WeatherForecast.getTempMin(0));      // 获取最低温度
      Serial.print(F("Icon: "));
      Serial.println(WeatherForecast.getIconDay(0));      // 获取天气图标代码
      Serial.print(F("Weather Direction: "));
      Serial.println(WeatherForecast.getTextDay(0));      // 获取天气状况的文字描述
      Serial.print(F("WindDir: "));
      Serial.println(WeatherForecast.getWindDirDay(0));   // 获取风向
      Serial.print(F("WindScale: "));
      Serial.println(WeatherForecast.getwindScaleDay(0)); // 获取风力等级
      Serial.print(F("Humidity: "));
      Serial.println(WeatherForecast.getHumidity(0));     // 获取相对湿度百分比数值
      Serial.print(F("Precip: "));
      Serial.println(WeatherForecast.getPrecip(0));       // 获取降水量,毫米
      Serial.print(F("UvIndex: "));
      Serial.println(WeatherForecast.getUvIndex(0));      // 获取紫外线强度指数
      Serial.println(F("========================="));
    
  } else {    // 更新失败
    Serial.println("Update Failed...");
    Serial.print("Server Response: ");
    Serial.println(WeatherForecast.getServerCode()); // 参考 https://dev.heweather.com/docs/start/status-code
  }

  if(AirQuality.get()){ // 获取更新
    Serial.println(F("======AirQuality Info======"));
    Serial.print("Server Response: ");
    Serial.println(AirQuality.getServerCode()); // 获取API状态码
    Serial.print(F("Last Update: "));
    Serial.println(AirQuality.getLastUpdate()); // 获取服务器更新天气信息时间
    Serial.print(F("AirQuality Aqi: "));
    Serial.println(AirQuality.getAqi());        // 实时空气质量指数
    Serial.print(F("Category: "));
    Serial.println(AirQuality.getCategory());   // 实时空气质量指数级别
    Serial.print(F("Primary: "));
    Serial.println(AirQuality.getPrimary());    // 实时空气质量的主要污染物，优时返回值为 NA
    Serial.println(F("========================"));
  } else {  // 更新失败
    Serial.println("Update Failed...");
    Serial.print("Server Response: ");
    Serial.println(AirQuality.getServerCode()); // 参考 https://dev.heweather.com/docs/start/status-code
  }

  if(WeatherNow.get()){ // 获取天气更新
    Serial.println(F("======Weahter Now Info======"));
    Serial.print("Server Response: ");
    Serial.println(WeatherNow.getServerCode());  // 获取API状态码
    Serial.print(F("Last Update: "));
    Serial.println(WeatherNow.getLastUpdate());  // 获取服务器更新天气信息时间
    Serial.print(F("Temperature: "));
    Serial.println(WeatherNow.getTemp());        // 获取实况温度
    Serial.print(F("FeelsLike: "));
    Serial.println(WeatherNow.getFeelLike());    // 获取实况体感温度
    Serial.print(F("Icon: "));
    Serial.println(WeatherNow.getIcon());        // 获取当前天气图标代码
    Serial.print(F("Weather Now: "));
    Serial.println(WeatherNow.getWeatherText()); // 获取实况天气状况的文字描述
    Serial.print(F("windDir: "));
    Serial.println(WeatherNow.getWindDir());     // 获取实况风向
    Serial.print(F("WindScale: "));
    Serial.println(WeatherNow.getWindScale());   // 获取实况风力等级
    Serial.print(F("Humidity: "));
    Serial.println(WeatherNow.getHumidity());    // 获取实况相对湿度百分比数值
    Serial.print(F("Precip: "));
    Serial.println(WeatherNow.getPrecip());      // 获取实况降水量,毫米
    Serial.println(F("========================"));
  } else {    // 更新失败
    Serial.println("Update Failed...");
    Serial.print("Server Response: ");
    Serial.println(WeatherNow.getServerCode()); // 参考 https://dev.heweather.com/docs/start/status-code
  }
  if(WeatherForecast.getServerCode()=="200"&&AirQuality.getServerCode()=="200"&&WeatherNow.getServerCode()=="200"){err=0;}else{err=1;}
}

void getTime(){
     timeClient.update();
     Serial.println(timeClient.getFormattedTime());
     Serial.println(timeClient.getFormattedDate());
     Serial.println(timeClient.getEpochTime());
     switch (timeClient.getDay()){
     case 0: today_day = "Sun";break;
     case 1: today_day = "Mon";break;
     case 2: today_day = "Tue";break;
     case 3: today_day = "Wed";break;
     case 4: today_day = "Thu";break;
     case 5: today_day = "Fri";break;
     case 6: today_day = "Sat";break;
      }   
     h = timeClient.getHours();
     m = timeClient.getMinutes();
     s = timeClient.getSeconds();
     Serial.print(timeClient.getDay());
     Serial.print(timeClient.getHours());
     Serial.print(timeClient.getMinutes());
     Serial.print(timeClient.getSeconds());
     localMillisAtUpdate = millis();
     localEpoc = (h * 60 * 60 + m * 60 + s);
     delay(300);
}

void showAnimClock()
{
  byte digPos[6]={0,8,17,25,34,42};
  int digHt = 12;
  int num = 6; 
  int i;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];
    dig[0] = h/10 ? h/10 : 10;
    dig[1] = h%10;
    dig[2] = m/10;
    dig[3] = m%10;
    dig[4] = s/10;
    dig[5] = s%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
  } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig6x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(15,dots ? B00100100 : 0);
  setCol(32,dots ? B00100100 : 0);
  refreshAll();
  delay(30);
}

void showDigit(char ch, int col, const uint8_t *data)
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
    }
}

void setCol(int col, byte v)
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

// =======================================================================
int dualChar = 0;

unsigned char convertPolish(unsigned char _c)
{
  unsigned char c = _c;
  if(c==196 || c==197 || c==195) {
    dualChar = c;
    return 0;
  }
  if(dualChar) {
    switch(_c) {
      case 133: c = 1+'~'; break; // 'ą'
      case 135: c = 2+'~'; break; // 'ć'
      case 153: c = 3+'~'; break; // 'ę'
      case 130: c = 4+'~'; break; // 'ł'
      case 132: c = dualChar==197 ? 5+'~' : 10+'~'; break; // 'ń' and 'Ą'
      case 179: c = 6+'~'; break; // 'ó'
      case 155: c = 7+'~'; break; // 'ś'
      case 186: c = 8+'~'; break; // 'ź'
      case 188: c = 9+'~'; break; // 'ż'
      //case 132: c = 10+'~'; break; // 'Ą'
      case 134: c = 11+'~'; break; // 'Ć'
      case 152: c = 12+'~'; break; // 'Ę'
      case 129: c = 13+'~'; break; // 'Ł'
      case 131: c = 14+'~'; break; // 'Ń'
      case 147: c = 15+'~'; break; // 'Ó'
      case 154: c = 16+'~'; break; // 'Ś'
      case 185: c = 17+'~'; break; // 'Ź'
      case 187: c = 18+'~'; break; // 'Ż'
      default:  break;
    }
    dualChar = 0;
    return c;
  }    
  switch(_c) {
    case 185: c = 1+'~'; break;
    case 230: c = 2+'~'; break;
    case 234: c = 3+'~'; break;
    case 179: c = 4+'~'; break;
    case 241: c = 5+'~'; break;
    case 243: c = 6+'~'; break;
    case 156: c = 7+'~'; break;
    case 159: c = 8+'~'; break;
    case 191: c = 9+'~'; break;
    case 165: c = 10+'~'; break;
    case 198: c = 11+'~'; break;
    case 202: c = 12+'~'; break;
    case 163: c = 13+'~'; break;
    case 209: c = 14+'~'; break;
    case 211: c = 15+'~'; break;
    case 140: c = 16+'~'; break;
    case 143: c = 17+'~'; break;
    case 175: c = 18+'~'; break;
    default:  break;
  }
  return c;
}

// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay) {
  c = convertPolish(c);
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

void printStringWithShift(const char* s, int shiftDelay){
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = int( curEpoch  ) % 86400L;
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}
//亮度自动控制
void LightControl()
{  Lval =10 - map(analogRead(A0),0,1023,0,10);  
   sendCmdAll(CMD_INTENSITY,Lval);
}

//一键配网
void SmartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  printStringWithShift("Waiting For Smartconfig... ",15);   
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    delay(500);                   // wait for a second
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      printStringWithShift("SmartConfig Success!",15); 
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}

bool AutoConfig()
{
    WiFi.begin();
    printStringWithShift("Connecting to Wifi ",15);              //等待网络连接
    for (int i = 0; i < 20; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
            Serial.println("Connection established!");   // NodeMCU将通过串口监视器输出"连接成功"信息。
            printStringWithShift("  Connection established!",15);     //已经建立网络连接
            Serial.printf("SSID:%s", WiFi.SSID().c_str());
            Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
            Serial.print("LocalIP:");
            Serial.print(WiFi.localIP());
            Serial.print(" ,GateIP:");
            Serial.println(WiFi.gatewayIP());
            return true;
        }
        else
        {
            Serial.print("WIFI AutoConfig Waiting......");
            Serial.println(wstatus);
            delay(1000);
        }
    }
    Serial.println("WIFI AutoConfig Faild!" );
    return false;
}
