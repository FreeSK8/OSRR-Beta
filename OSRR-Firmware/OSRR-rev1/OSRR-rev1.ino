
/*
  Name:    OSRR-FirmwareV1.ino
  Created: 01-03-2019
  Author:  DerelictRobot / Andrew Dresner
  Source Attribution: SolidGeek/StefanMeGit
  Description:  Null.
*/

// - Choose UART protocol:
#define ESC_UNITY             // ESC_UNITY for UART communication with a UNITY
//#define ESC_VESC                // ESC_VESC for UART communication with a VESC 4.12-6.6

const int wheelDiameter = 200;  //Wheel diameter in MM.
const float gearRatio = 0.25; //(Gear Ratio = Motor Gear Tooth Count)/(Wheel Gear Tooth Count)
                              // ie: 0.25 = 4:1 = 15T/60T
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

    int calcMPH = (((wheelDiameter / 304.8) * 3.14159 * (((UART.data.rpm) / 7) * gearRatio) * 60) / 5280);
    gfx.MoveTo(15, 5);
    gfx.TextColor(YELLOW, BLACK); gfx.Font(2);  gfx.TextSize(3);
    if (calcMPH >= 10) {
      gfx.print(String(calcMPH));
    }
    else if (calcMPH < 0) {
      gfx.print(String(calcMPH));
    }
    else if (calcMPH <= 9) {
      gfx.print("0");
      gfx.print(String(calcMPH));
    }

    
    
//    if (throttle > 130) {
//    int throttleBar = map(throttle, 128, 255, 50, 0);    
//    gfx.RectangleFilled(0,50,5,throttleBar,CYAN);
//    } else if (throttle < 125)  {
//    int throttleBar = map(throttle, 127, 0, 50, 0);    
//    gfx.RectangleFilled(0,50,5,throttleBar,ORANGE);     
//    } else  {
//      gfx.RectangleFilled(0,50,5,0,BLACK);
//    }

    gfx.MoveTo(0, 100);
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
//    gfx.TextColor(ORANGE, BLACK);
//    gfx.print("B ");
//    int currentBatWatts = (UART.data.avgInputCurrent * 2) * (UART.data.inpVoltage);
//    if (currentBatWatts >= 100) {
//      gfx.println(String(currentBatWatts));
//    } else if (currentBatWatts >= 10) {
//      gfx.print("0");
//      gfx.println(String(currentBatWatts));
//    } else if (currentBatWatts <= 9) {
//      gfx.print("00");
//      gfx.println(String(currentBatWatts));
//    }


//    gfx.TextColor(YELLOW, BLACK);
//    gfx.print("T ");
//    float calcODO = ((0.492126 * 3.14159 * ((UART.data.tachometerAbs) / 7) * 0.23) / 5280);
//    gfx.println(String(calcODO));

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
