// Toastershield
// a shield for Arduino Uno
// to use a toaster oven as SDM reflow oven

#define Version 0.1

// pin definitions
#define SW0 9
#define SW1 8

#define FANCTRL 3
#define HEATERCTRL 2

#define ACSENSE A0

//MAX6675 SPI pin definitions
#define CSTC  10               //chip select pin for MAX6675

#define SWSTART SW0
#define SWSTOP SW1


// PID controller definitions
#include <PID_v1.h>
double Setpoint, Input, Output;
#include "PIDparams.h"

// PID controller object
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

unsigned long windowStartTime;

#define PWM_WINDOWSIZE_MS 5000
#define PID_OUTPUT_MAX 800.0
#define PID_OUTPUT_MIN 0.0

// Thermocouple definitions
#include <xMAX6675.h>      

#define MOSI  11    //master out slave in (not connected, only read MAX6675)
#define MISO  12    //master in slave out
#define SCK   13    //serial clock

// thermocouple object
xMAX6675 tc; 

#define tCONV 220   // conversion time is 0.22s per data sheet
#define nMEAS 8     // number of measurements to average

double TempC[nMEAS], TempCAvg;

unsigned char TempCIdx;
unsigned long TempLastMillis;

unsigned long SoakStart, HeatStart;

#include "reflowdefs.h"

#define HEATRAMP_TIMEOUT 200000UL
#define HEATSOAK_TIMEOUT HEATRAMP_TIMEOUT+200000UL
#define HEATREFLOW_TIMEOUT HEATSOAK_TIMEOUT+200000UL

// LCD definitions
#include <Arduino.h>
#include <Wire.h>
#include <MicroLCD.h>
LCD_SSD1306 lcd; /* for SH1106 OLED module */

#define LCD_SIZEX SSD1306_LCDWIDTH
#define LCD_SIZEY SSD1306_LCDHEIGHT

#include "lcdsymbols.h"

#define READY_TEXT    "  READY   "
#define RAMPUP_TEXT   " HEAT UP  "
#define SOAK_TEXT     "   SOAK   "      
#define REFLOWUP_TEXT "  REFLOW  "
#define OPENDOOR_TEXT "OPEN DOOR!"
#define COOLDOWN_TEXT "COOL DOWN "
#define ERROR_TEXT    "  ERROR   "

#define LCD_YPOS_TEMP 1
#define LCD_YPOS_SYMBOL 1
#define LCD_YPOS_BAR 4
#define LCD_YPOS_STATE 6

// Buzzer
#include "pitches.h"
#define BUZZER 6 // pin

#define TL 50
#define PAUSE 250

// delay for relays to turn them on/off tuned for AC zero crossing 
// these values should be good for the relay model
// they can be optimizied through characterization
// for other models they must be changed through characterization
//  or simpe AC oscilloscope measurement of switched voltage
#define HEATERON_DELAY 9
#define HEATEROFF_DELAY 5
#define FANRON_DELAY 9
#define FANOFF_DELAY 3

#define ACSENSETIMEOUT_MS 17 // avoid blocking, when oven is not powered by AC, but USB

unsigned char AcSense(void) {

 unsigned long Millis;

 Millis = millis();
 while (digitalRead(ACSENSE)) {if (millis() > Millis+ACSENSETIMEOUT_MS) return 0;};

 Millis = millis();
 while (!digitalRead(ACSENSE)) {if (millis() > Millis+ACSENSETIMEOUT_MS) return 0;};

 return 1;
}

#define HEATER_ON {if (AcSense()) { delay(HEATERON_DELAY); digitalWrite(HEATERCTRL,1);} lcd.setCursor(8,LCD_YPOS_SYMBOL); lcd.draw(HeaterSymbol16x16,16,16);} 
#define HEATER_OFF {if (AcSense()) { delay(HEATEROFF_DELAY); digitalWrite(HEATERCTRL,0);} lcd.setCursor(8,LCD_YPOS_SYMBOL); lcd.draw(EmptySymbol16x16,16,16);}

#define IF_HEATER_ON digitalRead(HEATERCTRL)

#define FAN_ON {if (AcSense()) {  delay(FANRON_DELAY); digitalWrite(FANCTRL,1);} lcd.setCursor(104,LCD_YPOS_SYMBOL); lcd.draw(FanSymbol16x16,16,16);} 
#define FAN_OFF  {if (AcSense()) { delay(FANOFF_DELAY); digitalWrite(FANCTRL,0);} lcd.setCursor(104,LCD_YPOS_SYMBOL); lcd.draw(EmptySymbol16x16,16,16);}

#define IF_FAN_ON digitalRead(FANCTRL)

enum ReflowFsmState {READY, STARTPID, RAMPUP, SOAK, REFLOWUP, OPENDOOR, COOLDOWN, ERROR} ReflowFsm;

void alarm() {
 tone(BUZZER, NOTE_B7);
 delay(TL);
 tone(BUZZER, NOTE_D7);
 delay(TL);
 tone(BUZZER, NOTE_F7);
 delay(TL);
 tone(BUZZER, NOTE_D7);
 delay(TL);
 tone(BUZZER, NOTE_B7);
 delay(TL);
   
 noTone(BUZZER);
}

void beep () {
 tone(BUZZER, NOTE_B7);
 delay(TL);
 noTone(BUZZER);
}

void errorbeep () {
 tone(BUZZER, NOTE_F7);
 delay(TL);
 noTone(BUZZER);
}

bool pressed(unsigned char pbswitch) {

 if (!digitalRead(pbswitch)) {
   beep();
   while (!digitalRead(pbswitch));
   return true;
 } else {
   return false;
 }
}

void wrongpressed(unsigned char pbswitch) {

 unsigned int vandalcheck = 0; 

 if (!digitalRead(pbswitch)) {
  errorbeep();
  for (;(vandalcheck < 10000) && !digitalRead(pbswitch); vandalcheck++);
 }

}

// average function
double Avg (double Vals[]) {

 unsigned char n;
 double Sum = 0.0;
// sum values   
 for (n = 0; n < nMEAS; n++) Sum += Vals[n];
 return (Sum/(double)nMEAS);
}

#define IF_STOP {if (pressed(SWSTOP)) ReflowFsm = COOLDOWN;}
#define WRONG_STOP {wrongpressed(SWSTOP); }
#define WRONG_START {wrongpressed(SWSTART); }

void setup() {

 unsigned char n;
 unsigned long Millis;

 Millis = millis();
  
 tc.attach(MOSI,MISO,SCK,CSTC); // attach the thermocouple

// switches pull up
 pinMode(SW0, INPUT_PULLUP);  
 pinMode(SW1, INPUT_PULLUP);  

 pinMode(ACSENSE,INPUT);

// relays
 digitalWrite(FANCTRL,0);
 pinMode(FANCTRL,OUTPUT);

 digitalWrite(HEATERCTRL,0);
 pinMode(HEATERCTRL,OUTPUT);

// buzzer
 pinMode(BUZZER,OUTPUT);

 delay(100);  // otherwise OLED does not start
// LCD
 lcd.begin();
 lcd.clear();
 lcd.setCursor(40, 3);
 lcd.setFontSize(FONT_SIZE_XLARGE);
 lcd.print("V "); lcd.print(Version);
 delay(1000);
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.draw(logo, 128, 64);
 errorbeep();
 delay(100);
 beep();
 delay(500);


// PID controller
 Setpoint = PID_OUTPUT_MIN;
 Output = 0.0; 
 Input = 0.0;
 windowStartTime = Millis;

//tell the PID to range between 0 and the full window size
 myPID.SetOutputLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);

//we do not turn on the PID for now
 myPID.SetMode(MANUAL);

 for (n = 0; n < nMEAS;n++) {
  delay(tCONV);
  TempC[n] = tc.readC();
 }
 TempCAvg = Avg(TempC);
 TempCIdx = 0;

 TempLastMillis = Millis;

 ReflowFsm = READY;
  
 lcd.clear();
}

void loop() {

 unsigned long Millis;
 unsigned char CursorX;

 Millis = millis();

// LCD display output
 lcd.setCursor(24,LCD_YPOS_STATE); //position for state
// FSM output decoder  for display
 switch (ReflowFsm) {
  case READY:
   lcd.print(F(READY_TEXT));
   lcd.setCursor((EMPTYBAR_SIZEX-LCD_SIZEX)/2,LCD_YPOS_BAR); lcd.draw(emptybar,EMPTYBAR_SIZEX,EMPTYBAR_SIZEY);
   break;
  case STARTPID:
  case RAMPUP:
   lcd.print(F(RAMPUP_TEXT));
   break;
  case SOAK:
   lcd.print(F(SOAK_TEXT));   
   CursorX = (unsigned char)((double)(Millis - SoakStart)/(double)SoakTime*double(EMPTYBAR_SIZEX/FULLBARSTEP_SIZEX))*FULLBARSTEP_SIZEX;
   if (CursorX >= FULLBARSTEP_SIZEX) { lcd.setCursor(CursorX-FULLBARSTEP_SIZEX,LCD_YPOS_BAR);lcd.draw(fullbarstep,FULLBARSTEP_SIZEX,FULLBARSTEP_SIZEY);}
   break;
  case REFLOWUP:
   lcd.print(F(REFLOWUP_TEXT));
   lcd.setCursor(0,LCD_YPOS_BAR); lcd.draw(emptybar,EMPTYBAR_SIZEX,EMPTYBAR_SIZEY);
   break;
  case OPENDOOR:
   lcd.print(F(OPENDOOR_TEXT));
   break;
  case COOLDOWN:
   lcd.print(F(COOLDOWN_TEXT));
   lcd.setCursor(0,LCD_YPOS_BAR); lcd.draw(emptybar,EMPTYBAR_SIZEX,EMPTYBAR_SIZEY);
   break;
  case ERROR:
   lcd.print(F(ERROR_TEXT));
   break;
 }




 if (Millis > TempLastMillis+tCONV) {  // read only if at least tCONV ha elapsed
// read temperature
  TempC[TempCIdx] = tc.readC();
  if (TempCIdx == (nMEAS-1)) TempCIdx = 0; else TempCIdx++;
// calculate new average
  TempCAvg = Avg(TempC);
// save last time stamp
  TempLastMillis = Millis;
  lcd.setCursor(44,LCD_YPOS_TEMP); lcd.print(TempCAvg,0); lcd.print(" C "); 
}  
 

 Input = TempCAvg;  // Temp value for PID controller

// FSM output decoder  
 switch (ReflowFsm) {
  case READY:
   WRONG_STOP;
   FAN_OFF; 
   HEATER_OFF;
   break;
  case STARTPID:
   Setpoint = PreheatTemp;    
   Output = 0.0; // this value needs to be reset before PID is turned on
   myPID.SetMode(AUTOMATIC); // turn on the PID, initializes controller
   windowStartTime = Millis; // initialize PWM element
   HeatStart = Millis; // log time when heat is applied for error condition
   break;
  case RAMPUP:
  case SOAK:
   WRONG_START;
   FAN_ON;
   myPID.Compute();
   if ((Millis - windowStartTime) >= (unsigned long) PWM_WINDOWSIZE_MS) { //time to shift the Relay Window
    windowStartTime += PWM_WINDOWSIZE_MS;
   }
   if ((unsigned long)(Output/PID_OUTPUT_MAX*(double)PWM_WINDOWSIZE_MS) > (Millis - windowStartTime)) { 
    HEATER_ON;
   } else {
    HEATER_OFF;
   }
   break;
  case REFLOWUP:
   WRONG_START;
   myPID.SetMode(MANUAL); // turn PID off
   FAN_ON;
   HEATER_ON;
  break;
  case OPENDOOR:
   Setpoint = PID_OUTPUT_MIN;
   FAN_ON;
   HEATER_OFF;
   alarm();
   break;
  case COOLDOWN:
   WRONG_START;
   WRONG_STOP;
   Setpoint = PID_OUTPUT_MIN;
   myPID.SetMode(MANUAL); // turn PID off
   FAN_ON;
   HEATER_OFF;
   break;
  case ERROR:
   WRONG_START;
   WRONG_STOP;
   FAN_ON;
   HEATER_OFF;
   errorbeep(); delay (500);
   break;
 }


// FSM condition, next state  
 switch (ReflowFsm) {
  case READY:
   if (TempCAvg > ReadyTemp) ReflowFsm = COOLDOWN; 
   else if (pressed(SWSTART)) ReflowFsm = STARTPID;
   break;
  case STARTPID:
   ReflowFsm = RAMPUP;
   break;
  case RAMPUP:
   IF_STOP;
   SoakStart = Millis; // save start time
   if (TempCAvg >= SoakTemp) ReflowFsm = SOAK;
   if (Millis-HeatStart >= HEATRAMP_TIMEOUT) ReflowFsm = ERROR; 
   break;
  case SOAK: 
   IF_STOP;
   if (pressed(SWSTOP)) ReflowFsm = COOLDOWN;
   if ((Millis - SoakStart) >= SoakTime) ReflowFsm = REFLOWUP;
   if (Millis-HeatStart >= HEATSOAK_TIMEOUT) ReflowFsm = ERROR;
   break;
  case REFLOWUP:
   IF_STOP;
   if (TempCAvg >= ReflowTempMax) ReflowFsm = OPENDOOR; 
   if (Millis-HeatStart >= HEATREFLOW_TIMEOUT) ReflowFsm = ERROR;
   break;
  case OPENDOOR:
   if (pressed(SWSTART) || pressed(SWSTOP)) ReflowFsm = COOLDOWN;
   break;
  case COOLDOWN:
   if (TempCAvg < ReadyTemp-FanHysteresis) ReflowFsm = READY;
   break;   
  case ERROR:
   break;
 }

}




