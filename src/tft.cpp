#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "sensorNTC.h"
#include "temperatureController.h"
#include "tft.h"
#include "WiFi.h"

#ifdef DRIVER_ILI9341
#include <Adafruit_ILI9341.h>
#include <FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#endif
#ifdef DRIVER_ST7735
#include <Adafruit_ST7735.h>
#endif

//prepare driver for display
#ifdef DRIVER_ILI9341
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
#endif
#ifdef DRIVER_ST7735
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
#endif

#ifdef useTFT
const GFXfont *myFont;
const GFXfont *myFontM;
const GFXfont *myFontB;
int textSizeOffset;

// number of screen to display
int screen = SCREEN_NORMALMODE;
int lastRSSI;

unsigned long startCountdown = 0;

void calcDimensionsOfElements(void);
void draw_screen(void);
#endif

void initTFT(void) {
  #ifdef useTFT
  // start driver
  #ifdef DRIVER_ILI9341
  // switch display on
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, LED_ON);
  tft.begin();
  myFont = &FreeSans9pt7b;
  myFontM = &FreeMono9pt7b;
  myFontB = &FreeSansBold9pt7b;
  textSizeOffset = 0;
  lastRSSI = 9;
  #endif
  #ifdef DRIVER_ST7735
  tft.initR(INITR_BLACKTAB);
  myFont = NULL;
  textSizeOffset = 0;
  #endif

  tft.setFont(myFont);
  tft.setRotation(TFT_ROTATION); 

  calcDimensionsOfElements();

  // clear screen
  tft.fillScreen(TFT_BLACK);
  draw_screen();

  // show the displays resolution
  Log.printf("  TFT sucessfully initialized.\r\n");
  Log.printf("  tftx = %d, tfty = %d\r\n", tft.width(), tft.height());

  #else
  Log.printf("    TFT is disabled in config.h\r\n");
  #endif
}

#ifdef useTFT
int16_t getRelativeX(int16_t xBasedOnTFTwithScreenWidth320px) {
  return (float)(xBasedOnTFTwithScreenWidth320px) /(float)(320) * tft_getWidth();;
}

int16_t getRelativeY(int16_t yBasedOnTFTwithScreenHeight240px) {
  return (float)(yBasedOnTFTwithScreenHeight240px)/(float)(240) * tft_getHeight();;
}

// rect: x, y, width, heigth
int valueUpRect[4];
int valueDownRect[4];
#if defined (useStandbyButton) || defined(useShutdownButton)
int shutdownRect[4];
int confirmShutdownYesRect[4];
int confirmShutdownNoRect[4];
#endif

int plusMinusHorizontalLineMarginLeft;
int plusMinusHorizontalLineMarginTop;
int plusMinusHorizontalLineLength;
int plusMinusVerticalLineMarginTop;
int plusMinusVerticalLineLength;
int plusMinusVerticalLineMarginLeft;

int tempAreaLeft; int tempAreaTop; int tempAreaWidth;
int fanAreaLeft; int fanAreaTop; int fanAreaWidth;
int ambientAreaLeft; int ambientAreaTop; int ambientAreaWidth;

#if defined (useStandbyButton) || defined(useShutdownButton)
int shutdownWidthAbsolute;
int shutdownHeightAbsolute;
#endif

void calcDimensionsOfElements(void) {
  // upper left corner is 0,0
  // width and heigth are only valid for landscape (rotation=1) or landscape upside down (rotation=3)
  //                       ILI9341   ST7735
  //                       AZ-Touch
  // tft.width   0 <= x <  320       160
  // tft.height  0 <= y <  240       128

  // ALL VALUES ARE BASED ON A 320x240 DISPLAY and automatically resized to the actual display size via getRelativeX() and getRelativeYI()
  int marginTopAbsolute   = 12;
  int marginLeftAbsolute  = 14;
  // int areaHeightAbsolute  = 64;           // make sure: 4*marginTopAbsolute + 3*areaHeightAbsolute = 240
  int areaHeightAbsolute  = (240 - 4*marginTopAbsolute) / 3;

  int valueUpDownWidthAbsolute  = 80;
  int valueUpDownHeightAbsolute = 55;
  #if defined (useStandbyButton) || defined(useShutdownButton)
      shutdownWidthAbsolute     = 40;
      shutdownHeightAbsolute    = 40;
  #endif
  int valueUpRectTop;
  int valueDownRectTop;
  #if defined (useStandbyButton) || defined(useShutdownButton)
  int shutdownRectTop;
  #endif

  tempAreaLeft     = getRelativeX(marginLeftAbsolute);
  fanAreaLeft      = getRelativeX(marginLeftAbsolute);
  ambientAreaLeft = getRelativeX(marginLeftAbsolute);
  tempAreaTop     = getRelativeY(marginTopAbsolute) + 15;
  fanAreaTop      = getRelativeY(marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute) + 20;
  valueUpRectTop   = fanAreaTop;
  ambientAreaTop = getRelativeY(marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute+areaHeightAbsolute+marginTopAbsolute ) + 30;
  valueDownRectTop = ambientAreaTop;
    #if defined (useStandbyButton) || defined(useShutdownButton)
    tempAreaWidth     = getRelativeX(320-marginLeftAbsolute - shutdownWidthAbsolute-marginLeftAbsolute);    // screen - marginleft - [Area] - 40 shutdown - marginright
    #else
    tempAreaWidth     = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    #endif
    #ifdef useTouch
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute); // screen - marginleft - [Area] - 80 up/down  - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute);
    #else
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute -    0);
    #endif
    #if defined (useStandbyButton) || defined(useShutdownButton)
    shutdownRectTop   = getRelativeY(marginTopAbsolute);
    #endif
  
    #ifdef useTouch
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute); // screen - marginleft - [Area] - 80 up/down  - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute - valueUpDownWidthAbsolute-marginLeftAbsolute);
    #else
    fanAreaWidth      = getRelativeX(320-marginLeftAbsolute -    0);                                        // screen - marginleft - [Area]               - marginright
    ambientAreaWidth = getRelativeX(320-marginLeftAbsolute -    0);
    #endif
    #if defined (useStandbyButton) || defined(useShutdownButton)
    shutdownRectTop   = getRelativeY(240-shutdownHeightAbsolute-marginTopAbsolute);
    #endif
  #endif

  valueUpRect[0] = getRelativeX(320-valueUpDownWidthAbsolute-marginLeftAbsolute);
  valueUpRect[1] = valueUpRectTop;
  valueUpRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueUpRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  valueDownRect[0] = getRelativeX(320-valueUpDownWidthAbsolute-marginLeftAbsolute);
  valueDownRect[1] = valueDownRectTop;
  valueDownRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueDownRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  plusMinusHorizontalLineLength     = (valueUpRect[2] / 2) - 4; // 36
  plusMinusHorizontalLineMarginLeft = (valueUpRect[2] - plusMinusHorizontalLineLength) / 2; // 22
  plusMinusHorizontalLineMarginTop  = valueUpRect[3] / 2; // 27

  plusMinusVerticalLineLength       = plusMinusHorizontalLineLength; // 36
  plusMinusVerticalLineMarginTop    = (valueUpRect[3] - plusMinusVerticalLineLength) / 2; // 9
  plusMinusVerticalLineMarginLeft   = valueUpRect[2] / 2; // 40

  #if defined (useStandbyButton) || defined(useShutdownButton)
  shutdownRect[0] = getRelativeX(320-shutdownWidthAbsolute-marginLeftAbsolute);
  shutdownRect[1] = shutdownRectTop;
  shutdownRect[2] = getRelativeX(shutdownWidthAbsolute);
  shutdownRect[3] = getRelativeY(shutdownHeightAbsolute);

  confirmShutdownYesRect[0]  = getRelativeX(40);
  confirmShutdownYesRect[1]  = getRelativeY(90);
  confirmShutdownYesRect[2]  = getRelativeX(60);
  confirmShutdownYesRect[3]  = getRelativeY(60);
  confirmShutdownNoRect[0]   = getRelativeX(200);
  confirmShutdownNoRect[1]   = getRelativeY(90);
  confirmShutdownNoRect[2]   = getRelativeX(60);
  confirmShutdownNoRect[3]   = getRelativeY(60);
  #endif
}

int16_t tft_getWidth(void) {
  return tft.width();
}

int16_t tft_getHeight(void) {
  return tft.height();
}

void tft_fillScreen(void) {
  tft.fillScreen(TFT_BLACK);
};

/*
https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
https://www.heise.de/ratgeber/Adafruit-GFX-Library-Einfache-Grafiken-mit-ESP32-und-Display-erzeugen-7546653.html?seite=all
https://www.heise.de/select/make/2023/2/2304608284785808657
                        AZ-Touch
              ST7735    ILI9341
  TextSize    Standard  FreeSans9pt7b FreeSerif9pt7b  FreeMono9pt7b  FreeSerifBoldItalic24pt7b
              y1/h
  1 pwm hum   0/8       -12/17        -11/16          -9/13          -30/40
  2 temp      0/16      -24/34        -22/32          -18/26         -60/80
  3           0/24      -36/51        -33/48          -27/39         -90/120
  4 countdown 0/32      -48/68        -44/64          -36/52         -120/160
  8           0/64      -96/136       -88/128         -72/104        -240/320
  15          0/120     -180/255      -165240         -135/195       -450/600
*/
void printText(int areaX, int areaY, int areaWidth, int lineNr, const char *str, uint8_t textSize, const GFXfont *f, bool wipe, int foreground) {
  // get text bounds
  GFXcanvas1 testCanvas(tft_getWidth(), tft_getHeight());
  int16_t x1; int16_t y1; uint16_t w; uint16_t h;
  testCanvas.setFont(f);
  testCanvas.setTextSize(textSize);
  testCanvas.setTextWrap(false);
  testCanvas.getTextBounds("0WIYgy,", 0, 0, &x1, &y1, &w, &h);
  // Log.printf("  x1 = %d, y1 = %d, w=%d, h=%d\r\n", x1, y1, w, h);
  int textHeight = h;
  int textAreaHeight = textHeight +2; // additional 2 px as vertical spacing between lines
  // y1=0 only for standardfont, with every other font this value gets negative!
  // This means that when using standarfont at (0,0), it really gets printed at 0,0.
  // With every other font, printing at (0,0) means that text starts at (0, y1) with y1 being negative!
  int textAreaOffset = -y1;
  // Don't know why, but a little additional correction has to be done for every font other than standard font. Doesn't work perfectly, sometimes position is wrong by 1 pixel
  // if (textAreaOffset != 0) {
  //   textAreaOffset = textAreaOffset + textSize;
  // }

  // Version 1: flickering, but faster
  #ifdef useTouch
  tft.setFont(f);
  tft.setTextSize(textSize);
  tft.setTextWrap(false);
  if (wipe) {
    tft.fillRect (areaX, areaY + lineNr*textAreaHeight, areaWidth, textAreaHeight, TFT_BLACK);
  }
  tft.setCursor(areaX, areaY + textAreaOffset + lineNr*textAreaHeight);
  tft.printf(str);
  #endif

  // Version 2: flicker-free, but slower. Touch becomes unusable.
  #ifndef useTouch
  GFXcanvas1 canvas(areaWidth, textAreaHeight);
  canvas.setFont(f);
  canvas.setTextSize(textSize);
  canvas.setTextWrap(false);
  canvas.setCursor(0, textAreaOffset);
  canvas.println(str);
  tft.drawBitmap(areaX, areaY + lineNr*textAreaHeight, canvas.getBuffer(), areaWidth, textAreaHeight, foreground, TFT_BLACK);
  #endif
}

void switchOff_screen(boolean switchOff) {
  #ifdef useTFT
  if (switchOff) {
    Log.printf("  Will switch TFT off.\r\n");
    digitalWrite(TFT_LED, !LED_ON);
    // if digitalWrite does not work for your screen, then try: tft_fillScreen();
  } else {
    Log.printf("  Will switch TFT on.\r\n");
    digitalWrite(TFT_LED, LED_ON);
    // if digitalWrite does not work for your screen, then try: draw_screen();
  }
  #endif
}

void draw_screen(void) {
  if (modeIsOff) {
    return;
  }
  #ifdef useTFT
  char buffer[100];
  char percentEscaped = 0x25;
  char degreesSymbol = 0x7F;

  if (screen == SCREEN_NORMALMODE) {
    int color;
    // temperature
    color = TFT_WHITE;
    sprintf(buffer, "Radiator Temperature:");
    printText(tempAreaLeft, tempAreaTop - 25, tempAreaWidth - 50, 0, buffer,      textSizeOffset + 1, myFontB, false, color);
    if (temperature[0] <= temperatureMin) color = TFT_GREEN;
    else if (temperature[0] <= temperatureMin + (temperatureMax-temperatureMin)/2) color = TFT_YELLOW;
    else color = TFT_RED;
    sprintf(buffer, "%.1f", temperature[0]);
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth - 230, 0, buffer, textSizeOffset + 2, myFont, true, color);
    sprintf(buffer, "%.1f", temperature[1]);
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth - 230, 1, buffer, textSizeOffset + 2, myFont, true, color);
    color = TFT_WHITE;
    sprintf(buffer, "%cC High", degreesSymbol);
    printText(tempAreaLeft + 70, tempAreaTop, tempAreaWidth - 150, 0, buffer, textSizeOffset + 2, myFont, false, color);
    sprintf(buffer, "%cC Low", degreesSymbol);
    printText(tempAreaLeft + 70, tempAreaTop, tempAreaWidth, 1, buffer, textSizeOffset + 2, myFont, false, color);
    
    // sprintf(buffer, "%d/255 PWM (%d%s)", getPWMvalue(), (100*getPWMvalue())/255, percentEscaped.c_str());
    // printText(tempAreaLeft, tempAreaTop, tempAreaWidth, 2, buffer, textSizeOffset + 2, myFont, true);
  
    // fan
    printText(fanAreaLeft, fanAreaTop, fanAreaWidth, 0, "RPM:    Load:     Duty / Target:", textSizeOffset + 1, myFontB, false, TFT_WHITE);
    for (int i = 0; i < 3; i++) {
      int load = 100*rpm[i]/FANMAXRPM1;
      if (load < 1) color = TFT_WHITE;
      else if (load < 50) color = TFT_GREEN;
      else if (load < 80) color = TFT_YELLOW;
      else color = TFT_RED;
      sprintf(buffer, "%4d (%3d%c) %3d / %3d (%3d%c)", rpm[i], load, percentEscaped, pwmValue, pwmTarget, (100*pwmValue)/255, percentEscaped);
      printText(fanAreaLeft, fanAreaTop + 8, fanAreaWidth, i+1, buffer, textSizeOffset + 1, myFontM, true, color);
    }
    
    // footer
    int ambientLine = 0;
    color = TFT_WHITE;
    sprintf(buffer, "Flow Rate:");
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth - 210, ambientLine, buffer,      textSizeOffset + 1, myFontB, false, color);
    if (rpm[3] < 400) color = TFT_RED;
    else color = TFT_GREEN;
    sprintf(buffer, "%.0f L/hour", rpm[3]*60*0.01);
    printText(ambientAreaLeft + 100, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 2, myFont, true, color);
    /*
    color = TFT_WHITE;
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, "Parameters:", textSizeOffset + 1, myFontB, false, color);
    sprintf(buffer, "%.1f-%.1f %cC Min-Max temperature", getMinTemperature(),getMaxTemperature(), degreesSymbol);
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true, color);
    // sprintf(buffer, "%.1f Min temperature", getMinTemperature());
    // printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true);
    sprintf(buffer, "%.1f C Offset temperature", getOffsetTemperature());
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer,      textSizeOffset + 1, myFont, true, color);
    */
    
    // WiFi RSSI
    int8_t rssi = WiFi.RSSI();
    // sprintf(buffer, "RSSI: %s", (String)rssi);
    // printText(getRelativeX(260), tempAreaTop, tempAreaWidth, 0, buffer,      textSizeOffset + 1, myFont, true);
    int xOffset = 190;
    if (rssi >= -55) rssi = 5;  
    else if (rssi < -55 & rssi > -65) rssi = 4;
    else if (rssi < -65 & rssi > -75) rssi = 3;
    else if (rssi < -75 & rssi > -85) rssi = 2;
    else if (rssi < -85 & rssi > -96) rssi = 1;
    else rssi = 0;
    if (!WiFi.isConnected()) rssi =0;
    if (lastRSSI != rssi) {
      int yBar1 = 18;
      int yBar2 = 16;
      int yBar3 = 12;
      int yBar4 = 8;
      int yBar5 = 4;
      if (rssi == 5) { 
        tft.fillRect(xOffset + 102,yBar1,4,2 , TFT_GREEN);
        tft.fillRect(xOffset + 107,yBar2,4,4 , TFT_GREEN);
        tft.fillRect(xOffset + 112,yBar3,4,8 , TFT_GREEN);
        tft.fillRect(xOffset + 117,yBar4,4,12, TFT_GREEN);
        tft.fillRect(xOffset + 122,yBar5,4,16, TFT_GREEN);
      } else if (rssi == 4) {
        tft.fillRect(xOffset + 102,yBar1,4,2 , TFT_GREEN);
        tft.fillRect(xOffset + 107,yBar2,4,4 , TFT_GREEN);
        tft.fillRect(xOffset + 112,yBar3,4,8 , TFT_GREEN);
        tft.fillRect(xOffset + 117,yBar4,4,12, TFT_GREEN);
        tft.drawRect(xOffset + 122,yBar5,4,16, TFT_GREEN);
      } else if (rssi == 3) {
        tft.fillRect(xOffset + 102,yBar1,4,2 , TFT_YELLOW);
        tft.fillRect(xOffset + 107,yBar2,4,4 , TFT_YELLOW);
        tft.fillRect(xOffset + 112,yBar3,4,8 , TFT_YELLOW);
        tft.drawRect(xOffset + 117,yBar4,2,12, TFT_YELLOW);
        tft.drawRect(xOffset + 122,yBar5,4,16, TFT_YELLOW);
      } else if (rssi == 2) {
        tft.fillRect(xOffset + 102,yBar1,4,2 , TFT_YELLOW);
        tft.fillRect(xOffset + 107,yBar2,4,4 , TFT_YELLOW);
        tft.drawRect(xOffset + 112,yBar3,4,8 , TFT_YELLOW);
        tft.drawRect(xOffset + 117,yBar4,4,12, TFT_YELLOW);
        tft.drawRect(xOffset + 122,yBar5,4,16, TFT_YELLOW);
      } else if (rssi == 1) {
        tft.fillRect(xOffset + 102,yBar1,4,2 , TFT_RED);
        tft.drawRect(xOffset + 107,yBar2,4,4 , TFT_RED);
        tft.drawRect(xOffset + 112,yBar3,4,8 , TFT_RED);
        tft.drawRect(xOffset + 117,yBar4,4,12, TFT_RED);
        tft.drawRect(xOffset + 122,yBar5,4,16, TFT_RED);
      } else {
        tft.drawRect(xOffset + 102,yBar1,4,2 , TFT_RED);
        tft.drawRect(xOffset + 107,yBar2,4,4 , TFT_RED);
        tft.drawRect(xOffset + 112,yBar3,4,8 , TFT_RED);
        tft.drawRect(xOffset + 117,yBar4,4,12, TFT_RED);
        tft.drawRect(xOffset + 122,yBar5,4,16, TFT_RED);
      }
      lastRSSI = rssi;
    }

    #ifdef useTouch
    // increase temperature or pwm
    tft.fillRoundRect(valueUpRect[0],   valueUpRect[1],    valueUpRect[2],   valueUpRect[3],   4, TFT_GREEN);
    //   plus sign
    //     horizontal line
    tft.drawLine(valueUpRect[0]+plusMinusHorizontalLineMarginLeft, valueUpRect[1]+plusMinusHorizontalLineMarginTop,   valueUpRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueUpRect[1]+plusMinusHorizontalLineMarginTop,   TFT_BLACK);
    tft.drawLine(valueUpRect[0]+plusMinusHorizontalLineMarginLeft, valueUpRect[1]+plusMinusHorizontalLineMarginTop+1, valueUpRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueUpRect[1]+plusMinusHorizontalLineMarginTop+1, TFT_BLACK);
    //     vertical line
    tft.drawLine(valueUpRect[0]+plusMinusVerticalLineMarginLeft,   valueUpRect[1]+plusMinusVerticalLineMarginTop, valueUpRect[0]+plusMinusVerticalLineMarginLeft,   valueUpRect[1]+plusMinusVerticalLineMarginTop + plusMinusVerticalLineLength, TFT_BLACK);
    tft.drawLine(valueUpRect[0]+plusMinusVerticalLineMarginLeft+1, valueUpRect[1]+plusMinusVerticalLineMarginTop, valueUpRect[0]+plusMinusVerticalLineMarginLeft+1, valueUpRect[1]+plusMinusVerticalLineMarginTop + plusMinusVerticalLineLength, TFT_BLACK);
    // decrease temperature or pwm
    tft.fillRoundRect(valueDownRect[0], valueDownRect[1],  valueDownRect[2], valueDownRect[3], 4, TFT_GREEN);
    //   minus sign
    //     horizontal line
    tft.drawLine(valueDownRect[0]+plusMinusHorizontalLineMarginLeft, valueDownRect[1]+plusMinusHorizontalLineMarginTop,   valueDownRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueDownRect[1]+plusMinusHorizontalLineMarginTop,   TFT_BLACK);
    tft.drawLine(valueDownRect[0]+plusMinusHorizontalLineMarginLeft, valueDownRect[1]+plusMinusHorizontalLineMarginTop+1, valueDownRect[0]+plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueDownRect[1]+plusMinusHorizontalLineMarginTop+1, TFT_BLACK);
    #endif

    #if defined (useStandbyButton) || defined(useShutdownButton)
    // shutdown button
    // square, without round corners
    // tft.fillRect(shutdownRect[0],   shutdownRect[1],    shutdownRect[2],   shutdownRect[3],   TFT_RED);
    // round corners, inner part
    tft.fillRoundRect(shutdownRect[0], shutdownRect[1], shutdownRect[2], shutdownRect[3], getRelativeX(4), TFT_RED);
    // round corners, outer line
    // tft.drawRoundRect(shutdownRect[0], shutdownRect[1], shutdownRect[2], shutdownRect[3], 4, TFT_WHITE);
    tft.drawCircle(shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/2), getRelativeX(shutdownWidthAbsolute*0.375),   TFT_WHITE);
    tft.drawCircle(shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/2), getRelativeX(shutdownWidthAbsolute*0.375)-1, TFT_WHITE);
    tft.drawLine(  shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/4), shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2),   shutdownRect[1]+getRelativeY(shutdownHeightAbsolute*3/4), TFT_WHITE);
    tft.drawLine(  shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2)+1, shutdownRect[1]+getRelativeY(shutdownHeightAbsolute/4), shutdownRect[0]+getRelativeX(shutdownWidthAbsolute/2)+1, shutdownRect[1]+getRelativeY(shutdownHeightAbsolute*3/4), TFT_WHITE);
    #endif
  #ifdef useShutdownButton
  } else if (screen == SCREEN_CONFIRMSHUTDOWN) {
    printText(getRelativeX(44), getRelativeY(50), getRelativeX(200), 0, "Please confirm shutdown",      textSizeOffset + 1, myFont, false);

    // confirm: yes
    tft.fillRoundRect(confirmShutdownYesRect[0], confirmShutdownYesRect[1], confirmShutdownYesRect[2], confirmShutdownYesRect[3], 4, TFT_RED);
    printText(confirmShutdownYesRect[0]+getRelativeX(12), confirmShutdownYesRect[1] + getRelativeY(22), getRelativeX(200), 0, "Yes",      textSizeOffset + 1, myFont, false);
    // confirm: no
    tft.fillRoundRect(confirmShutdownNoRect[0],  confirmShutdownNoRect[1],  confirmShutdownNoRect[2],  confirmShutdownNoRect[3],  4, TFT_GREEN);
    printText(confirmShutdownNoRect[0]+ getRelativeX(18), confirmShutdownNoRect[1]  + getRelativeY(22), getRelativeX(200), 0, "No",       textSizeOffset + 1, myFont, false);
  } else if (screen == SCREEN_COUNTDOWN) {
    float floatSecondsSinceShutdown = (unsigned long)(millis()-startCountdown) / 1000;
    // rounding
    floatSecondsSinceShutdown = floatSecondsSinceShutdown + 0.5 - (floatSecondsSinceShutdown<0);
    // convert float to int
    int intSecondsSinceShutdown = (int)floatSecondsSinceShutdown;

    // clear screen
    tft.fillScreen(TFT_BLACK);
    sprintf(buffer, "%d", SHUTDOWNCOUNTDOWN - intSecondsSinceShutdown);
    printText(getRelativeX(115), getRelativeY(80), getRelativeX(200), 0, buffer,       textSizeOffset + 4, myFont, false);

    if ((unsigned long)(millis() - startCountdown) > SHUTDOWNCOUNTDOWN*1000 + 15000){
      // if EPS32 is still alive, which means power is still available, then stop countdown and go back to normal mode
      Log.printf("hm, still alive? Better show mainscreen again\r\n");
      screen = SCREEN_NORMALMODE;
      // clear screen
      tft.fillRect(0, 0, 320, 240, TFT_BLACK);
      draw_screen();
    }
  #endif
  }
  #endif
}
