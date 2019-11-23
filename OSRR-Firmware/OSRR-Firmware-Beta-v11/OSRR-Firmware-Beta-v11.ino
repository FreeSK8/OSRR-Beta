
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
    
  screenHomeClassic();                                //Update values on LCD
      
}

void loop() 
{

  unsigned long currentMillis = millis();

  if( currentMillis - previousThrottleUpdate >= VescUpdateInterval ) 
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
  
    previousThrottleUpdate = currentMillis;
  }

  checkTelemetry();
  
  if( currentMillis - previousLCDUpdate >= LCDUpdateInterval ) 
  {
    screenHomeClassic();
    previousLCDUpdate = currentMillis;
  }
  
  yield(); // Required for ESP
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

void checkTelemetry()
{
  if ( UART.getVescValues()) {
    previousTelemetryUpdate = millis();
    remoteConnected = true;
    
    #ifdef IMPERIAL
      speedValue = ((ratioRpmSpeed * UART.data.rpm) * 0.621371);
    #endif
    #ifdef METRIC
      speedValue = (ratioRpmSpeed * UART.data.rpm);
    #endif

    boardVoltage = boardVoltage * .8 + UART.data.inpVoltage * .2;  //Smoothing

    avgMotorCurrent = UART.data.avgMotorCurrent * 2;

    #ifdef ESC_UNITY
      avgInputCurrent = UART.data.avgInputCurrent;
    #endif    
    #ifdef ESC_VESC
      avgInputCurrent = UART.data.avgInputCurrent*2;
    #endif   

    avgInputWatts = avgInputCurrent * boardVoltage;
    
    avgInputCurrent = UART.data.avgInputCurrent;

    #ifdef IMPERIAL
        tachometer = ( ratioPulseDistance * UART.data.tachometerAbs ) * 0.621371;
    #endif
    #ifdef METRIC
        tachometer = ( ratioPulseDistance * UART.data.tachometerAbs );
    #endif
    
  } else {
    if (remoteConnected && millis() >= previousTelemetryUpdate + remoteConnectionTimeout ) { //if status is connected but no telemetry was recieved for a while
      remoteConnected = false;
    }
  }
}
