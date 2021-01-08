# ESP8266-NTP-Weather-Clock
esp8266-NTP-Weather-Clock  https://github.com/sunnysucd/ESP8266-NTP-Weather-Clock/
NTP-Weather-Max7219(8*32)-Clock

includes 2 libraries:          1. ntp  https://github.com/sunnysucd/NTPClient
                               2. weather https://github.com/taichi-maker/ESP8266-Seniverse


概要：使用乐鑫ESP-TOUCH一键配网，阿里时间服务器对时（可更换时间服务器），中途断网也可走时，约每1小时对时1次，增加了心知天气，可以了解今日天气（天气状况，当前温度，最低、高温度以及湿度）和未来3天的天气，并按光敏电阻传感器自动调节亮度，根据时间自动切换显示白天和晚上的天气状况。

一键配网请见  https://github.com/sunnysucd/esp8266-smartconfig  

内有安卓APP下载

https://www.ixigua.com/6898228517885084160?logTag=t2Sfp9yoIszi9MuPFxElF

硬件要求：ESP8266 MAX7219(8*32或8*48）可自行添加温湿度传感器以及物理按钮等

接口：
光敏电阻输入 A0
DIN  D7
CS   D8
CLK  D5

