
#include <DHT.h>
#include <DHT_U.h>

#include "WiFiEsp.h" 
// 블루투스 모듈로 와이파이 입력 및 plantid, plantInfoId, userid 입력

//suitablePalntData에서 적정 온습도 가져오기
#include <SoftwareSerial.h> 

#define rxPin 3 
#define txPin 2 

SoftwareSerial esp01(txPin, rxPin); // SoftwareSerial NAME(TX, RX); 
SoftwareSerial bluetooth(8, 9);  //블루투스 통신

int status = WL_IDLE_STATUS;       // the Wifi radio's status 

const char* host = "13.209.68.93"; 
const String tx_url = "/ubuntu/arduino/sensorData.php?lumi_r="; 
const String rx_url = "/ubuntu/arduino/suitablePlantData.php?plantid=";  

String wifi_id;
String wifi_pw;
String plantid;

WiFiEspClient client_rx; // WiFiEspClient 객체  
WiFiEspClient client_tx; // WiFiEspClient 객체  

const int ledPin_b = 5;         //LED와 연결된 핀 번호 저장하는 변수 선언
const int ledPin_g = 6;         //LED와 연결된 핀 번호 저장하는 변수 선언
const int ledPin_r = 11;         //LED와 연결된 핀 번호 저장하는 변수 선언
const int analog_2=A2;         //물수위센서 변수 선언
int level;
int motor = 10; //모터 릴레이 핀
int water = 0 ; // water 변수 선언
const int inputcds = A0;      //포토레지스터와 연결된 핀 번호 저장하는 변수 선언

String line = "";   //적정 정보 저장 변수 
bool flag = false;   //flag 

// ***************************************************
void printWifiStatus() {         // wifi 상태 표시 코드  
  Serial.print("SSID: ");        // print the SSID of the network you're attached to 
  Serial.println(WiFi.SSID()); 
  IPAddress ip = WiFi.localIP(); // print your WiFi shield's IP address 
  Serial.print("IP Address: "); 
  Serial.println(ip); 
  long rssi = WiFi.RSSI();       // wifi 신호 세기 
  Serial.print("Signal strength (RSSI):"); 
  Serial.print(rssi); 
  Serial.println(" dBm"); 
} 

// ****************************************************
void get_data(String p) { 
  if (client_rx.connect(host, 80)) { 
    Serial.println("Connected to server"); 
    client_rx.print("GET ");    // Make a HTTP request 
    client_rx.print(rx_url);  
    client_rx.print(p);         //plantid
    client_rx.print(" HTTP/1.1\r\n"); 
    client_rx.print("Host: "); 
    client_rx.print(host); 
    client_rx.print("\r\n"); 
    client_rx.print("Connection: close\r\n\r\n"); 
    flag = true; 
  } 
} 

// ***************************************************       "/ubuntu/arduino/sensorData.php?lumi=50&humi=50&sort=2&plantid=2"; 
void put_data(int w, int l, String p) {          //나중에 제거
  if (client_tx.connect(host, 80)) { 
    Serial.println("Connected to server"); 
    client_tx.print("GET ");    // Make a HTTP request 
    client_tx.print(tx_url);  
    client_tx.print(l);
    client_tx.print("&humi_r=");
    client_tx.print(w);
    client_tx.print("&plantid=");
    client_tx.print(p); //plantid
    client_tx.print(" HTTP/1.1\r\n"); 
    client_tx.print("Host: "); 
    client_tx.print(host); 
    client_tx.print("\r\n"); 
    client_tx.print("Connection: close\r\n\r\n"); 
    flag = true; 
  } 
} 
// ****************************************************
void setup() { 
  Serial.begin(9600);                 //시리얼모니터 
  bluetooth.begin(9600);
  
  pinMode(motor,OUTPUT);
  pinMode(ledPin_r, OUTPUT);
  pinMode(ledPin_g, OUTPUT);
  pinMode(ledPin_b, OUTPUT);
  pinMode(inputcds, INPUT);
  
} 

// *****************************************************
bool receive_data = false; 
int cnt =1; //데이터 입력 1시간 변수

String line_blue = "";
bool receive_data_blue = false; 

String humidity;
String luminance;

int main_flag =1;
int ble_flag=1;

// *******************************************************
void loop() { 
if (main_flag==1){
  while (bluetooth.available()){
    char c = bluetooth.read();
    if (c=='{') {
      receive_data_blue = true;    
    }
    else if (receive_data_blue == true) line_blue += c; 
    
  }
    if (line_blue != ""){
    Serial.print(line_blue);
    Serial.println(); 
    wifi_id = json_parser(line_blue, "w_id"); 
    Serial.print(F("wifi_id: ")); Serial.println(wifi_id); 
    wifi_pw = json_parser(line_blue, "w_pw"); 
    Serial.print(F("wifi_pw: ")); Serial.println(wifi_pw); 
    plantid = json_parser(line_blue, "p_id"); 
    Serial.print(F("plantid: ")); Serial.println(plantid); 
    Serial.println();
    
    line_blue="";
    delay(2000);
    ble_flag +=1;
    if (ble_flag==3){
      bluetooth.end();
    main_flag =2;
      }
    
  }
  
}
  if (main_flag==2){
  esp01.begin(9600);
  WiFi.init(&esp01);                  // initialize ESP module 
  while ( status != WL_CONNECTED) {   // attempt to connect to WiFi network 
    Serial.print("Attempting to connect to WPA SSID: "); 
    Serial.println(wifi_id); 
    //status = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network 원본
    char wi_id[wifi_id.length()+1];
    for (int x=0; x<sizeof(wi_id); x++){
      wi_id[x]=wifi_id[x];
      }
    char wi_pw[wifi_pw.length()+1];
    for (int x=0; x<sizeof(wi_pw); x++){
      wi_pw[x]=wifi_pw[x];
      }
    status = WiFi.begin(wi_id, wi_pw);  // Connect to WPA/WPA2 network 수정
  } 
  Serial.println("You're connected to the network"); 
  printWifiStatus();                  // wifi 상태표시 
  Serial.println(); 
  Serial.println("Starting connection to server..."); 
  delay(1000); 
  main_flag=3;
  }
  if(main_flag==3){
  get_data(plantid); 
  while (client_rx.available()) { 
    char c = client_rx.read(); 
    if(c == '{') receive_data = true; // { 데이터 저장 시작, '{'는 저장 안함 
    else if (receive_data == true) line += c; 
  } 
  if (flag == true){  
    if (!client_rx.connected()) {  //정보 수신 종료 되었으면  
      receive_data = false; 
      Serial.println(); 
      Serial.println(F("Disconnecting from server...")); 
      client_rx.stop(); 
      client_tx.stop();
      Serial.println(line); 
      Serial.println(); 
      humidity = json_parser(line, "humi"); 
      Serial.print(F("humidity: ")); Serial.println(humidity); 
      luminance = json_parser(line, "lumi"); 
      Serial.print(F("luminance: ")); Serial.println(luminance); 
      line = ""; 
    } 
  } 
  
// ****************************************************************토양습도 프로브를 이용한 모터조절
water=analogRead(A3);             //토양 습도 센서 아날로그핀 A3 번 사용 , A3 값 읽어와 water에 저장
Serial.print("Soil humidity: ");  //토양 습도 체크
Serial.println(water);
 if ( water > 1023 - humidity.toInt()*1023/100)   // 토양 습도 체크 적정습도 보다 낮으면 모터 동작 시켜 물공급 아니면 정지, (1023 - humidity.toInt()*1023/100)
   {
   digitalWrite(motor,HIGH);
   Serial.println("motor on");
 }
 else   {
   digitalWrite(motor,LOW);
   Serial.println("motor off");
 } 
Serial.println("**humidity**");      //설정한 습도
Serial.println(humidity.toInt()*1023/100);

// **********************************************************물수위센서
delay(500);
level=analogRead(analog_2);
Serial.println(level);
if(level<500){ //물 수위 조절해야함...
  Serial.println("low");
  digitalWrite(ledPin_r, HIGH);
  digitalWrite(ledPin_g, LOW);
  digitalWrite(ledPin_b, LOW);
 }

// ***********************************************************조도센서
int cds = analogRead(inputcds);
delay(2000);
Serial.print("INPUT CDS= ");          //측정한 조도
Serial.println(cds);
Serial.println("**luminance**");        //설정한 조도
Serial.println(luminance.toInt()*1023/100);
Serial.print("led_light= ");      //LED 조절 값
Serial.println(luminance.toInt()*255/100);

analogWrite(ledPin_r, luminance.toInt()*255/100);
analogWrite(ledPin_g, luminance.toInt()*255/100);
analogWrite(ledPin_b, luminance.toInt()*255/100);          //cds*100/1023 => 센서 조도

int temp_water = water /1023*100;
water = 100 - temp_water;
int temp_cds = cds / 1023 * 100;
cnt +=1; //추가
put_data(water, temp_cds, plantid);          //이게 필요 없을 거 같은데?

delay(1000);

} 
} //loop 끝

// ***********************************************************데이터 추출
String json_parser(String s, String a) {  
  String val; 
    int st_index = s.indexOf(a); 
    int val_index = s.indexOf(':', st_index); 
    if (s.charAt(val_index + 1) == '"') { 
      int ed_index = s.indexOf('"', val_index + 2); 
      val = s.substring(val_index + 2, ed_index); 
    } 
    else { 
      int ed_index = s.indexOf(',', val_index + 1); 
      val = s.substring(val_index + 1, ed_index); 
    } 
  return val; 
} 