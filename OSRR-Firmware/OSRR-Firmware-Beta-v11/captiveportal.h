#ifndef CAPTIVE_H
#define CAPTIVE_H

#include "functions.h"

void handleNotFound();

void writeConfigEEPROM(int, int, int);
bool readConfigEEPROM();
void webConfig();


void handleConfig()
{
  bool checkPass = true;
  String mtrPulley;
  String whlPulley;
  String whlDia;

  if ( webServer.hasArg( "mtrPulley" ) )
  {
    mtrPulley = urldecode( webServer.arg( "mtrPulley" ) );
  }

  if ( webServer.hasArg( "whlPulley" ) )
  {
    whlPulley = urldecode( webServer.arg( "whlPulley" ) );
  }
  
  if ( webServer.hasArg( "whlDia" ) )
  {
    whlDia = urldecode( webServer.arg( "whlDia" ) );
  }

  e_motorPulley = mtrPulley.toInt();
  e_wheelPulley = whlPulley.toInt();
  e_wheelDiameter = whlDia.toInt();

  if (e_motorPulley > 0 && e_wheelPulley > 0 && e_wheelDiameter > 0)
  {
    writeConfigEEPROM(e_motorPulley, e_wheelPulley, e_wheelDiameter);
    webServer.send ( 200, "text/html", "<html><head><script></script></head><body style='background-color:black;font-size:67pt;color:white;'>Please Restart the OSRR!<br>" + mtrPulley + "<br>" + whlPulley + "<br>" + whlDia + "</body></html>" );
  }
  else
  {
    handleNotFound();
  }
}

boolean captivePortal()
{
  if (!isIp(webServer.hostHeader()) )
  {
    webServer.sendHeader("Location", String("http://") + toStringIp(webServer.client().localIP()), true);
    webServer.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleNotFound()
{
  if (captivePortal())
  {
    return;
  }

  String message = "<!DOCTYPE html><html><head><style>p, form, select{font-size:67pt; color:white;} input{font-size:67pt; color:black;}</style></head><body style='background-color:black'>";
  message += "<form action='/config.php'><table><tr><td>";
  message += "Motor Pulley: </td><td><input type='text' name='mtrPulley' maxlength='3' size='3'></td></tr><tr><td>";
  message += "Wheel Pulley: </td><td><input type='text' name='whlPulley' maxlength='3' size='3'></td></tr><tr><td>";
  message += "Wheel Dia(mm): </td><td><input type='text' name='whlDia' maxlength='3' size='4'></td></tr><tr><td colspan=2 style='text-align:center'>";
  message += "<input type='submit' value='Submit'></td></tr></table>";
  message += "</form>";
  message += "<p>Click the 'Submit' button to update your settings</p>";
  message += "</body></html>";

  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.sendHeader("Content-Length", String(message.length()));
  webServer.send ( 200, "text/html", message );
}

#endif
