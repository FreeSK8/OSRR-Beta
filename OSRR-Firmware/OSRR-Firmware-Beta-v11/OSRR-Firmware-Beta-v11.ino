
/*
  Name:    OSRR-Firmware-Beta-v11.ino
  Created: 01-03-2019
  Author:  DerelictRobot / Andrew Dresner
  Source Attribution: SolidGeek/StefanMeGit
  Description:  OSRR Beta Firmware v1.1, for beta hardware version v0.3.
*/
#include <Wire.h>
#include "display.h"
#include <Adafruit_ADS1015.h>
#include "config.h"

//Initiate GXF4dIoD9 graphics class
GFX4dIoD9 gfx = GFX4dIoD9();

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

#include "captiveportal.h"
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

void setup() 
{
  EEPROM.begin(512);
  WiFi.softAPdisconnect(true);

  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(false);
  gfx.BacklightOn(true);
  gfx.Orientation(PORTRAIT);
  
  bootlogo();
  gfx.Cls();
  delay(50);

  /** Setup UART port (Serial on ESP8266) */
  Serial.begin(115200);
  //  while (!Serial) {
  //    ;
  //  }

  /** Define which ports to use as UART */
  UART.setSerialPort(&Serial);

  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)

  ads.begin();

  UART.nunchuck.lowerButton = false;

  bool forceconfig = !readConfigEEPROM();

  //check thumbwheel position, goto setup if held at full brake position. 
  readThrottle();
  
  if( throttle < 20 || forceconfig )
  {
    webConfig();
  }
    
  updateLCD();                                //Update values on LCD
  gfx.RoundRect(2, 135, 52, 155, 3, LIME);    //Draw Remote Battery Meter Border
      
}

void loop() 
{

  unsigned long currentMillis = millis();

  if( currentMillis - previousVescUpdate >= VescUpdateInterval ) 
  {
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
  
  if( currentMillis - previousLCDUpdate >= LCDUpdateInterval ) 
  {
    updateLCD();
    previousLCDUpdate = currentMillis;
  }

  yield(); // Required for ESP
}

char output[20];
double inpVoltage = 0;

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
    
    sprintf(output,"%02d",speedValue);
    gfx.print(String(output));

    gfx.MoveTo(12, 70);

    inpVoltage = inpVoltage * .8 + UART.data.inpVoltage * .2;  //Smoothing

    if( inpVoltage > ( 3.9 * float( eesettings.cellCount ) ) )       gfx.TextColor(LIME, BLACK); 
    else if( inpVoltage > ( 3.6 * float( eesettings.cellCount ) ) )  gfx.TextColor(YELLOW, BLACK); 
    else                                  gfx.TextColor(RED, BLACK); 
    
    gfx.Font( 2 );  gfx.TextSize( 1 );
    gfx.print( "V " );
    gfx.print( String( inpVoltage, 1 ) );

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
    gfx.MoveTo( 12, 90 );
    gfx.TextColor( ORANGE, BLACK );
    gfx.print( "W" );
#ifdef ESC_UNITY
    int currentBatWatts = (UART.data.avgInputCurrent) * (UART.data.inpVoltage);
#endif    
#ifdef ESC_VESC
    int currentBatWatts = (UART.data.avgInputCurrent * 2) * (UART.data.inpVoltage);
#endif    
   
    sprintf(output,"% 05d",currentBatWatts);
    gfx.print(output);

    gfx.MoveTo(12, 110);
    gfx.TextColor(CYAN, BLACK);
    gfx.print("O ");
    
#ifdef IMPERIAL
    distanceValue = ( ratioPulseDistance * UART.data.tachometerAbs ) * 0.621371;
#endif
#ifdef METRIC
    distanceValue = ( ratioPulseDistance * UART.data.tachometerAbs );
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
      gfx.Circle(65, 145, 8, BLACK);
      gfx.Circle(65, 145, 7, BLACK);
      gfx.Circle(65, 145, 6, BLACK);
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

void updateRemoteBattery() 
{
  int batteryLevel;
  remoteBatRaw = ads.readADC_SingleEnded( 3 );
  batteryLevel = map( remoteBatRaw, min_ads_bat, max_ads_bat, 0, 100 );
  batteryLevel = constrain( batteryLevel, 0, 100 );
  remoteBatteryDisplay( batteryLevel );

  //    gfx.MoveTo(12, 64);
  //    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
  //    gfx.print("R ");
  //    gfx.print(String(remoteBatRaw));

}

void remoteBatteryDisplay(int remoteBatVal) 
{  
  if (remoteBatVal > 95)  
  {
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
  else if (remoteBatVal > 70 && remoteBatVal <= 95) 
  {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, YELLOW);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, LIME);
  }
  else if (remoteBatVal > 50 && remoteBatVal <= 70) 
  {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, YELLOW);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal > 25 && remoteBatVal <= 50) 
  {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, ORANGE);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, BLACK);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal > 10 && remoteBatVal <= 25) 
  {
    gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    gfx.RoundRectFilled(16, 137, 26, 153, 3, BLACK);
    gfx.RoundRectFilled(28, 137, 38, 153, 3, BLACK);
    gfx.RoundRectFilled(40, 137, 50, 153, 3, BLACK);
  }
  else if (remoteBatVal <= 10) 
  {
    if (remoteBatFlash) 
    {
      gfx.RoundRectFilled(4, 137, 14, 153, 3, RED);
    }
    else 
    {
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
