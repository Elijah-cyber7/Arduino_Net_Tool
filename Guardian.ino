#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttons = A0;
int buttonValues;
int x = 0;

// Screen Tracking Variables *************************************
#define minRow       0              /* default =   0 */
#define maxRow     127              /* default = 127 */
#define minLine      0              /* default =   0 */
#define maxLine     63  



// Variables for Second Screen 
#define maxChan 11
#define standardLine 0
unsigned long startTime = 0;
unsigned long curTime = 0;
unsigned long packets = 0;
unsigned long deauths = 0;
int chan = 1;
unsigned long maxValue = 0;
double multiplier = 0.0;
unsigned int val[128];
unsigned int networks = 0;


//**************************************************

bool exited = false;
long just_left = 0;
char* menu_options[] = {"Scan","Packet_Monitor","Deauth"};
int text_locations[] = {10,30,50};



void setup()
{

  pinMode(buttons, INPUT);
  Serial.begin(9600);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(300);
  display.clearDisplay();
  display.fillRect(0,x,128,20, WHITE);
  display.display();
}

// **************CallBackFunc****************

void callback(uint8_t *buff, uint16_t type){
  packets++;
  if ( buff[12] == 0xA0 || buff[12] == 0xC0){
    deauths++;
  }
}
/// *** channel func 
void chanReset(){
  for(int i = 0; i < maxRow; i++){
    val[i] = 0;  
  }
  packets = 0;
  multiplier = 1.0;
 
  
}
/// **************************Reading Packets
void readPackets(){
  if(curTime - startTime >= 500){
    startTime = curTime;
    for(int i = 0; i < maxRow; i++){
      val[i] = val[i+1];
    }
    val[127] = packets;
    if( val[127] > maxValue) maxValue = val[127];
    if(maxValue > 47) multiplier = 47.00/(double) maxValue;
    else multiplier = 1.0;
    displayTraffic();
  }
  
   
}
//******************************************************Deauthentication Vars *************




void setAddresses(uint8_t payload[26], uint8_t source[6], uint8_t dest[6]){
  memcpy(&payload, deathPacket, 26); 
  memcpy(&payload[4], dest, 6);
  memcpy(&payload[10], source, 6);
  memcpy(&payload[16], source, 6);
  payload[24] = 1;
}


void displayTraffic(){
  display.clearDisplay();
  display.drawLine(minRow, 12, maxRow, 12, WHITE);
  display.setCursor(0, standardLine);
  display.print("Ch:");
  display.setCursor(30, standardLine);
  display.print("Pkts:");
  display.setCursor(80, standardLine);
  display.print(String(packets));
  display.setCursor(20, standardLine);
  display.print(String(chan));
  for (int i = 0; i < maxRow; i++) display.drawLine(i, maxLine, i, maxLine - val[i]*multiplier, WHITE);
  display.display();
  packets = 0;
}

int readButtons() {
  buttonValues = analogRead(buttons);
  if (buttonValues > 300 && buttonValues < 400) {
    return 3;
    delay(150);
  }
  if (buttonValues > 1000 ) {
    return 1;
     delay(150);
  }
  if (buttonValues > 700 && 800 > buttonValues) {
    return 2;
    delay(150);
  }
  return 0;
}

void loop() {
  
  moveSelector();
  

}

void moveSelector() {
   mainScreen();
  if((millis() - just_left) > 1000 && readButtons() == 2){
    exited = false;
    check_selected(x);
    //delay(150);
  }
  if (readButtons() == 1 && x > 0) {
    x -= 20;
    display.clearDisplay();
    delay(150);
  }
  if (readButtons() == 3 && x < 40) {
    display.clearDisplay();
    x += 20;
    delay(150);
  }
  else if(readButtons() == 3 && x >= 40){
    display.clearDisplay();
    x = 0;
    delay(150);
  }
  else if(readButtons() == 1 && x <= 0){
    display.clearDisplay();
    x = 40;
    delay(150);
  }
}
void moveSelectorr() {
   if((millis() - just_left) > 1000 && readButtons() == 2){
    exited = false;
    check_selected(x);
    //delay(150);
  }
  if (readButtons() == 1 && x > 0) {
    x -= 6;
    display.clearDisplay();
    delay(150);
  }
  if (readButtons() == 3 && x < 40) {
    display.clearDisplay();
    x += 6;
    delay(150);
  }
  else if(readButtons() == 3 && x >= 40){
    display.clearDisplay();
    x = 6;
    delay(150);
  }
  else if(readButtons() == 1 && x <= 0){
    display.clearDisplay();
    x = 6;
    delay(150);
  }
}
void mainScreen(){
  display.drawFastHLine(0, 0, 128, WHITE);
  display.drawFastHLine(0, 20, 128, WHITE);
  display.drawFastHLine(0, 40, 128, WHITE);
  display.drawFastHLine(0, 60, 128, WHITE);
  display.fillRect(0, x, 128, 20, WHITE);
  check_location();
  display.display();
}
void check_location(){
 for(int i = 0; i < 3;i++){
   if(x == text_locations[i]-10){
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(2, text_locations[i]);
    display.println(String(menu_options[i])+x);
 }
 else{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(2, text_locations[i]);
    display.println(String(menu_options[i])+x);
 }
}
}

void check_selected(int z){
   String screen;
   for(int i = 0; i < 3; i++){
    if((z + 10) == text_locations[i]){
      screen = menu_options[i];
      break;
    }
   }
   if(screen.equals(menu_options[0])){
     displayfirstscreen();
   }
   else if(screen.equals(menu_options[1])){
    displaySecondScreen();
   }
}
void displayfirstscreen(){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("Wifi Scanner");
      display.setCursor(0,20);
      display.print("Elijah Cyber");
      display.display();
      delay(2000);   
      display.setCursor(0, 0);
      display.setTextSize(.5);
      display.setTextColor(WHITE);
      display.clearDisplay();
      WiFi.scanDelete();
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      delay(100);
      networks = WiFi.scanNetworks();
      printNetworks(networks);     
    while(!exited){
          //moveSelectorr();
          display.display();
          exitedd();
    }
    delay(200);
    display.clearDisplay();
    
    return;
}
//*********************************************Packet Sniffer*******************
void displaySecondScreen(){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.print("Packet-Monitor");
      display.setCursor(0,20);
      display.print("Elijah Cyber");
      display.display();
      delay(2000);
     while(!exited){
      curTime = millis();
     
      ////////////////////// WiFi SetUp/////////////////
      wifi_set_opmode(STATION_MODE);
      wifi_promiscuous_enable(0);
      WiFi.disconnect();
      wifi_set_promiscuous_rx_cb(callback);
      wifi_set_channel(chan);
      wifi_promiscuous_enable(1);

      readPackets();
      ////Getting Button Inputs////
      if(readButtons() == 1 && chan < maxChan){
        chan++;
        wifi_set_channel(chan);
        chanReset();
        
        delay(150);
      }
      else if (readButtons()==3 && chan > 0 ){
      chan--;
      wifi_set_channel(chan);
      chanReset();
      delay(150);
      }
      //readPackets();
      //displayTraffic();
      display.display();
      exitedd();
      }                                          
    delay(200);
    packets = 0;
    multiplier = 1.0;
    chan = 0;
    display.clearDisplay();
    
    return;
}

void exitedd(){
  int presscount = 0;
  int t = 0;
         if(readButtons() == 2 && t == 0){
          presscount ++;
          t = millis();
          delay(150);
          if(readButtons() == 2 && t > 0){
            presscount++;
            
          }
          if((millis() - t)<1000 && presscount >= 2){just_left = millis(); exited = true;}
          else if((millis() - t)>1200){
            t == 0;
            presscount == 0;
          }
         }
            
         }

void printNetworks(int networksFound)
{ 
   int cursr = 3;
   for(int i = 0; i < networksFound; i++){
       display.setCursor(0,cursr - 3);
       display.drawLine(0,cursr - 6, 128, cursr - 6, WHITE);
       display.drawLine(0,cursr + 6, 128, cursr + 6, WHITE);
       display.print(String(WiFi.SSID(i).c_str()));
       cursr+=12; 
       display.display();
      }
      return;

}
