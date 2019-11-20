#ifndef CAPTIVE_H
#define CAPTIVE_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "functions.h"

char apname[32];
const byte DNS_PORT = 53;
IPAddress apIP(10, 5, 5, 5);

DNSServer dnsServer;
ESP8266WebServer webServer( 80 );


void handleNotFound();

void writeConfigEEPROM(int, int, int, int);
bool readConfigEEPROM();
void webConfig();


String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".mp4")) return "video/mp4";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


void handleConfig()
{
  String mtrPulley, whlPulley, whlDia, strCellCnt;

  mtrPulley = urldecode( webServer.arg( "mtrPulley" ) );
  whlPulley = urldecode( webServer.arg( "whlPulley" ) );
  whlDia = urldecode( webServer.arg( "whlDia" ) );
  strCellCnt = urldecode( webServer.arg( "cellCnt" ) );

  int e_motorPulley = mtrPulley.toInt();
  int e_wheelPulley = whlPulley.toInt();
  int e_wheelDiameter = whlDia.toInt();
  int e_CellCount = strCellCnt.toInt();

  if ( e_motorPulley > 0 && e_wheelPulley > 0 && e_wheelDiameter > 0 && e_CellCount > 0 )
  {
    writeConfigEEPROM(e_motorPulley, e_wheelPulley, e_wheelDiameter, e_CellCount );
    webServer.send ( 200, getContentType(".html"), "<html><head><script></script></head><body style='background-color:black;font-size:67pt;color:white;'>Please Restart the OSRR!<br>" + mtrPulley + "<br>" + whlPulley + "<br>" + whlDia +  "<br>" + strCellCnt + "</body></html>" );

    gfx.TextColor(YELLOW, BLACK); gfx.Font(2);  gfx.TextSize(1);
    gfx.println("REBOOT");
    gfx.println("  ME");
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
    webServer.send ( 302, getContentType(".txt"), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
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

  if(eesettings.motorPulley > 2000)   eesettings.motorPulley = 0;
  if(eesettings.wheelPulley > 2000)   eesettings.wheelPulley = 0;
  if(eesettings.wheelDiameter > 2000) eesettings.wheelDiameter = 0;
  if(eesettings.cellCount > 2000)     eesettings.cellCount = 0;
  

  String message = "<!DOCTYPE html><html><head>";
  message += "<style>p, form, select{font-size:60pt; color:white;} input{font-size:60pt; color:black;} .button {background-color: #4CAF50; color: white;padding: 15px 32px;text-align: center;font-size: 60pt;float: left;width: 100%;}";
  message += "</style></head><body style='background-color:black'>";
  message += "<form action='/config.php'><table><tr><td>";
  message += " Motor Pulley: </td><td><input type='text' name='mtrPulley' maxlength='3' size='3' value='" + String( eesettings.motorPulley ) + "'></td></tr><tr><td>";
  message += " Wheel Pulley: </td><td><input type='text' name='whlPulley' maxlength='3' size='3' value='" + String( eesettings.wheelPulley ) + "'></td></tr><tr><td>";
  message += "   Cell Count: </td><td><input type='text' name='cellCnt' maxlength='2' size='3' value='" + String( eesettings.cellCount ) +  "'></td></tr><tr><td>";
  message += "Wheel Dia(mm): </td><td><input type='text' name='whlDia' maxlength='3' size='4' value='" + String( eesettings.wheelDiameter ) + "'></td></tr><tr><td colspan=2 style='text-align:center'>";
  message += "<input type='submit' class='button' value='Submit'></td></tr></table>";
  message += "</form></body></html>";

  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.sendHeader("Content-Length", String(message.length()));
  webServer.send ( 200, getContentType(".html"), message );
}



void initWebConfig()
{
  String mac;
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
    yield();
    delay(1);
  }
}

#endif
