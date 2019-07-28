
/*
  Name:    OSRR-rev1.ino
  Created: 01-03-2019
  Author:  DerelictRobot / Andrew Dresner
  Source Attribution: SolidGeek/StefanMeGit
  Description:  Null.
*/

// - Choose UART protocol:
#define ESC_UNITY             // ESC_UNITY for UART communication with a UNITY
//#define ESC_VESC                // ESC_VESC for UART communication with a VESC 4.12-6.6

// - Choose Metric or Imperial
//#define METRIC
#define IMPERIAL

// Defining variables to hold values for speed and distance calculation
float wheelDiameter = 200;  //Wheel diameter in MM.
float wheelPulley = 60;     //Wheel Pulley Tooth Count
float motorPulley = 15;     //Motor Pulley Tooth Count
float motorPoles = 14;      //Motor Poles - 14 default

float gearRatio;                      
float ratioRpmSpeed;
float ratioPulseDistance;
float distanceValue;
                              
#include <Wire.h>
#include "ESP8266WiFi.h"
#include "GFX4dIoD9.h"
#include <Adafruit_ADS1015.h>

#ifdef ESC_VESC
#include <VescUart.h>
#endif
#ifdef ESC_UNITY
#include <VescUartUnity.h>
#endif

#ifdef ESC_UNITY
// Defining struct to handle callback data for UNITY
struct callback {
  float ampHours;
  float inpVoltage;
  long rpm;
  long tachometerAbs;
  uint8_t headlightActive;
  float avgInputCurrent;
  float avgMotorCurrent0;
  float avgMotorCurrent1;
  float dutyCycleNow0;
  float dutyCycleNow1;
  bool eStopArmed;
  int8_t receiverRssi;
  float filteredFetTemp0;
  float filteredFetTemp1;
  float filteredMotorTemp0;
  float filteredMotorTemp1;
} returnData;
#endif

#ifdef ESC_VESC
// Defining struct to handle callback data for VESC
struct callback {
  float ampHours;
  float inpVoltage;
  long rpm;
  long tachometerAbs;
  uint8_t headlightActive;
  float avgInputCurrent;
  float avgMotorCurrent0;
  float dutyCycleNow0;
  bool eStopArmed;
  int8_t receiverRssi;
  float filteredFetTemp0;
  float filteredMotorTemp0;
} returnData;
#endif


// Defining struct to hold stats
struct stats {
  float maxSpeed;
  long maxRpm;
  float minVoltage;
  float maxVoltage;
};

// Defining variables for speed and distance calculation



bool recievedData = false;
uint32_t lastTimeReceived = 0;
int timeoutMax = 500;
unsigned long lastDataCheck;
bool connBlink = true;
int connPulse = 1;
int connBlinkCount = 0;
unsigned long failCount = 0;
unsigned long successCount = 0;

//struct vescValues data;

/** Initiate VescUart class */
// Initiate VescUart class for UART communication
#ifdef ESC_UNITY
  VescUartUnity UART;
#endif
#ifdef ESC_VESC
  VescUart UART;
#endif

GFX4dIoD9 gfx = GFX4dIoD9();
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

unsigned long previousMillis = 0;
const long interval = 250;

int16_t thumbwheelVal;
String command;
int throttle = 127;
int throttleCenterMargin = 5;
bool throttleActive = false;
int min_ads = 0;
int max_ads = 1700;
int adc_max_limit = 2000;


void calculateRatios()  {
  gearRatio = (motorPulley) / (wheelPulley);                              
  ratioRpmSpeed = (gearRatio * 60 * wheelDiameter * 3.14156) / ((motorPoles / 2) * 1000000);
  ratioPulseDistance = (gearRatio * wheelDiameter * 3.14156) / ((motorPoles * 3) * 1000000);

}

void setup() {


  delay(100);
  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(false);
  gfx.BacklightOn(true);
  gfx.Orientation(PORTRAIT);
//  gfx.SmoothScrollSpeed(5);
  gfx.TextColor(CYAN, BLACK); gfx.Font(2);  gfx.TextSize(1);
  // gfx.TextWindow(0, 0, 80, 82, ORANGE, BLACK);
  gfx.println("      ");
  gfx.println("  OSRR");
  gfx.println(" ONLINE");
  delay(500);
  gfx.Cls();
  delay(150);

  /** Setup UART port (Serial on Atmega32u4) */
  Serial.begin(115200);
//  while (!Serial) {
//    ;
//  }
  delay(100);
  /** Define which ports to use as UART */
  UART.setSerialPort(&Serial);

  delay(100);
  updateLCD();
  delay(100);
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();
  delay(100);

  UART.nunchuck.lowerButton = false;

  calculateRatios();

}





void loop() {

  thumbwheelVal = ads.readADC_SingleEnded(0);
  if (thumbwheelVal > adc_max_limit){
    throttle = 127;
  }
  else
  {
  throttle = map(thumbwheelVal, min_ads, max_ads, 0, 255);
  throttle = constrain(throttle, 0, 255);
  }
  /** The lowerButton is used to set cruise control ON */
  //      UART.nunchuck.lowerButton = true;

  /** The lowerButton is used to set cruise control OFF */
  UART.nunchuck.lowerButton = false;
  /** The valueY is used to control the speed, where 127 is the middle = no current */
  UART.nunchuck.valueY = throttle;
  /** Call the function setNunchuckValues to send the current nunchuck values to the VESC */
  UART.setNunchuckValues();
  delay(20); //approx 48hz



  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateLCD();
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
    gfx.MoveTo(15, 5);
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

    
    gfx.MoveTo(0, 80);
    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
    gfx.print("V ");
    gfx.println(String(UART.data.inpVoltage));

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
    gfx.TextColor(ORANGE, BLACK);
    gfx.print("W ");
    int currentBatWatts = (UART.data.avgInputCurrent * 2) * (UART.data.inpVoltage);
    if (currentBatWatts >= 1000) {
      gfx.println(String(currentBatWatts));
      }
      else if (currentBatWatts < 0) {
      gfx.println("-RGN");  
      }
     else if (currentBatWatts >= 100) {
      gfx.print("0");
      gfx.println(String(currentBatWatts));
      }       
     else if (currentBatWatts >= 10) {
      gfx.print("00");
      gfx.println(String(currentBatWatts));
      } 
    else if (currentBatWatts <= 9) {
      gfx.print("000");
      gfx.println(String(currentBatWatts));
    }


    gfx.TextColor(CYAN, BLACK);
    gfx.print("T ");
    #ifdef IMPERIAL
    distanceValue = (ratioPulseDistance * UART.data.tachometerAbs) * 0.621371;
    #endif
    #ifdef METRIC
    distanceValue = (ratioPulseDistance * UART.data.tachometerAbs);
    #endif    
    gfx.println(String(distanceValue));

//    successCount++;
//    gfx.TextColor(CYAN, BLACK); gfx.Font(2);  gfx.TextSize(1);
//    gfx.println(String(successCount));
//    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
//    gfx.println(String(failCount));

    if (connBlink) {
      gfx.CircleFilled(65, 145, 8, GREEN);
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
}
