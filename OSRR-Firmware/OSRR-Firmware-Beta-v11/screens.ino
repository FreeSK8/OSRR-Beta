char output[20];

void screenHomeClassic() {   //display the home screen
  //draw speed
  gfx.MoveTo(15, 10);
  if (remoteConnected) gfx.TextColor(YELLOW, BLACK); else gfx.TextColor(GRAY, BLACK);
  gfx.Font(2);  
  gfx.TextSize(3);
  sprintf(output,"%02d",constrain(int(speedValue),0,99));
  gfx.print(String(output));

  //draw board voltage
  gfx.MoveTo(12, 70);
  if (remoteConnected) { 
    if( boardVoltage > ( 3.9 * float( eesettings.cellCount ) ) )       gfx.TextColor(LIME, BLACK); 
    else if( boardVoltage > ( 3.6 * float( eesettings.cellCount ) ) )  gfx.TextColor(YELLOW, BLACK); 
    else gfx.TextColor(RED, BLACK); 
  } else gfx.TextColor(GRAY, BLACK);
  gfx.Font( 2 );  gfx.TextSize( 1 );
  gfx.print( "V " );
  gfx.print( String( boardVoltage, 1 ) );

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

  //draw board watts
  gfx.MoveTo( 12, 90 );
  if (remoteConnected) gfx.TextColor(ORANGE, BLACK); else gfx.TextColor(GRAY, BLACK);
  gfx.print( "W" );
  sprintf(output,"% 05d",avgInputWatts);
  gfx.print(output);

  // draw odomiter
  gfx.MoveTo(12, 110);
  if (remoteConnected) gfx.TextColor(CYAN, BLACK); else gfx.TextColor(GRAY, BLACK);
  gfx.print("O ");
  gfx.print(String(distanceValue));

  //    successCount++;
  //    gfx.TextColor(CYAN, BLACK); gfx.Font(2);  gfx.TextSize(1);
  //    gfx.println(String(successCount));
  //    gfx.TextColor(RED, BLACK); gfx.Font(2);  gfx.TextSize(1);
  //    gfx.println(String(failCount));

  //draw battery meter
  updateRemoteBattery();

  //draw conection status
  if (remoteConnected){
    if (connBlink) {
      gfx.CircleFilled(65, 145, 8, LIMEGREEN);
    } else  {
      gfx.Circle(65, 145, 8, BLACK);
      gfx.Circle(65, 145, 7, BLACK);
      gfx.Circle(65, 145, 6, BLACK);
    }
    connBlink = !connBlink;
  }else{
      gfx.CircleFilled(65, 145, 8, YELLOW);
  }
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
  
  gfx.RoundRect(2, 135, 52, 155, 3, LIME);    //Draw Remote Battery Meter Border
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

