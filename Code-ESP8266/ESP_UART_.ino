#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define WIFIMANAGER_ENABLED

//#include "SPISlave.h"
//#include "SPICalls.h"
//#include "WiFiSPICmd.h"

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
//#include <Ticker.h>                       

#include <String.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include<SoftwareSerial.h>
#include <math.h>

#define ESP_CMD_HEADER 0x69
#define ESP_START_SEND_DATA 0x01
#define ESP_REQUEST_SEND_DATA 0x02
#define ESP_RESPONE    0x70
#define ESP_RESEND_DATA    0x03
#define ESP_RESEND_COMMAND    0x04
/*
   Cmd Struct Message:
   __________________________________________________________________________________
  | HEADER    | PARAM LEN |    CMD     |   DATA  |   DATA    |  DATA  | .. |CHECKSUM |
  |___________|___________|____________|_________|___________|________|____|_________|
  |   8 bit   |   8 bit   | 8 bit      |  8bit   |   8bit    | nbytes | .. |   8bit  |
  |___________|___________|____________|_________|___________|________|____|_________|
*/
/*-----------------------------------------------------------------------------------------------------*/


const uint8_t SS_ENABLE_PIN = 5;  // PIN for circuit blocking SS to GPIO15 on reset 

//Ticker ticker;  // for status LED

/*-----------------------------------------------------------------------------------------------------*/
#define D1 (4)
#define D2 (5)
constexpr int IUTBITRATE = 115200;

constexpr int BLOCKSIZE = 16; // use fractions of 256
//constexpr int ReportInterval = IUTBITRATE / 8;
bool uart_receive_complete = false;
SoftwareSerial logger;


/*-----------------------------------------------------------------------------------------------------*/
//void tick()
//{
//    //toggle state
//    int state = digitalRead(LED_BUILTIN);  // get the current state of LED
//    digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
//}

// Gets called when WiFiManager enters configuration mode
/*-----------------------------------------------------------------------------------------------------*/
void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
  //  if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
//     entered config mode, make led toggle faster
//    ticker.attach(1, tick);
}
/*-----------------------------------------------------------------------------------------------------*/

WiFiClient espClient;
PubSubClient client(espClient);
/*-----------------------------------------------------------------------------------------------------*/
const char* mqtt_server = "test.mosquitto.org";  //localhost

//const char* mqtt_server = "tailor.cloudmqtt.com";
char msg[50];
String clientId = "ESP8266Client-" + String(ESP.getChipId(), HEX);

String subscribe_broker = "/server-request/" + String(ESP.getChipId());


    
void reconnect() {

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect with user name + pass
   // if (client.connect(clientId.c_str(), "gaxlylcj","YYhH75v1oDzx")) {
    if (client.connect(clientId.c_str())) {       //test 
      Serial.print("connected ");
      //  resubscribe
      client.subscribe(subscribe_broker.c_str());
      Serial.println(subscribe_broker.c_str());

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);

    }
  }
}
/*-----------------------------------------------------------------------------------------------------*/
uint8_t _CalculateChecksum(uint8_t *buffer, int length)
{
  uint16_t sum = 0;
  int i;
  uint8_t kq;
  for (i = 0; i < length; i++)
    sum += buffer[i];
  kq= (65536 - sum) % 256;
  return kq; 
}

void callback(char* topic, byte* payload, unsigned int length) {
 uint8_t serverrequestdata[4];
  serverrequestdata[0] = 0x40;//header 9
  serverrequestdata[1] = 0x00;//datalen = 0, no data, chu o
  serverrequestdata[2] = ESP_REQUEST_SEND_DATA;// command
  serverrequestdata[3] = _CalculateChecksum(serverrequestdata,3);
  
  uint8_t data_mqtt[2];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data_mqtt[i] = payload[i];
  }
  Serial.println();
  //stop uart
   uart_receive_complete = false;
    
  // Switch on the LED if an 1 was received as first character
  if ((char)data_mqtt[0] == '8' && (char)data_mqtt[1] == '9') {

    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    logger.enableTx(true);
//    logger.write(serverrequestdata, 4);
//    logger.enableTx(false);

    //upload data da luu len server
    upload_server_data();
    delay(20);
    //Serial.println("Send request to MCU");
    data_mqtt[0] = 0;
    data_mqtt[1] = 0;
    digitalWrite(BUILTIN_LED, HIGH);
  } else {
      digitalWrite(BUILTIN_LED, LOW);
      delay(400);
      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
/*-----------------------------------------------------------------------------------------------------*/
// tinh aqi
//tim min
uint16_t tim_min(uint16_t array_in_min[], uint8_t len_min){
  uint16_t temp = array_in_min[0];
  for(int i = 1 ;  i < len_min; i++){
    if(array_in_min[i] < temp)
        temp = array_in_min[i];
  }
  return temp;
}
//tim max
uint16_t tim_max(uint16_t array_in_max[], uint8_t len_max){
  uint16_t temp = array_in_max[0];
  for(int i = 1 ;  i < len_max; i++){
    if(array_in_max[i] > temp)
        temp = array_in_max[i];
  }
  return temp;
}
//he so w

float he_so_w(uint16_t array_in[]){
  uint16_t min = tim_min(array_in,12);
  uint16_t max = tim_max(array_in,12);
   float w;
  if(max == 0) 
    w = 0;
  else
    w = min/max;
  if(w > 0.5)
  {
    return w;
  }
  else
  {
    return w = 0.5;
  }
  //Serial.print("he so w");
 // Serial.println(w, 2);
 }

//he so nowcast
float he_so_nowcast( uint16_t array_in[]){
  float heso_w = he_so_w(array_in);
  float tu_so =0, mau_so = 0, kq;
  
  if(heso_w > 0.5){
    for(int i = 0; i < 12; i ++){
      tu_so += pow(heso_w, (i+1))*array_in[i+1];
      mau_so += pow(heso_w,(i+1));
    }
    Serial.println("he so nw1");
    Serial.println(kq,2);
    if(mau_so == 0)
      return kq = 0;
    else
      return kq = tu_so/mau_so;
  }
  else{
    for(int i = 0; i < 12; i ++){
      tu_so += pow(0.5, (i+1))*array_in[i];
      
    }
    Serial.println("he so nw");
    Serial.println(tu_so,3);
    return tu_so;
  }
}
//bang co so
float bang_co_so_o3[8] = {0, 160,200,300,400,800,1000,1200};
float bang_co_so_voc[8] = {0,125,350,550,800,1600,2100,2630};
float bang_co_so_pm10[8] = {0,50,150,250,350,420,500,600};
float bang_co_so_pm25[8] = {0,25,50,80,150,250,350,500};
float bang_co_so_i[8] = {0, 50,100,150,200,300,400,500};
uint8_t data_aqi[12][4];
uint8_t data_aqi_len = 0;

uint16_t aqi_dust_cal(uint16_t arrayin[], int which_dust){
  //uint16_t hs_nc =  (uint16_t) he_so_nowcast(arrayin);
  float hs_nc =   he_so_nowcast(arrayin);
  uint8_t vi_tri = 0;
  float array_co_so[8];
  if(which_dust == 1)
    memcpy(array_co_so, bang_co_so_pm10, sizeof(array_co_so));
  else
  {
    memcpy(array_co_so, bang_co_so_pm25, sizeof(array_co_so));
  }
  
  for (int i = 0; i < 8; i++)
  {
    if (hs_nc > array_co_so[i])
        vi_tri = i;
  }
  float tuso = (bang_co_so_i[vi_tri+1] - bang_co_so_i[vi_tri])*(hs_nc - array_co_so[vi_tri] );
  float mauso = array_co_so[vi_tri+1] -array_co_so[vi_tri] ;
  uint16_t kq ;
  if(mauso == 0)
      kq = 0;
   else
      kq = (uint16_t)((tuso/mauso) + bang_co_so_i[vi_tri]);

  return kq;
}


uint16_t aqi_ozon_tvoc_cal(uint16_t datain, int which_element){
  uint8_t vi_tri = 0;
    float array_co_so[8];
  if(which_element == 1)
    memcpy(array_co_so, bang_co_so_o3, sizeof(array_co_so));
  else
  {
    memcpy(array_co_so, bang_co_so_voc, sizeof(array_co_so));
  }
  
  for (int i = 0; i < 8; i++)
  {
    if (datain > array_co_so[i])
        vi_tri = i;
  }
  float tuso = (bang_co_so_i[vi_tri+1] - bang_co_so_i[vi_tri])*(datain -array_co_so[vi_tri] );
  float mauso = array_co_so[vi_tri+1] -array_co_so[vi_tri] ;
  uint16_t kq;
  if(mauso == 0)
    kq = 0;
  else
    kq = (uint16_t)((tuso/mauso)+bang_co_so_i[vi_tri]);

  return kq;
}



uint16_t aqi_final(uint8_t datain[12][4], uint8_t data_cal_aqi[]){

union tam_to_muoisau{
    uint8_t tam[2];
    uint16_t muoisau;
    }data_16;
uint16_t  array_each_element[12];
uint16_t aqi[4];

//tinh aqi cho bui pm10 
  
    for(int j = 0; j <12; j++){
      data_16.tam[0] = datain[j][0];
      data_16.tam[1] = datain[j][1];
      array_each_element[j] = data_16.muoisau;
        Serial.print(array_each_element[j]);
        Serial.print(" ");
    }
      aqi[0] = aqi_dust_cal(array_each_element, 1);
  Serial.print("kq aqi bui:");
  Serial.println( aqi[0]);
//tinh aqi cho bui pm2.5
    for(int j = 0; j <12; j++){
      data_16.tam[0] = datain[j][2];
      data_16.tam[1] = datain[j][3];
      array_each_element[j] = data_16.muoisau;
       Serial.print(array_each_element[j]);
        Serial.print(" ");
    }
      aqi[1] = aqi_dust_cal(array_each_element, 0); 
  
  Serial.print("kq aqi bui 25:");
  Serial.println( aqi[1]);
// aqi cho tvoc
      data_16.tam[0] = data_cal_aqi[6];
      data_16.tam[1] = data_cal_aqi[7];
      uint16_t tvoc_val = data_16.muoisau;
      aqi[2] = aqi_ozon_tvoc_cal(tvoc_val, 0);
  Serial.print("kq aqi tvoc:");
  Serial.println( aqi[2]);
//aqi cho ozone   
      data_16.tam[0] = data_cal_aqi[12];
      data_16.tam[1] = data_cal_aqi[13];
      uint16_t ozone_val = data_16.muoisau;
     aqi[3] = aqi_ozon_tvoc_cal(ozone_val, 1);
  Serial.print("kq aqi ozone:");
  Serial.println( aqi[3]);
//tinh aqi cuoi cung
    uint16_t temp_aqi = aqi[0];
    for(int k = 1; k <  4; k++)
    {
        if(aqi[k] > temp_aqi)
            temp_aqi = aqi[k];

    }
    Serial.print("aqi = ");
    Serial.println(temp_aqi);
  return temp_aqi;
}
/*-----------------------------------------------------------------------------------------------------*/
int row =0; 
uint16_t data_save_1hour[24][8];
void luu_data_30phut( uint8_t in_data[], uint16_t aqi_fin){

union tam_to_muoisau{
    uint8_t tam[2];
    uint16_t muoisau;
    }data_16;    
      /*-----------------------------------------------------*/
      //dust 25
      data_16.tam[0] = in_data[0];
      data_16.tam[1] = in_data[1];
    if(data_16.muoisau <1000)
         data_save_1hour[row][0] = data_16.muoisau;   
    else
        data_save_1hour[row][0] = 0;
       /*-----------------------------------------------------*/
      //dust 10
      data_16.tam[0] = in_data[2];
      data_16.tam[1] = in_data[3];
    if(data_16.muoisau <1000)
         data_save_1hour[row][1] = data_16.muoisau;   
    else
        data_save_1hour[row][1] = 0;
     
       /*-----------------------------------------------------*/
       //co2
      for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[4+i];
     }
     if(data_16.muoisau < 60000)
         data_save_1hour[row][2] = data_16.muoisau;   
    else
        data_save_1hour[row][2] = 400;
     
      /*-----------------------------------------------------*/
      //tvoc
    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[6+i];
    }
     if(data_16.muoisau < 60000)
         data_save_1hour[row][3] = data_16.muoisau;   
    else
        data_save_1hour[row][3] = 0;
     /*-----------------------------------------------------*/
    //temp
   for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[8+i];
    }
    if(data_16.muoisau <101)
         data_save_1hour[row][4] = data_16.muoisau;   
    else
        data_save_1hour[row][4] = 20;

     /*-----------------------------------------------------*/
    //humi
    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[10+i];
    }
     if(data_16.muoisau <101)
         data_save_1hour[row][5] = data_16.muoisau;   
    else
        data_save_1hour[row][5] = 50;
    
     /*-----------------------------------------------------*/
    //0zone
    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[12+i];
    }
    if(data_16.muoisau <1000)
        data_save_1hour[row][6] = data_16.muoisau;   
    else
        data_save_1hour[row][6] = 10;
   /*-----------------------------------------------------*/
   if(aqi_fin <300)
        data_save_1hour[row][7] = aqi_fin; 
    else
        data_save_1hour[row][7] = 10 ;
    
  row++;
  if(row > 23)
    row = 0;
}
/*-----------------------------------------------------------------------------------------------------*/
void upload_server_data(void)
{
  uint16_t total_temp =0;
  uint16_t total;
  uint16_t datajson[8];
  for(int i = 0; i < 8; i ++)
  {
    for(int j = 0; j < 24; j++)
    {
       total_temp += data_save_1hour[j][i] ;
      }
     datajson[i] = total_temp/24;
     total_temp =0;
   }

    buildJson(datajson);
    
 }
/*-----------------------------------------------------------------------------------------------------*/
 void buildJson(uint16_t in_data[]) {

    uint32_t mqtt_chip_key = ESP.getChipId();
    char eclientid[25];
    snprintf(eclientid,25,"%06lu",mqtt_chip_key);
    
    String publish_broker = "/divice-connect-data/" + String(ESP.getChipId());

    StaticJsonDocument<256> doc;   

      doc["pm25"] = in_data[0]; 
      doc["pm10"] = in_data[1];
      doc["co2"] =  in_data[2]; 
      doc["tvoc"] = in_data[3]; 
      doc["temp"] = in_data[4];
      doc["humd"] = in_data[5]; 
      doc["o3"] = in_data[6]; 
      doc["time"] = 0;
      doc["aqi"] = in_data[7];
      doc["id"] = eclientid; 
    
    char buffer[512];
    size_t n = serializeJson(doc, buffer);
   // Serial.println(n);
   if( client.publish(publish_broker.c_str(), buffer, n) == true)
   // client.publish("/divice-connect-data", buffer, n);
    { 
      Serial.print("Send data to mqtt success" );
     // Serial.println(buffer);
    }
    else
      Serial.print("can not send to mqtt");
    delay(100);

}
/*-----------------------------------------------------------------------------------------------------*/
void buildJson_test(uint8_t in_data[], uint16_t aqi_fin) {
   union tam_to_muoisau{
    uint8_t tam[2];
    uint16_t muoisau;
    }data_16;

    uint32_t mqtt_chip_key = ESP.getChipId();
    char eclientid[25];
    snprintf(eclientid,25,"%06lu",mqtt_chip_key);
    
    String publish_broker = "/divice-connect-data/" + String(ESP.getChipId());
    
//    char aqi_mq[20];
//    snprintf(aqi_mq,20,"%04u",aqi_fin);
    
 
    StaticJsonDocument<256> doc;   
      data_16.tam[0] = in_data[0];
      data_16.tam[1] = in_data[1];
      doc["pm25"] = data_16.muoisau; 

      data_16.tam[0] = in_data[2];
      data_16.tam[1] = in_data[3];
       doc["pm10"] = data_16.muoisau;
   
      for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[4+i];
     }
     doc["co2"] =  data_16.muoisau; 

    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[6+i];
    }
     doc["tvoc"] = data_16.muoisau; 

   for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[8+i];
    }
   doc["temp"] = data_16.muoisau;
    
    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[10+i];
    }
   doc["humd"] = data_16.muoisau; 

    for(int i = 0; i < 2; i ++){
      data_16.tam[i] = in_data[12+i];
    }
   doc["o3"] = data_16.muoisau; 
   doc["time"] = 0;
   doc["aqi"] = aqi_fin;
   doc["id"] = eclientid; 
    
    char buffer[512];
    size_t n = serializeJson(doc, buffer);
   // Serial.println(n);
  // if( client.publish(publish_broker.c_str(), buffer, n) == true)
   if( client.publish("/divice-connect-data", buffer, n) == true)
    { 
      Serial.print("Send data to mqtt success" );
     // Serial.println(buffer);
    }
    else
      Serial.print("can not send to mqtt");
    delay(100);

}

/*-----------------------------------------------------------------------------------------------------*/
int request_time = 0;
void xulydata(uint8_t datain[]){
  uint8_t indata[32];
  uint8_t ok[4] ;
     ok[0] = 0x40;
     ok[1] = 0x00;//data lend
     ok[2] = ESP_RESPONE;
     ok[3] = _CalculateChecksum(ok,3);
   uint8_t resend[4] ;
     resend[0] = 0x40;
     resend[1] = 0x00;
     resend[2] = ESP_RESEND_DATA;
     resend[3] = _CalculateChecksum(resend,3);
  if(datain[0] == 0){
      Serial.println("Zero data");
     return;
    }

  else if(datain[0] == ESP_CMD_HEADER)
  {

    //kt nhan du byte hay khong
    uint8_t datalen = datain[1];
    uint8_t receivedChecksum = datain[3 + datalen];
    int expectedChecksum = _CalculateChecksum(datain, 3 + datalen);
 
    if (receivedChecksum != expectedChecksum){
              request_time++;
        if(request_time > 5)
        {
          request_time = 0;
           return;
        }
       
        logger.enableTx(true);
        logger.write(resend, 4);//resend
        logger.enableTx(false);
        Serial.println("Receive lost packet, request mcu resend");      
    }
    else{
          switch(datain[2]){
            case ESP_RESPONE: {
                   Serial.println("Process data from MCU: Check connect, send ""ok"" back");  
                  // SPISlave.setData(ok);
                  logger.enableTx(true);
                  logger.write(ok, 4);
                  logger.enableTx(false);
                  delay(100);
                  break;
              }
            case ESP_START_SEND_DATA:{
                  uint8_t len = datain[1];
                  Serial.println("Process data from MCU:Received data from MCU");
                  for(int i = 0; i < len; i ++)
                  {
                    indata[i] = datain[3+i];
                    if(i < 4)
                     data_aqi[data_aqi_len][i] =  datain[3+i];
                  }
                  data_aqi_len++;
                  if(data_aqi_len >11)
                    data_aqi_len = 0;
                
                    //tinh aqi
                   uint16_t aqi_fin = aqi_final(data_aqi,indata);
                   luu_data_30phut(indata,aqi_fin);
                  //tao file json rồi up lên web           
                  buildJson_test(indata,aqi_fin );
                  break;
            }
            case ESP_RESEND_COMMAND:{
              uint8_t serverrequestdata[4];
                serverrequestdata[0] = 0x40;//header 9
                serverrequestdata[1] = 0x00;//datalen = 0, no data, chu o
                serverrequestdata[2] = ESP_REQUEST_SEND_DATA;// command
                serverrequestdata[3] = _CalculateChecksum(serverrequestdata,3);
                  logger.enableTx(true);
                  logger.write(serverrequestdata, 4);
                  logger.enableTx(false);
                
              }
           default: 
               Serial.println("something wrong");  
               break;
          }
    }
     Serial.println(receivedChecksum);
     Serial.println(expectedChecksum);
  }
  
  else{
     Serial.println("Process data from MCU: Wrong header");
     logger.enableTx(true);
     logger.write(resend, 4);
     logger.enableTx(false);
     delay(100);
   }
 }
/*-----------------------------------------------------------------------------------------------------*/
//consexptr SoftwareSerialConfig swSerialConfig = SWSERIAL_8N1;
void setup()
{
   
  Serial.begin(115200);
 // Serial.swap();
 // Serial.setRxBufferSize(4 * BLOCKSIZE);
  logger.begin(9600,SWSERIAL_8N1, D1, D2, false, 4 * BLOCKSIZE, 4 * BLOCKSIZE);
   
//    WiFiSpiEspCommandProcessor::init();
     pinMode(2, OUTPUT);
     digitalWrite(LED_BUILTIN, HIGH); // turn led off
    // start ticker with 0.15 because we start in AP mode and try to connect
//     ticker.attach(0.15, tick);
    Serial.setDebugOutput(true);
 
    WiFi.mode(WIFI_OFF);  // The Wifi is started either by the WifiManager or by user invoking "begin"  
    pinMode(SS_ENABLE_PIN, OUTPUT);
    digitalWrite(SS_ENABLE_PIN, HIGH);  // enable SS signal to GPIO15 (https://github.com/JiriBilek/WiFiSpiESP/issues/6)
    WiFi.persistent(true);
    Serial.println(F("WifiManager enabled."));

    WiFiManager wifiManager;
  // set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
    // connect
    wifiManager.autoConnect();
    // connected successfully
//    ticker.detach();
    digitalWrite(LED_BUILTIN, HIGH); // turn led off 
    
  //client.setServer(mqtt_server, 15587); //cloudmqtt
 client.setServer(mqtt_server, 1883);   //mosquito
  client.setCallback(callback);
}

/*-----------------------------------------------------------------------------------------------------*/
uint8_t dataBuf[32];  // copy of receiver buffer 
uint8_t data_process[32];
uint8_t length_databuf = 0;

/*-----------------------------------------------------------------------------------------------------*/

void loop() {
   if (!client.connected()) {
       reconnect();   
   }
  
/*-----------------------------------------------------------------------------------------------------*/
//nhan du lieu tu stm32 thong  qua uart
uint8_t recieve_uart;

while(logger.available() > 0){
        dataBuf[length_databuf++] = logger.read();  
        delay(5);
        uart_receive_complete = true;
}

if(uart_receive_complete){           
        //delay(100);
        uint8_t lengh2 =length_databuf;
        for(int i = 0; i < lengh2; i ++)
        {
          data_process[i] = dataBuf[i];
        }
        //XU LY DATA O DAY
        xulydata(data_process);
        Serial.println("data nhan duoc tu MCU: "); 
        for(int i = 0; i < lengh2; i++)
       {       
            Serial.print(dataBuf[i], HEX);   
            Serial.print(" ");
            data_process[i] = 0;     
            dataBuf[i] = 0;  
        }
        Serial.println();
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
     
        uart_receive_complete = false;
        length_databuf = 0;
    }
     // length_databuf = 0;
//uart_xuly();

  /*-------------------------------------------------------------------*/
  /*-------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
//nhận từ http request 1 tiếng 1 lần
 client.loop();
/*-----------------------------------------------------------------------------------------------------*/
//gui qua stm32
}
