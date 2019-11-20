#ifndef CONFIG_H
#define CONFIG_H

// - Choose UART protocol:
#define ESC_UNITY             // ESC_UNITY for UART communication with a UNITY
//#define ESC_VESC                // ESC_VESC for UART communication with a VESC 4.12-6.6

// - Choose Metric or Imperial
//#define METRIC
#define IMPERIAL

// - Choose Simple or Full Display mode
// #define SIMPLE
// #define FULL

#include <EEPROM.h>
#include "CRC32.h"
#include <math.h>

// Defining variables to hold values for speed and distance calculation
struct __attribute__((__packed__)) EEPROMSettings
{
  uint16_t wheelDiameter;
  uint16_t wheelPulley;
  uint16_t motorPulley;
  float motorPoles;  //Motor Poles - 14 default
  uint16_t cellCount;

  uint32_t CRC; //4
};

EEPROMSettings eesettings;


float gearRatio;
float ratioRpmSpeed;
float ratioPulseDistance;
float distanceValue;


unsigned long previousLCDUpdate = 0;
unsigned long previousVescUpdate = 0;
const long LCDUpdateInterval = 250;
const long VescUpdateInterval = 20;

int16_t thumbwheelVal0;
int16_t thumbwheelVal1;
String command;
int16_t throttle = 127;
int16_t throttleCenterMargin = 5;
bool throttleActive = false;
int16_t min_ads = 0;
int16_t max_ads = 1700;
int16_t adc_max_limit = 2000;


int16_t remoteBatRaw;
int16_t min_ads_bat = 1000;
int16_t max_ads_bat = 1400;
int16_t adc_max_bat_limit = 2000;

int16_t remoteRSSIRaw;

void calculateRatios()  {
  gearRatio = ( float( eesettings.motorPulley ) / float( eesettings.wheelPulley ) );
  ratioRpmSpeed = gearRatio * 60 * ( float( eesettings.wheelDiameter ) * M_PI ) / ((eesettings.motorPoles / 2) * 1000000);
  ratioPulseDistance = gearRatio * ( float( eesettings.wheelDiameter ) * M_PI ) / ((eesettings.motorPoles * 3) * 1000000);
}

//UNUSED (Future ToDo)
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

//UNUSED
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

// UNUSED
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
bool remoteBatFlash = true;
bool connBlink = true;
int connBlinkCount = 0;
unsigned long failCount = 0;
unsigned long successCount = 0;


uint32_t CalculateEEPROM_CRC()
{
  uint8_t* settings_ptr = (uint8_t*)&eesettings;
  int length = sizeof(eesettings) - sizeof(uint32_t);
  
  return CRC32::calculate(settings_ptr, length);
}

bool LoadSavedSettings()
{
  uint32_t calculated = CalculateEEPROM_CRC();
  uint32_t saved = eesettings.CRC;

  if( calculated == saved )
  {
    /*
    Serial.println(eesettings.wheelDiameter);
    Serial.println(eesettings.wheelPulley);
    Serial.println(eesettings.motorPulley);
    Serial.println(eesettings.cellCount);
    */
    return true;
  }

  gfx.TextColor( RED, BLACK ); gfx.Font( 2 );  gfx.TextSize( 1 );
  gfx.println( "  Uh " );
  gfx.println( "  Oh " );
  gfx.println();

  long long mill = millis() + 2000;
  while( millis() < mill )
  {
    yield();
  }

  return false;
}


bool readConfigEEPROM()
{
  EEPROM.get( 0, eesettings );
  if( !LoadSavedSettings() ) return false;
  
  calculateRatios();
}

void writeConfigEEPROM(int mp, int wp, int wd, int cellcount)
{
  eesettings.wheelDiameter = wd;
  eesettings.wheelPulley = wp;
  eesettings.motorPulley = mp;
  eesettings.cellCount = cellcount;
  eesettings.motorPoles = 14;  //default

  uint32_t calcCRC = CalculateEEPROM_CRC();
  eesettings.CRC = calcCRC;
  EEPROM.put( 0, eesettings );
  
  EEPROM.commit();
  EEPROM.end();

  calculateRatios();
}


#endif
