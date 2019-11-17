
/*
  Name:    OSRR-Firmware-Beta-v11.ino
  Created: 01-03-2019
  Author:  DerelictRobot / Andrew Dresner
  Source Attribution: SolidGeek/StefanMeGit
  Description:  OSRR Beta Firmware v1.1, for beta hardware version v0.3.
*/

#include <EEPROM.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>

#include <ESP8266WebServer.h>

#include "functions.h"

#include "GFX4dIoD9.h"
#include <Adafruit_ADS1015.h>
#include "config.h"


/** Initiate VescUart class */
// Initiate VescUart class for UART communication
#ifdef ESC_UNITY
#include <VescUartUnity.h>
VescUartUnity UART;
#endif
#ifdef ESC_VESC
#include <VescUart.h>
VescUart UART;
#endif


GFX4dIoD9 gfx = GFX4dIoD9();

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */


char apname[32];
const byte DNS_PORT = 53;
IPAddress apIP(10, 5, 5, 5);

DNSServer dnsServer;
ESP8266WebServer webServer( 80 );

#include "captiveportal.h"

String mac;

void setup() 
{
  EEPROM.begin(512);
  WiFi.softAPdisconnect(true);
  delay(100);
  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(false);
  gfx.BacklightOn(true);
  gfx.Orientation(PORTRAIT_R);
  
  gfx.TextColor(CYAN, BLACK); gfx.Font(2);  gfx.TextSize(1);
  // gfx.TextWindow(0, 0, 80, 82, ORANGE, BLACK);
  gfx.println("      ");
  gfx.println("  OSRR");
  gfx.println(" ONLINE");
  delay(500);
  gfx.Cls();
  delay(50);

  /** Setup UART port (Serial on Atmega32u4) */
  Serial.begin(115200);
  //  while (!Serial) {
  //    ;
  //  }
  delay(50);
  /** Define which ports to use as UART */
  UART.setSerialPort(&Serial);


  delay(50);
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)

  ads.begin();
  delay(50);

  UART.nunchuck.lowerButton = false;

    readConfigEEPROM();  
    calculateRatios();
    delay(10);

    //check thumbwheel position, goto setup if held at full brake position. 
    readThrottle();
    if (throttle < 20)
      {
        webConfig();
      }
      
  updateLCD();                                //Update values on LCD
  gfx.RoundRect(2, 135, 52, 155, 3, LIME);    //Draw Remote Battery Meter Border
      
}

void loop() 
{

  unsigned long currentMillis = millis();

  if (currentMillis - previousVescUpdate >= VescUpdateInterval) {

  readThrottle();

  /** The lowerButton is used to set cruise control ON */
  //      UART.nunchuck.lowerButton = true;

  /** The lowerButton is used to set cruise control OFF */
  UART.nunchuck.lowerButton = false;
  /** The valueY is used to control the speed, where 127 is the middle = no current */
  UART.nunchuck.valueY = throttle;
  /** Call the function setNunchuckValues to send the current nunchuck values to the VESC */
  UART.setNunchuckValues();

  //remoteRSSIRaw = ads.readADC_SingleEnded(2);

    previousVescUpdate = currentMillis;
  }
  
  if (currentMillis - previousLCDUpdate >= LCDUpdateInterval) {
    updateLCD();
    previousLCDUpdate = currentMillis;
  }

  yield(); // Required for ESP
}

void updateLCD()  {
  if ( UART.getVescValues() ) {
#ifdef IMPERIAL
    int speedValue = ((ratioRpmSpeed * UART.data.rpm) * 0.621371);
#endif
#ifdef METRIC
    int speedValue = (ratioRpmSpeed * UART.data.rpm);
#endif
    gfx.MoveTo(15, 10);
    gfx.TextColor(YELLOW, BLACK); gfx.Font(2);  gfx.TextSize(3);
    if (speedValue >= 10) {
      gfx.print(String(speedValue));
    }
    else if (speedValue < 0) {
      gfx.print(String(speedValue));
    }
    else if (speedValue <= 9) {
      gfx.print("0");
      gfx.print(String(speedValue));
    }


    gfx.MoveTo(12, 70);
    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
    gfx.print("V ");
    gfx.print(String(UART.data.inpVoltage, 1));

    //    gfx.TextColor(CYAN, BLACK);
    //    gfx.print("M ");
    //    int currentMotorWatts = (UART.data.avgMotorCurrent * 2) * (UART.data.dutyCycleNow * UART.data.inpVoltage);
    //    if (currentMotorWatts >= 100) {
    //      gfx.println(String(currentMotorWatts));
    //    } else if (currentMotorWatts >= 10) {
    //      gfx.print("0");
    //      gfx.println(String(currentMotorWatts));
    //    } else if (currentMotorWatts <= 9) {
    //      gfx.print("00");
    //      gfx.println(String(currentMotorWatts));
    //    }
    //
    //
    //
    gfx.MoveTo(12, 90);
    gfx.TextColor(ORANGE, BLACK);
    gfx.print("W ");
#ifdef ESC_UNITY
    int currentBatWatts = (UART.data.avgInputCurrent) * (UART.data.inpVoltage);
#endif    
#ifdef ESC_VESC
    int currentBatWatts = (UART.data.avgInputCurrent * 2) * (UART.data.inpVoltage);
#endif    
    if (currentBatWatts >= 1000) {
      gfx.print(String(currentBatWatts));
    }
    else if (currentBatWatts < 0) {
      gfx.print("-RGN");
    }
    else if (currentBatWatts >= 100) {
      gfx.print("0");
      gfx.print(String(currentBatWatts));
    }
    else if (currentBatWatts >= 10) {
      gfx.print("00");
      gfx.print(String(currentBatWatts));
    }
    else if (currentBatWatts <= 9) {
      gfx.print("000");
      gfx.print(String(currentBatWatts));
    }

    gfx.MoveTo(12, 110);
    gfx.TextColor(CYAN, BLACK);
    gfx.print("O ");
#ifdef IMPERIAL
    distanceValue = (ratioPulseDistance * UART.data.tachometerAbs) * 0.621371;
#endif
#ifdef METRIC
    distanceValue = (ratioPulseDistance * UART.data.tachometerAbs);
#endif
    gfx.print(String(distanceValue));



    //    successCount++;
    //    gfx.TextColor(CYAN, BLACK); gfx.Font(2);  gfx.TextSize(1);
    //    gfx.println(String(successCount));
    //    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
    //    gfx.println(String(failCount));

    if (connBlink) {
      gfx.CircleFilled(65, 145, 8, LIMEGREEN);
    } else  {
      gfx.Circle(65, 145, (8), BLACK);
      gfx.Circle(65, 145, (7), BLACK);
      gfx.Circle(65, 145, (6), BLACK);
    }
    connBlink = !connBlink;
  }
  else
  {
    //    failCount++;
    connBlinkCount++;

    if (connBlinkCount >= 5) {
      gfx.CircleFilled(65, 145, 8, YELLOW);
      connBlinkCount = 0;
    }

  }
  updateRemoteBattery();
}

void updateRemoteBattery() {
  int batteryLevel;
  remoteBatRaw = ads.readADC_SingleEnded(3);
  batteryLevel = map(remoteBatRaw, min_ads_bat, max_ads_bat, 0, 100);
  batteryLevel = constrain(batteryLevel, 0, 100);
  remoteBatteryDisplay(batteryLevel);

  //    gfx.MoveTo(12, 64);
  //    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
  //    gfx.print("R ");
  //    gfx.print(String(remoteBatRaw));

}


void remoteBatteryDisplay(int remoteBatVal) {
  if (remoteBatVal > 95)  {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, YELLOW);
    if (remoteBatFlash) {
      gfx.RoundRectFilled(40, 137, 50, 153, 3, LIME);
    }
    else  {
      gfx.RoundRectFilled(40, 137, 50, 153, 3, LIMEGREEN);
    }
    remoteBatFlash = !remoteBatFlash;
  }
  else if (remoteBatVal > 70 && remoteBatVal <= 95) {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, YELLOW);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, LIME);
  }
  else if (remoteBatVal > 50 && remoteBatVal <= 70) {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, YELLOW);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal > 25 && remoteBatVal <= 50) {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, BLACK);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal > 10 && remoteBatVal <= 25) {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, BLACK);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, BLACK);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal <= 10) {
    if (remoteBatFlash) {
      gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    }
    else {
      gfx.RoundRectFilled(4, 137, 14, 153, 3, BLACK);
    }
    remoteBatFlash = !remoteBatFlash;
    gfx.RoundRectFilled(16, 137, 26, 153, 3, BLACK);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, BLACK);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
}


void readThrottle()
{
  thumbwheelVal0 = ads.readADC_SingleEnded(0);
    if (thumbwheelVal0 > adc_max_limit) {
      throttle = 127;
    }
    else
    {
      throttle = map(thumbwheelVal0, min_ads, max_ads, 0, 255);
      throttle = constrain(throttle, 0, 255);
    }
}


void initWebConfig()
{
  WiFi.softAPdisconnect(true);
  mac = WiFi.macAddress();
  mac.replace(":","");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  mac.remove( 0, 6 );
  mac = "OSRR-" + mac;
  mac.toCharArray( apname, 32 );
  WiFi.softAP( apname );

  dnsServer.start(DNS_PORT, "*", apIP);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  webServer.on("/config.php",handleConfig);
  
  webServer.onNotFound( handleNotFound ); //will use for default, just give option page
  webServer.begin();
}

void webConfig()
{
  
  gfx.TextColor(ORANGE, BLACK); gfx.Font(2);  gfx.TextSize(1);
  gfx.println("      ");
  gfx.println(" SETUP");
  gfx.println("  MODE");
  initWebConfig();
  delay(250);
  while(1)
  {
  dnsServer.processNextRequest();
  webServer.handleClient();
  delay(1);
  }
}



bool readConfigEEPROM()
{
  motorPulley = EEPROM.read(addr_motorPulley); 
  wheelPulley = EEPROM.read(addr_wheelPulley); 
  wheelDiameter = EEPROM.read(addr_wheelDiameter);
  calculateRatios();
}

void writeConfigEEPROM(int mp, int wp, int wd)
{
  EEPROM.write(addr_motorPulley, mp);
  EEPROM.write(addr_wheelPulley, wp);
  EEPROM.write(addr_wheelDiameter, wd);
  //if success, need to add check
  EEPROM.commit();
  EEPROM.end();
  delay(100);
}
