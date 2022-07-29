///////////////////// RFID defines //////////////////////////

#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus
#define SS_PIN    21
#define RST_PIN   22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin     12
#define redPin       32
//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 
bool start;
//////////////////////////////////////////////////////////////

///////////////////// scale defines //////////////////////////
#define RXD2 16
#define TXD2 17
char c;
String readString;
float oldVal = 0;
//////////////////////////////////////////////////////////////

///////////////////// sound defines //////////////////////////
#include <SoftwareSerial.h>
enum{ZERO,ADD,REMOVE,AND};
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
static int8_t play_indx_song[] = {0x7e, 0x04, 0x42, 0x02, 101, 0xef};
bool first;
int unit = 1;
int goal = 1;
////////!///////////////////////////////////////////return

///////////////////// wifi defines //////////////////////////
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "AndroidAPE4BD"
#define WIFI_PASSWORD "lccm3605"

/* 2. Define the API Key */
#define API_KEY "AIzaSyAiE9hg0wrGLOPtctSv81nD_3mZnHOFWqI"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "aid-scale"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ahmad@1.com"
#define USER_PASSWORD "123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;
//////////////////////////////////////////////////////////////

///////////////////////Date & Time defines////////////////////////////
#include "time.h"

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

//////////////////////////////////////////////////////////////

String workerName;
String workerId;
String workerProduct;
String workerLanguage;

int start_time[3]={0};  
int curr_time[3]={0};  


void setup(){
  ///////////// RFID setup //////////////////////////
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  
  // Init MFRC522
  mfrc522.PCD_Init(); 
  Serial.println("Approach your reader card...");
  Serial.println();
  start = false;
  ///////////////////////////////////////////////////////////

  ///////////////////// scale setup //////////////////////////
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.println("serial2test");
  //////////////////////////////////////////////////////////////

  ///////////////////// sound setup //////////////////////////
  Serial1.begin(9600);
  send_command_to_MP3_player(select_SD_card, 5);
  //////////////////////////////////////////////////////////////
  
  ///////////////////// wifi setup //////////////////////////
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      
        digitalWrite(redPin,HIGH);
        digitalWrite(greenPin,HIGH);
        delay(300);
        digitalWrite(redPin,LOW);
        digitalWrite(greenPin,LOW);
        delay(300);
        Serial.print(".");
        //delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

  //////////////////////////////////////////////////////////////

  ///////////////////////Date & Time settup////////////////////////////
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  /////////////////////////////////////////////////////////////////////
  
  pinMode(3, INPUT_PULLUP);
  first = true;
  workerName = "";
  workerId = "";
  workerProduct = "";
  workerLanguage = "Arabic";
}

void loop() {
  
  if(digitalRead(3)==LOW){
      start = false;
      first = true;
      digitalWrite(redPin,LOW);
  }
  ///////////////////// RFID reed //////////////////////////
  if (mfrc522.PICC_IsNewCardPresent() && !start ) 
  {   
    start = true;
    if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    Serial.printf("readCard error");
  }else { }
  }else if(!start){ Serial.printf("readCard error");delay(1000);}
   if(!start) return;
  
  
  if(first){
    //////////////////////////////read rfid id///////////////////////////////////
    Serial.printf("rfid read: \n");
    workerId=readingData();
    Serial.printf(workerId.c_str());
    /////////////////////////////////////////////////////////////////////////////
    
    first = false;
    digitalWrite(redPin,HIGH);
    
     ////////////////////////read from firestore/////////////////////////////////
     Serial.printf("Doc read:\n");
     String documentPath = "Workers/"+workerId;
     Serial.println(documentPath);
     //String mask = "Name";
     if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(),"" )){ //mask.c_str()
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

            FirebaseJson payload;
            payload.setJsonData(fbdo.payload().c_str());

            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/Name/stringValue", true);
            Serial.println(jsonData.stringValue);
            workerName = jsonData.stringValue;
            payload.get(jsonData, "fields/Product/stringValue", true);
            Serial.println(jsonData.stringValue);
            workerProduct = jsonData.stringValue;
            payload.get(jsonData, "fields/Language/stringValue", true);
            Serial.println(jsonData.stringValue);
            workerLanguage = jsonData.stringValue;

            /////////////////////////////////// get product data /////////////////////////////
            documentPath = "Products/"+workerProduct;
            if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(),"" )){ //mask.c_str()
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

            FirebaseJson payload;
            payload.setJsonData(fbdo.payload().c_str());

            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/Unit/stringValue", true);
            Serial.println(jsonData.stringValue);
            unit = getNum(jsonData.stringValue);
            payload.get(jsonData, "fields/Pack/stringValue", true);
            Serial.println(jsonData.stringValue);
            goal = getNum(jsonData.stringValue);     
              Serial.println(unit);
              Serial.println(goal);     
        }
        else
            Serial.println(fbdo.errorReason());
            //////////////////////////////////////////////////////////////////////////////////
            
        }
        else
            Serial.println(fbdo.errorReason());
      ////////////////////////////////////////////////////////////////////////////
   sayIntro(workerLanguage);
   getTime(start_time);
  }
  //////////////////////////////////////////////////////////

  ///////////////////// scale read //////////////////////////

  while (Serial2.available()) {
    c = Serial2.read();
    readString += c;
  }
  if (readString.length() > 0 ) {
    float curr = getNum(readString);
    Serial.println(curr);
    Serial.println(oldVal);
    oldVal = oldVal * 0.50 + curr * 0.50;
    if( oldVal < curr + 0.01 && oldVal > curr - 0.01){
      if(curr > goal - unit/2 && curr < goal + unit/2){
        play_indx_song[3]=10;
        play_indx_song[4]=6;
        if(workerLanguage == "Hebrew") play_indx_song[4]=16;
        if(workerLanguage == "Russian") play_indx_song[4]=26;
        send_command_to_MP3_player(play_indx_song, 6);
        uploadData(curr);
        oldVal = -10;
        for(int j=0;j<20;j++){
          if(digitalRead(3)==LOW){
          start = false;
          first = true;
          digitalWrite(redPin,LOW);
          }
          delay(1000);   
        }
        play_indx_song[3]=10;
        play_indx_song[4]=7;
        if(workerLanguage == "Hebrew") play_indx_song[4]=17;
        if(workerLanguage == "Russian") play_indx_song[4]=27;
        send_command_to_MP3_player(play_indx_song, 6);
        getTime(start_time);
        delay(1500);
      }else{
      sayNum((goal- curr + unit/2)/unit,workerLanguage,false);
      play_indx_song[3]=10;
      play_indx_song[4]=5;
      if(workerLanguage == "Hebrew") play_indx_song[4]=15;
      if(workerLanguage == "Russian") play_indx_song[4]=25;
      send_command_to_MP3_player(play_indx_song, 6);
      oldVal = -10;
    delay(1000);
      }
    }
    readString = "";
  }
  //////////////////////////////////////////////////////////////

  //instructs the PICC when in the ACTIVE state to go to a "STOP" state
  mfrc522.PICC_HaltA(); 
  // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
  mfrc522.PCD_StopCrypto1();  
  delay(75);
}


void uploadData(float weight){
  if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0))
    {
        dataMillis = millis();
        count++;

        Serial.print("Commit a document (append array)... ");

        // The dyamic array of write object fb_esp_firestore_document_write_t.
        std::vector<struct fb_esp_firestore_document_write_t> writes;

        // A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t transform_write;

        // Set the write object write operation type.
        // fb_esp_firestore_document_write_type_update,
        // fb_esp_firestore_document_write_type_delete,
        // fb_esp_firestore_document_write_type_transform
        transform_write.type = fb_esp_firestore_document_write_type_transform;

        // Set the document path of document to write (transform)
        transform_write.document_transform.transform_document_path = "Jobs/"+getDate();

        // Set a transformation of a field of the document.
        struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

        // Set field path to write.
        field_transforms.fieldPath = "jobs";

        // Set the transformation type.
        // fb_esp_firestore_transform_type_set_to_server_value,
        // fb_esp_firestore_transform_type_increment,
        // fb_esp_firestore_transform_type_maaximum,
        // fb_esp_firestore_transform_type_minimum,
        // fb_esp_firestore_transform_type_append_missing_elements,
        // fb_esp_firestore_transform_type_remove_all_from_array
        field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;

        //String txt = "Hello World! " + String(count);
        content.set("values/[0]/mapValue/fields/Name/stringValue", workerName);
        content.set("values/[0]/mapValue/fields/ID/stringValue", workerId);
        content.set("values/[0]/mapValue/fields/SubmitTime/stringValue", getTime(curr_time));
        content.set("values/[0]/mapValue/fields/Tolerance/doubleValue", weight - goal);
        content.set("values/[0]/mapValue/fields/Weight/doubleValue", weight);
        content.set("values/[0]/mapValue/fields/WorkTime/doubleValue", getTimer(start_time,curr_time));
        content.set("values/[0]/mapValue/fields/ProductName/stringValue", workerProduct);
        
        // Set the transformation content.
        field_transforms.transform_content = content.raw();

        // Add a field transformation object to a write object.
        transform_write.document_transform.field_transforms.push_back(field_transforms);

        // Add a write object to a write array.
        writes.push_back(transform_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());
    }
}



float getNum(String string){
  int i=0;
  float sum=0;
  char first = string[0];
  while(string[i] > '9' || string[i] < '0'){
      i++;
  }
  while(string[i] <= '9' && string[i] >= '0'){
    sum = 10*sum + string[i] - '0';
    i++;
  }
  if(string[i]=='.'){
    sum = sum + (string[i+1]-'0')/10.0;
  }
  if(first == '-')sum*=-1;
  return sum;
}


void send_command_to_MP3_player(int8_t command[], int len){
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ Serial1.write(command[i]); Serial.print(command[i], HEX); }
  delay(1000);
}

void sayNum(int num,String language,bool justNum ){
  if(num > 0 && !justNum){
    sayWord(ADD , language);
  } else if(!justNum){
    sayWord(REMOVE ,language);
    num*=-1;
  }
  if(language == "Arabic") {play_indx_song[3]=1; sayArabic(num);}
  if(language == "Hebrew") {play_indx_song[3]=2; sayHebrew(num);}
  if(language == "Russian"){play_indx_song[3]=3; sayRussian(num);};
  /*switch (language){
    case "Arabic":
      play_indx_song[3]=1;
      sayArabic(num);
      break;
    case "Hebrew":
      play_indx_song[3]=2;
      sayHebrew(num);
      break;
    case "Russian":
      play_indx_song[3]=3;
      break;
  }*/
}

void sayArabic(int num){

  bool first = true;
  if(num / 100 > 0 ){
    play_indx_song[4]=100+num / 100;
    send_command_to_MP3_player(play_indx_song, 6);
    first = false;
    delay(250);
  }  
  num = num % 100;
  if(num / 10 == 1 ){
    if(first){
      play_indx_song[4]= num;
      send_command_to_MP3_player(play_indx_song, 6);
      delay(500);
      return;
    }else{
      play_indx_song[4]= num + 200;
      send_command_to_MP3_player(play_indx_song, 6);
      delay(500);
      return;
    }
  }
  if(num % 10 > 0 ){
    if(first){
      first = false;
      play_indx_song[4]=(num % 10);
      send_command_to_MP3_player(play_indx_song, 6);
      delay(250);
    }else{
      play_indx_song[4]=(num % 10 + 200);
      send_command_to_MP3_player(play_indx_song, 6);
      delay(250);
    }
  }
  if(num / 10  > 0 ){
    if(first){
      play_indx_song[4]=(num/10)*10;
      send_command_to_MP3_player(play_indx_song, 6);
      delay(250);
    }else{
      Serial.println((num/10) + 220);
      play_indx_song[4]=(num/10) + 220;
      send_command_to_MP3_player(play_indx_song, 6);
    delay(300);
    }
  }
}

void sayHebrew(int num){
  if(num < 10 && num > 0){
    play_indx_song[4]=num+20;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
    return;
  }
  if(num / 100 > 0 ){
    play_indx_song[4]=100+num / 100;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
  }  
  num = num % 100;
  if(num / 10 > 1 ){
    play_indx_song[4]=10 * (num / 10);
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
  }
  if(num / 10 == 1 ){
    play_indx_song[4]= num;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(500);
    return;
  }
}

  void sayRussian(int num){
//  if(num < 10 && num > 0){
//    play_indx_song[4]=num+20;
//    send_command_to_MP3_player(play_indx_song, 6);
//    delay(250);
//    return;
//  }
  if(num / 100 > 0 ){
    play_indx_song[4]=100+num / 100;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
  }  
  num = num % 100;
  if(num / 10 > 1 ){
    play_indx_song[4]=10 * (num / 10);
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
  }
  if(num / 10 == 1 ){
    play_indx_song[4]= num;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(500);
    return;
  }
  
  num = num % 10;
  if(num  > 0 ){
    //sayWord(AND,'R');
    play_indx_song[4]=num;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(250);
  }
}

void sayWord(int x,String language){
  if(language == "Arabic") x+=10;
  if(language == "Hebrew") x+=20;
  if(language == "Russian") x+=30;
 /* switch (language){
    case "Arabic":
      x+=10;
      break;
    case "Hebrew":
      x+=20;
      break;
    case "Russian":
      x+=30;
      break;
  }*/
  int temp3=play_indx_song[3];
  int temp4=play_indx_song[4];
  play_indx_song[3]=4;
  play_indx_song[4]=x;
  send_command_to_MP3_player(play_indx_song, 6);
  play_indx_song[3] = temp3;
  play_indx_song[4] = temp4;
  delay(200);
}



String readingData(){
  //prints the technical details of the card/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 
  
  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  //buffer for read data
  byte buffer[SIZE_BUFFER] = {0};
 
  //the block to operate
  byte block = 1;
  byte size = SIZE_BUFFER;//</p><p>  //authenticates the block to operate
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
//    digitalWrite(redPin, HIGH);
//    delay(1000);
//    digitalWrite(redPin, LOW);
    return "";
  }

  //read data from block
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
//    digitalWrite(redPin, HIGH);
//    delay(1000);
//    digitalWrite(redPin, LOW);
    return "";
  }
  else{
//      digitalWrite(greenPin, HIGH);
//      delay(1000);
//      digitalWrite(greenPin, LOW);
  }

//  Serial.print(F("\nData from block ["));
//  Serial.print(block);Serial.print(F("]: "));

 //prints read data
  for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
  {
//      Serial.write(buffer[i]);
  }
//  Serial.println(" ");

  String temp=String((char*)buffer+1);
  temp.remove(4);
  return temp;
}


void sayIntro(String language){
    int sound = 1;
    if(language == "Hebrew")  sound = 11;
    if(language == "Russian") sound = 21;
    play_indx_song[3]=10;
    play_indx_song[4]= sound++;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(2600);
    play_indx_song[4]= sound++;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(500);
    play_indx_song[4]= sound++;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(3000);
    //play_indx_song[3]=2;
    //if(language == "Arabic") 
    sayNum(goal,language,true);
    //if(language == "Hebrew") sayHebrew(goal);
    delay(300);
    play_indx_song[3]=10;
    play_indx_song[4]= sound++;
    send_command_to_MP3_player(play_indx_song, 6);
    delay(500);
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

int getTimer(int* start_time,int* end_time) {
  int timer_in_seconds=0;
  timer_in_seconds += 3600*(end_time[0]-start_time[0]); //hours
  timer_in_seconds += 60*(end_time[1]-start_time[1]); //minutes
  timer_in_seconds += (end_time[2]-start_time[2]); //seconds
  
  return timer_in_seconds;
}
