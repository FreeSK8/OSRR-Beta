#ifndef CONFIG_H
#define CONFIG_H

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


unsigned long previousMillis = 0;
const long interval = 250;

int16_t thumbwheelVal0;
int16_t thumbwheelVal1;
String command;
int throttle = 127;
int throttleCenterMargin = 5;
bool throttleActive = false;
int min_ads = 0;
int max_ads = 1700;
int adc_max_limit = 2000;


int16_t remoteBatRaw;
int min_ads_bat = 1000;
int max_ads_bat = 1400;
int adc_max_bat_limit = 2000;

int16_t remoteRSSIRaw;

void calculateRatios()  {
  gearRatio = (motorPulley) / (wheelPulley);
  ratioRpmSpeed = (gearRatio * 60 * wheelDiameter * 3.14156) / ((motorPoles / 2) * 1000000);
  ratioPulseDistance = (gearRatio * wheelDiameter * 3.14156) / ((motorPoles * 3) * 1000000);

}


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
bool remoteBatFlash = true;
bool connBlink = true;
int connBlinkCount = 0;
unsigned long failCount = 0;
unsigned long successCount = 0;

//struct vescValues data;

#endif
