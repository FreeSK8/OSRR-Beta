/************************************
 * Graphics Demo IoD-09             *
 * This Demo show's some of the     *
 * graphics features of the IoD-09  *    *
 * Enter your SSID & PASSWORD below *
 ***********************************/

#include "ESP8266WiFi.h"
#include "GFX4dIoD9.h"

const char* ssid = "********";
const char* password = "********";

GFX4dIoD9 gfx = GFX4dIoD9();


uint32_t invader1a[20] = {
18,18,
0x00000000, 0x00000780, 0x00000FC0,    
0x00001FE0, 0x00003FF0, 0x00007FF8,    
0x0000FFFC, 0x0001E79E, 0x0001E79E,    
0x0001FFFE, 0x0000FFFC, 0x00006798,     
0x0000C30C, 0x00018006, 0x00018006,    
0x0000C00C, 0x00006018, 0x00000000, 
};
uint32_t invader1b[20] = {
18,18,
0x00000000, 0x00000780, 0x00000FC0,     
0x00001FE0, 0x00003FF0, 0x00007FF8,    
0x0000FFFC, 0x0001E79E, 0x0001E79E,    
0x0001FFFE, 0x0000FFFC, 0x00001860,     
0x000037B0, 0x00006318, 0x00018006,    
0x0000F03C, 0x00018CC6, 0x00000000, 
};

uint32_t invader2a[20] = {
24,18,
0x00000000, 0x00060060, 0x000300C0,     
0x00618186, 0x0060C306, 0x0063FFC6,    
0x0067FFE6, 0x007E7E7E, 0x007E7E7E,    
0x007FFFFE, 0x003FFFFC, 0x003FFFFC,    
0x001FFFF8, 0x00060060, 0x000C0030,     
0x00180018, 0x0030000C, 0x00000000,  
};

uint32_t invader2b[20] = {
24,18,
0x00000000, 0x00060060, 0x000300C0,    
0x00018180, 0x0000C300, 0x0003FFC0,     
0x0007FFE0, 0x003E7E7C, 0x007E7E7E,    
0x007FFFFE, 0x007FFFFE, 0x007FFFFE,    
0x007FFFFE, 0x00660066, 0x006300C6,    
0x00018180, 0x0000E700, 0x00000000,  
};

uint32_t invader3a[20] = {
24,18,
0x00000000, 0x00007E00, 0x0000FF00,      
0x000FFFF0, 0x003FFFFC, 0x007FFFFE,    
0x007C3C3E, 0x007C3C3E, 0x007FFFFE,    
0x003FFFFC, 0x000FC3F0, 0x000FC3F0,    
0x00067E60, 0x000C0030, 0x00180018,   
0x000C0030, 0x00060060, 0x00000000,  
};

uint32_t invader3b[20] = {
24,18,
0x00000000, 0x00007E00, 0x0000FF00,      
0x000FFFF0, 0x003FFFFC, 0x007FFFFE,    
0x007C3C3E, 0x007C3C3E, 0x007FFFFE,    
0x003FFFFC, 0x000FC3F0, 0x000FC3F0,    
0x00037EC0, 0x00060060, 0x000C0030,     
0x00180018, 0x0030000C, 0x00000000,  
};

byte type;
byte type1;
byte type2;
byte typec;
boolean Wconnected = false;
int x;
int y;
int r;
uint16_t c;
int rot;
uint16_t n;


void setup() {
  Serial.begin(38400);
  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(true);
  //gfx.BacklightOn(true);
  gfx.Orientation(LANDSCAPE);
  gfx.SmoothScrollSpeed(5);
  gfx.TextColor(WHITE); gfx.Font(2);  gfx.TextSize(1);
  gfx.TextWindow(0, 0, 162, 90, ORANGE, BLACK);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  gfx.TWprintln("Setup done");
  for (int x = 0; x < 3; x ++) {
    String tab;
    String ssidtemp;
    int len;
    gfx.TWcolor(ORANGE);
    gfx.TWprintln("Scanning ...");
    int n = WiFi.scanNetworks();
    gfx.TWcolor(GREEN);
    gfx.TWprintln("Finished Scan");
    if (n == 0) {
      gfx.TWcolor(RED);
      gfx.TWprintln("no networks found");
    } else {
      gfx.TWcolor(GREEN);
      gfx.TWprint(String(n));
      gfx.TWprintln(" Networks found");
      for (int i = 0; i < n; ++i)
      {
        gfx.TWcolor(ORANGE);
        gfx.TWprint(String(i + 1));
        gfx.TWcolor(WHITE);
        gfx.TWprint(": ");
        gfx.TWcolor(YELLOW);
        ssidtemp = String(WiFi.SSID(i));
        len = ssidtemp.length();
        String tempssid = "";
        for (int n = 0; n < (10); n++) {
          if (n <= len) {
            tempssid = tempssid + ssidtemp.charAt(n);
          } else {
            tempssid = tempssid + " ";
          }
        }
        gfx.TWprint(tempssid);
        //gfx.TWprint(WiFi.SSID(i));

        if (WiFi.RSSI(i) < 0) {
          gfx.TWcolor(LIGHTGREEN);
        }
        if (WiFi.RSSI(i) < -50) {
          gfx.TWcolor(ORANGE);
        }
        if (WiFi.RSSI(i) < -80) {
          gfx.TWcolor(RED);
        }
        gfx.TWprint(String(WiFi.RSSI(i)));
        gfx.TWprintln("");
      }
    }


    delay(2000);
  }
   gfx.TWcolor(LIME);
   gfx.TWprintln("connecting to ");
   gfx.TWprintln(ssid);
    gfx.TWcolor(SALMON);
    WiFi.begin(ssid, password);
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 22)) {
  retries++;
  delay(1000);
  gfx.TWprintln("Connecting");
  
  }
  gfx.TWprintln(" ");
  if(WiFi.status() == WL_CONNECTED){
  Wconnected = true;
   gfx.TWcolor(LIME);
  gfx.TWprintln("WiFi connected");

  gfx.TWprintln("IP address: ");
  String ip;
  ip = ipToString(WiFi.localIP());
  gfx.TWprintln(ip);
  } else {
  Wconnected = false;
   gfx.TWcolor(RED);
  gfx.TWprintln("Connection Fail.");
  }
  if(Wconnected){
    delay(2000);
    gfx.TWcolor(YELLOW);
    gfx.TWprintln("Connecting to.");
    gfx.TWcolor(RED);
    gfx.TWprint("4D ");
    gfx.TWcolor(WHITE);
    gfx.TWprintln("Systems");
    delay(1000);
    gfx.MoveTo(0,0);
    gfx.PrintImageWifi("http://hosting.4dsystems.com.au/IoDFiles/iod9.gci");
    delay(4000);
    gfx.Cls();
    }
    gfx.Open4dGFX("iod9");
    gfx.TWcolor(WHITE);
}

void loop() {

 
  gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("UserImages");
  gfx.TWprintln("Demonstration");
  delay(3000);
  gfx.Cls();
   gfx.UserImages(0,0) ;  // Coolgauge1 show initialy, if required
  gfx.UserImages(1,0) ;  // Gauge1 show initialy, if required
  gfx.UserImage(2);  // Leddigits1 show all digits at 0, only do this once
  gfx.UserImage(4) ; // Spectrum1 show initial spectrum
  for(int n = 0; n < 100; n ++){
   gfx.UserImages(0, n) ; // where frame is 0 to 100 (for a displayed 0 to 100)
  gfx.UserImages(1, n) ; // where frame is 0 to 100 (for a displayed 0 to 100)
  gfx.LedDigitsDisplay(n, 3, 2, 1, 23, 0) ;  // Leddigits1
  //gfx.UserImages(iSpectrum1, frame, 5 + bar * 9) ; // where frame is 0 to 100 (for a displayed 0 to 100) and bar is 0 to 3 
  }
  for(int n = 0; n < 250; n++){
    for(int o = 0; o < 4; o++){
      gfx.UserImages(5, random(100), 5 + o * 9) ; // where frame is 0 to 100 (for a displayed 0 to 100) and bar is 0 to 3 
    }
  }
  gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("Primitive Circle");
  gfx.TWprintln("Demonstration");
  delay(3000);
  gfx.Cls();
  yield();
 for(int o = 0; o < 4; o ++){
  for(int n = 0; n < 1000; n++){
  x = random(159);
  y = random(79);
  r = random(20);
  c = random(65535);
  gfx.Circle(x,y,r,c);
}
 yield();
 }
 yield();
 gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("User Character");
  gfx.TWprintln("Demonstration");
  delay(3000);
  gfx.Cls();
 n=0;
  y=0;
  invaders();
 gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("Moving Widgets");
  gfx.TWprintln("Demonstration");
  delay(3000);
 gfx.Cls();
 for(int n = 0; n < 210; n ++){
 for(int o = 0; o < 4; o ++){
 gfx.Orientation(o);
 if(o != 1){
  //gfx.LedDigitsDisplay(n, 1, 4,1,9,0);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 - n, 55 - n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 + n, 55 - n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 - n, 55 + n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 + n, 55 + n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64, 55 + n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64, 55 - n);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 + n, 55);
  gfx.LedDigitsDisplaySigned(n, 7, 4,1,9,0, 64 - n, 55);
  //gfx.LedDigitsDisplaySigned(n, 1, 4,1,9,0);
  //delay(200);
 }
 }
 }
 gfx.Orientation(0);
 gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("LED Digits");
  gfx.TWprintln("Demonstration");
  delay(3000);
  gfx.TextColor(BLACK);
  gfx.Cls();
  for(int n = 0; n < 1000; n = n+5){
    gfx.LedDigitsDisplay(n, 9, 3, 3, 50, 0) ;
  }
 gfx.Cls();
  gfx.TextWindowRestore();
    delay(1000);
  gfx.TWprintln("3DPanel & Text");
  gfx.TWprintln("Demonstration");
  delay(3000);
  gfx.TextColor(BLACK);
  gfx.Cls();
  for(int o = 0; o < 4; o ++){
//delay(2000);
gfx.Cls();
switch(o){
case 0:
gfx.Orientation(o);
gfx.PanelRecessed(5,5,150,70,LIGHTGREY);
  gfx.MoveTo(45,31);
  gfx.print("Hello 4D");
  //gfx.Circle(39,39,39,BLUE);
  gfx.Rectangle(0,0,159,79,RED);
  delay(2000);

break;
case 1:
gfx.Orientation(o);
gfx.PanelRecessed(5,5,150,70,LIGHTGREY);
  gfx.MoveTo(45,31);
  gfx.print("Hello 4D");
  //gfx.Circle(39,39,39,BLUE);
  gfx.Rectangle(0,0,159,79,RED);
  delay(2000);

break;
case 2:
gfx.Orientation(o);
gfx.PanelRecessed(5,5,70,150,LIGHTGREY);
  gfx.MoveTo(20,61);
  gfx.print("Hello");
  //gfx.Circle(39,39,39,BLUE);
  gfx.Rectangle(0,0,79,159,RED);
  delay(2000);

break;
case 3:
gfx.Orientation(o);
gfx.PanelRecessed(5,5,70,150,LIGHTGREY);
  gfx.MoveTo(20,61);
  gfx.print("Hello");
  //gfx.Circle(39,39,39,BLUE);
  gfx.Rectangle(0,0,79,159,RED);
  delay(2000);

break;
}
  }
    gfx.Orientation(LANDSCAPE);
gfx.TextColor(WHITE);

}

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
  s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void moveinvadera(){
  if(typec == 0 || typec == 1){
  type = 1;
  } else {
  type = 0;
  }
  for(int i = 0; i < 4; i++){
  if(type == 0){
  gfx.UserCharacter(invader1a,20,8 + (i * 26) + n,10 + y,MAGENTA,BLACK);
  } else {
  gfx.UserCharacter(invader1b,20,8 + (i * 26) + n,10 + y,MAGENTA,BLACK);
  }
  }
}

void moveinvaderc(){
  for(int i = 0; i < 4; i++){
  if(type == 0){
  gfx.UserCharacter(invader2a,20,5 + (i * 26) + n,31 + y,LIGHTBLUE,BLACK);
  } else {
  gfx.UserCharacter(invader2b,20,5 + (i * 26) + n,31 + y,LIGHTBLUE,BLACK);
  }
  }
}
void moveinvaderb(){
  for(int i = 0; i < 4; i++){
  if(type == 0){
  gfx.UserCharacter(invader2a,20,5 + (i * 26) + n,52 + y,LIGHTBLUE,BLACK);
  } else {
  gfx.UserCharacter(invader2b,20,5 + (i * 26) + n,52 + y,LIGHTBLUE,BLACK);
  }
  }
}

void invaders(){
  for(int t=0;t<4;t++){
  for(int x=0;x<50;x++){
  n=n+1;
  moveinvadera();
  yield();
  delayMicroseconds(800);
  moveinvaderc();
  typec = typec + 1;
  if(typec > 3){
  typec = 0;
  }
  y=y+24;
  y=y-24;
  }
  for(int m=0;m<3;m++){
  y=y+1;
  moveinvadera();
  yield();
  delayMicroseconds(800);
  moveinvaderc();
  y=y+24;
  y=y-24;
  }
  for(int x=0;x<50;x++){
  n=n-1;
  moveinvadera();
  yield();
  delayMicroseconds(800);
  moveinvaderc();
  typec = typec + 1;
  if(typec > 3){
  typec = 0;
  }
  y=y+24;
  y=y-24;
  }
  for(int m=0;m<3;m++){
  y=y+1;
  moveinvadera(); 
  yield();
  delayMicroseconds(800);
  moveinvaderc();
  y=y+24;
  y=y-24;
  }
  yield();
  if(y > 205){
  y= 10;
  } 
  }
}

