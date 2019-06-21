# VescUartUnity

Arduino library for FocBox Unity from ENERTION

**Important:** This is not a dropin replacement for RollingGeckos library. You will have to make some changes to your software, as all functions and values is now within a class, see below.

## Implementation

To use the library you will have initiate the VescUartUnity class and set the Serial port for UART communcation.

```cpp
#include <VescUartUnity.h>

VescUartUnity UART;

void setup() {
  Serial.begin(115200);

  while (!Serial) {;}

  UART.setSerialPort(&Serial);
}
```

You can now safely use the functions and change the values of the class.

Getting VESC telemetry is easy:

```cpp
if ( UART.getVescValues() ) {
  Serial.println(UART.data.filteredFetTemp0);
  Serial.println(UART.data.inpVoltage);
  Serial.println(UART.data.filteredMotorTemp1);
  Serial.println(UART.data.ampHours);
  ...
}
```
