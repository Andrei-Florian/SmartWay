// Universum | Universum Projects > SmartWay

// Andrei Florian 5/NOV/2018 - 10/NOV/2018

#include <SigFox.h> // include SigFox library
#include <string.h>
#include <ctype.h> // include processing library
#include <RTCZero.h> // include library for internal RTC
#include <TinyGPS++.h> // include library for GPS communication
#include <stdio.h>

#include "functions.h"

// Initialise SigFox
int debug = false; // set debug to false
bool proDebug = true; // set to Zero to stop printing - EDIT

// Initialise RTC
RTCZero rtc; // create object
int resetTime = 1; // reset the time when the board starts

byte seconds;
byte minutes;
byte hours;

byte day;
byte month;
byte year;

// GPS Module
int baudGPS = 9600;

float latitude;
float longitude;

TinyGPSPlus gps;

// school start
int schoolHrs = 9; int schoolMins = 0;

// school finish
int homeHrs = 19; int homeMins = 30;

// travel time
int travelMins = 30;

// home co-ordinates
float homeX = 53.357929;
float homeY = -6.258268;

// school co-ordinates
float schoolX = 53.355504;
float schoolY = -6.258452;

// grid co-ordinates
float g1;
float g2;
float g3;
float g4;

float h1;
float h2;
float h3;
float h4;

float s1;
float s2;
float s3;
float s4;

// other
bool arrivedHome = true;
bool arrivedSchool = false;
bool offTrackVar;
bool lateVar;

int pNow;
int pHome;
int pSchool;

struct Check
{
  bool isHome()
  {
    if(latitude <= h2 && latitude >= h4 &&
       longitude <= h3 && longitude >= h1)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool isSchool()
  {
    if(latitude <= s4 && latitude >= s2 &&
       longitude <= s1 && longitude >= s3)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool isOut()
  {
    if(latitude <= g2 && latitude >= g4 &&
       longitude <= g3 && longitude >=g1)
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  bool track()
  {
    pNow = ((rtc.getHours() * 60) + rtc.getMinutes());
    pSchool = ((schoolHrs * 60) + schoolMins);
    pHome = ((homeHrs * 60) + homeMins);

    if(proDebug)
    {
      Serial.print("p.Now      "); Serial.println(pNow);
      Serial.print("p.School   "); Serial.println(pSchool);
      Serial.print("p.Home     "); Serial.println(pHome);
    }
    
    if(pNow > (pSchool - travelMins) && pNow < (pHome + travelMins))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  
  bool isSchoolTime()
  {
    pNow = ((rtc.getHours() * 60) + rtc.getMinutes());
    pSchool = ((schoolHrs * 60) + schoolMins);

    if(pNow > pSchool)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool isHomeTime()
  {
    pNow = ((rtc.getHours() * 60) + rtc.getMinutes());
    pHome = ((homeHrs * 60) + homeMins);

    if(pNow > pHome)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
};

struct Do
{
  void offTrackLoop()
  {
    if(offTrackVar)
    {
      parseData(0);
    }
    
    vibrateMotor();
    ledRed();
    delay(1000);
    vibrateMotor();
    resetLED();
    offTrackVar = true;
    delay(120000);
  }

  void lateLoop()
  {
    if(!lateVar)
    {
      parseData(1);
    }
    
    vibrateMotor();
    ledOrange();
    delay(1000);
    resetLED();
    lateVar = true;
  }

  void arrivedHomeLoop()
  {
    lateVar = false;
    offTrackVar = false;

    vibrateMotor();
    ledGreen();
    delay(1000);
    resetLED();
    parseData(2);
    
    arrivedHome = true;
    arrivedSchool = false;
  }

  void arrivedSchoolLoop()
  {
    lateVar = false;
    offTrackVar = false;

    vibrateMotor();
    ledGreen();
    delay(1000);
    resetLED();
    parseData(3);
    
    arrivedHome = false;
    arrivedSchool = true;
  }
};

Check check;
Do warn;

void reboot() 
{
  Serial.println("  Restarting Device");
  NVIC_SystemReset();
  while(1);
}

void generateGrids()
{
  // generate home
  h1 = homeY - 0.001;
  h2 = homeX + 0.001;
  h3 = homeY + 0.001;
  h4 = homeX - 0.001;

  // generate school
  s1 = schoolY + 0.001;
  s2 = schoolX - 0.001;
  s3 = schoolY - 0.001;
  s4 = schoolX + 0.001;

  // generate grid
  if(homeY > schoolY)
  {
    g1 = homeY - 0.01;
    g3 = schoolY + 0.01;
  }
  else
  {
    g1 = schoolY - 0.01;
    g3 = homeY + 0.01;
  }

  if(homeX > schoolX)
  {
    g2 = homeX + 0.01;
    g4 = schoolX - 0.01;
  }
  else
  {
    g2 = schoolX + 0.01;
    g4 = homeX - 0.01;
  }

  if(proDebug)
  {
    Serial.print("Home X      "); Serial.println(homeX, 4);
    Serial.print("School X    "); Serial.println(schoolX, 4);
    Serial.print("Home Y      "); Serial.println(homeY, 4);
    Serial.print("School Y    "); Serial.println(schoolY, 4);
    
    Serial.print("h1          "); Serial.println(h1, 3);
    Serial.print("h2          "); Serial.println(h2, 3);
    Serial.print("h3          "); Serial.println(h3, 3);
    Serial.print("h4          "); Serial.println(h4, 3);

    Serial.print("s1          "); Serial.println(s1, 3);
    Serial.print("s2          "); Serial.println(s2, 3);
    Serial.print("s3          "); Serial.println(s3, 3);
    Serial.print("s4          "); Serial.println(s4, 3);

    Serial.print("g1          "); Serial.println(g1, 3);
    Serial.print("g2          "); Serial.println(g2, 3);
    Serial.print("g3          "); Serial.println(g3, 3);
    Serial.print("g4          "); Serial.println(g4, 3);
  }
}

bool getGPS(int run)
{
  while(Serial1.available() > 0)
  {
    if(gps.encode(Serial1.read()))
    {
      if(run == 1)
      {
        processData();
      }
      else if(run == 2)
      {
        synchRTC();
      }

      if(gps.location.isValid() && gps.time.isValid() && gps.date.isValid())
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }

  if(millis() > 10000 && gps.charsProcessed() < 10)
  {
    Serial.println("Error - GPS Module Responded with Error");
    Serial.println("  Terminating Code");
    Serial.println("________________________________________");
    while(1) {};
  }
}

bool processData()
{
  if(gps.location.isValid())
  {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    
    latitude = 53.355504;
    longitude = -6.258452;
    
    if(check.isOut())
    {
      if(check.track())
      {
        Serial.println("OUT OF BOUNDS");
        warn.offTrackLoop();
      }
    }
    
    if(check.isHomeTime() && arrivedSchool)
    {
      if(check.isHome())
      {
        Serial.println("AT HOME");
        warn.arrivedHomeLoop();
      }
      else
      {
        Serial.println("LATE -> HOME");
        warn.lateLoop();
      }
    }
    else if(check.isSchoolTime() && arrivedHome)
    {
      Serial.println(".");
      if(check.isSchool())
      {
        Serial.println("AT SCHOOL");
        warn.arrivedSchoolLoop();
      }
      else
      {
        Serial.println("LATE -> SCHOOL");
        warn.lateLoop();
      }
    }
    else
    {
      Serial.println("ON WAY");
    }
  }
}

void synchRTC()
{
  Serial.println("Synching RTC");
  Serial.println("________________________________________");
  
  Serial.println("  OK - Initialising RTC");
  rtc.begin();

  Serial.println("  OK - Synching Time");
  // we have to trim the variable's first 2 digits (2018 -> 18)
  uint16_t yearRAW = gps.date.year();
  String yearString = String(yearRAW);
  yearString.remove(0,1);
  uint16_t year = yearString.toInt();
  
  rtc.setTime(gps.time.hour(), gps.time.minute(), gps.time.second());
  rtc.setDate(gps.date.day(), gps.date.month(), year);

  Serial.println("  Success - RTC Synched");
  Serial.println("________________________________________");
  Serial.println("");
  Serial.println("");
  Serial.println("");
}

void parseData(int state)
{
  SigFox.beginPacket(); // begin the message sending process
  
  if(state == 0) // off track
  {
    SigFox.print("off track"); // send the message
  }
  else if(state == 1) // late
  {
    SigFox.print("late"); // send the message
  }
  else if(state == 2) // arrived home
  {
    SigFox.print("at home"); // send the message
  }
  else // arrived school
  {
    SigFox.print("at school"); // send the message
  }

  SigFox.endPacket();
}

void setup()
{
  if(proDebug)
  {
    Serial.begin(9600);
    while(!Serial) {};

    Serial.println("Setting Up");
  }

  setOutputs();
  Serial.println("Initialising Software Serial");
  Serial1.begin(baudGPS);
  Serial.println("  Success - Software Serial Running");

  Serial.println("Initialising SigFox Module");
  if(!SigFox.begin())
  {
    Serial.println("  Error - SigFox Module Responded with Error");
    reboot();
  }
  Serial.println("  Success - Module Initialised");

  Serial.println("Starting SigFox");
  SigFox.begin();
  Serial.println("  Success - SigFox Module Online");

  Serial.println("Initialising Onboard RTC");
  rtc.begin(); // initialize RTC
  
  Serial.println("  OK - Setting Time and Date");
  Serial.println("  Success - RTC Online");

  Serial.println("Generating Grids");
  generateGrids();
  Serial.println("  Success - Grids are Ready");
  
  Serial.println("Setup Complete");
  Serial.println("  Warning - It may take a few minutes to connect to Satelite");
  Serial.println("");
  Serial.println("");

  if(proDebug)
  {
    Serial.println("Connecting to Satelite");
    Serial.println("________________________________________");
    Serial.println("  OK - Begining Initialisintion");
  }

  while(!getGPS(false)) // wait until data is recived
  {
    Serial.print(".");
    delay(200);
  }
  for(int i = 0; i < 5; i++) // margine of error check
  {
    getGPS(0);
    Serial.print(".");
    delay(500);
  }

  for(int i = 0; i < 15; i++)
  {
    getGPS(2);
    delay(50);
  }
  
  if(proDebug)
  {
    Serial.println("");
    Serial.println("  Success - GPS Initialised");
    Serial.println("  OK - Waiting for the device to connect to Satelite");
    Serial.println("________________________________________");
    Serial.println("");
  }
}

void loop()
{
  if(check.track())
  {
    getGPS(1);
  }
}
