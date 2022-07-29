/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-date-time-ntp-client-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include "time.h"

const char* ssid     = "Sofian - iPhone";
const char* password = "sofian98";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

typedef enum {
    my_WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    my_WL_IDLE_STATUS      = 0,
    my_WL_NO_SSID_AVAIL    = 1,
    my_WL_SCAN_COMPLETED   = 2,
    my_WL_CONNECTED        = 3,
    my_WL_CONNECT_FAILED   = 4,
    my_WL_CONNECTION_LOST  = 5,
    my_WL_DISCONNECTED     = 6
} my_internet_status;


void setup(){
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //disconnect WiFi as it's no longer needed
//  WiFi.disconnect(true);
//  WiFi.mode(WIFI_OFF);
}

void loop(){
  delay(1000);
  String date_str = getDate();
  int start_time[3]={0};
  String time_str1 = getTime(start_time);
  delay(63000);
  int end_time[3]={0};
  String time_str2 = getTime(end_time);
  int timer = getTimer(start_time,end_time);
  Serial.println(timer);
  Serial.println(date_str);
  Serial.println(time_str1);
  Serial.println(time_str2);
  Serial.println(WiFi.status());
}

String getDate(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");


  uint8_t day = timeinfo.tm_mday;
  String dayStr = day < 10 ? "0" + String(day) : String(day);
   
  uint8_t month = timeinfo.tm_mon + 1;
  String monthStr = month < 10 ? "0" + String(month) : String(month);
  
  uint16_t year = timeinfo.tm_year + 1900;
  String yearStr = String(year);

  return dayStr + "." + monthStr + "." + yearStr;
}

int getTimer(int* start_time,int* end_time) {
  int timer_in_seconds=0;
  timer_in_seconds += 3600*(end_time[0]-start_time[0]); //hours
  timer_in_seconds += 60*(end_time[1]-start_time[1]); //minutes
  timer_in_seconds += (end_time[2]-start_time[2]); //seconds
  
  return timer_in_seconds;
}

int stringToNum(String str) {
  int num=0,len=0;
  while(str[len]!='\0'){
    len++;
  }
  if(len==2) {
      num += (str[0]-'0')*10;
      num += (str[1]-'0');
  } else {
      num+=(str[0]-'0');
  }
  return num;
}

String getTime(int* time_arr) {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  String hour_str = String(timeinfo.tm_hour);
  time_arr[0]=stringToNum(hour_str);
  String minute_str = String(timeinfo.tm_min);
  time_arr[1]=stringToNum(minute_str);
  String sec_str = String(timeinfo.tm_sec);
  time_arr[2]=stringToNum(sec_str);
  
  return hour_str + ":" + minute_str + ":" + sec_str;
  
}
