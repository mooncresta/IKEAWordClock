/*
 *  Adapted from WordClock2 project on Instructables
 *  Thank you for to WhiteClockCompany WordClock2

 * Credit to:
 * Adafruit for the wonderful Neopixel library and 
   invaluable guide to using rgb addressable leds 
 
 **Warning!!
   You modify and upload this code at your own risk
   the previous authors and mysekf accept no liability for any
   damage to your clock as a result of you modifying
   the original code 

   SW - Lots of changes to improve refresh add extra animations and timing driven
   Modified Layout and added NTP code, special days, table driven, port to ESP32
*/

/*
 * Display for Minutes 
00  IT IS <<HOUR>> O'CLOCK
01 & 02 IT HAS JUST GONE <<HOUR>> O'CLOCK
03 & 04 IT IS NEARLY FIVE MINUTES PAST <<HOUR>>
05  IT IS FIVE MINUTES PAST <<HOUR>>
06 & 07 IT HAS JUST GONE FIVE MINUTES PAST <<HOUR>> 
08 & 09 IT IS NEARLY TEN MINUTES PAST <<HOUR>>
10  IT IS TEN MINUTES PAST <<HOUR>>
11 & 12 IT HAS JUST GONE TEN MINUTES PAST <<HOUR>> 
13 & 14 IT IS NEARLY Q PAST <<HOUR>>
15  IT IS QUARTER PAST <<HOUR>>
16 & 17 IT HAS JUST GONE QUARTER PAST <<HOUR>> 
18 & 19 IT IS NEARLY TWENTY MINUTES PAST <<HOUR>>
20  IT IS TWENTY MINUTES PAST <<HOUR>>
21 & 22 IT HAS JUST GONE TWENTY MINUTES PAST <<HOUR>> 
23 & 24 IT IS NEARLY TWENTY FIVE MINUTES PAST <<HOUR>>
25  IT IS TWENTY FIVE MINUTES PAST <<HOUR>>
26 & 27 IT HAS JUST GONE TWENTY FIVE MINUTES PAST <<HOUR>> 
28 & 29 IT IS NEARLY HALF PAST <<HOUR>>
30  IT IS HALF PAST <<HOUR>>
31 & 32 IT HAS JUST GONE HALF PAST <<HOUR>> 
<<HOUR>> = <<HOUR>> + 1
33 & 34 IT IS NEARLY TWENTY FIVE MINUTES TO <<HOUR>>
35  IT IS TWENTY FIVE MINUTES TO <<HOUR>>
36 & 37 IT HAS JUST GONE TWENTY FIVE MINUTES TO <<HOUR>> 
38 & 39 IT IS NEARLY TWENTY MINUTES TO <<HOUR>>
40  IT IS TWENTY MINUTES TO <<HOUR>>
41 & 42 IT HAS JUST GONE TWENTY MINUTES TO <<HOUR>> 
43 & 44 IT IS NEARLY QUARTER TO <<HOUR>>
45  IT IS QUARTER TO <<HOUR>>
46 & 47 IT HAS JUST GONE QUARTER TO <<HOUR>> 
48 & 49 IT IS NEARLY TEN MINUTES TO <<HOUR>>
50  IT IS TEN MINUTES TO <<HOUR>>
51 & 52 IT HAS JUST GONE TEN MINUTES TO <<HOUR>> 
53 & 54 IT IS NEARLY FIVE MINUTES TO <<HOUR>>
55  IT IS FIVE MINUTES TO <<HOUR>>
56 & 57 IT HAS JUST GONE FIVE MINUTES TO <<HOUR>> 
58 & 59 IT IS NEARLY <<HOUR>> O'CLOCK
*/

/*
SPECIAL TIMES
12:00 MIDDAY
00:00 MIDNIGHT
*/


 
#include <WiFi.h>
#include <time.h>
#include "timezone.h"

#include "arduino_secrets.h"

time_t now;
#define LOOPCOUNT 1000
int timeloop = 0;
bool ntpinit = true;
bool isSpecial = false;

const char ssid[]       = YOUR_SSID;
const char password[]   = YOUR_PASS;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#include "Wire.h" 
#define DS3231_I2C_ADDRESS 0x68
#include <Adafruit_NeoPixel.h>
// D3 is Pin 3 etc 
//ESP32 GIO15 is 15
#define PIN            15
#define NUMPIXELS     144
// LED Brightness
#define MAXBRIGHT 50
#define NIGHTBRIGHT 5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Save array
uint32_t pixelState[NUMPIXELS];
struct tm timeinfo;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

int second, minute, hour, dayOfWeek, dayOfMonth, month, year;

#define NUM_WORDS 36 // count of words below
// *****  NEW WORDS - 4 ******************************
int WordIt[] = {132, 133, -1};
int WordIs[] = {139, 140, -1};
int WordNearly[] = {130, 129, 128, 127, 126, 125, -1};
int WordHasjustgone[] = {141, 142, 143, 124, 123, 122, 121, 108, 109, 110, 111, -1};
// BASE WORDS - 21 ****************************************************
int WordTwenty[] = {113, 114, 115, 116, 117, 118, -1};
int WordMinTen[] = {84, 85, 86, -1};
int WordMinFive[] = {100, 99, 98, 97, -1};
int WordQuarter[] = {107, 106, 105, 104, 103, 102, 101, -1};
int WordMinutes[] = {89, 90, 91, 92, 93, 94, 95, -1};
int WordHalf[] = {83, 82, 81, 80, -1};
int WordTo[] = {79, 78, -1};
int WordPast[] = {77, 76, 75, 74, -1};
int WordFive[] = {36, 37, 38, 39, -1};
int WordOne[] = {59, 58, 57, -1};
int WordTwo[] = {68, 69, 70, -1};
int WordThree[] = {32, 31, 30, 29, 28, -1};
int WordFour[] = {27, 26, 25, 24, -1};
int WordSix[] = {65, 66, 67, -1};
int WordSeven[] = {60, 61, 62, 63, 64, -1};
int WordEight[] = {57, 56, 55, 54, 53, -1};
int WordNine[] = {44, 45, 46, 47, -1};
int WordTen[] = {68, 51, 44, -1};
int WordEleven[] = {39, 40, 41, 42, 43, 44, -1};
int WordTwelve[] = {53, 52, 51, 50, 49, 48, -1};
int WordOclock[] = {12, 13, 14, 15, 16, 17, 18, -1};
/****************************************************
* SPECIAL NEW WORDS - 11
**************************************************/
int WordMidday[] = {11, 10, 9, 8, 7, 6, -1};
int WordMidnight[] = {20, 21, 22, 4, 3, 2, 1, 0, -1};
int WordHappybirthday[] = {134, 135, 136, 137, 138, 35, 34, 33, 32, 31, 8, 7, 6, -1};
int WordAdam[] = {105, 87, 88, 89, -1};
int WordAlex[] = {82, 81, 63, 67, -1};
int WordHarry[] = {131, 105, 104, 101, 118, -1};
int WordGeorge[] = {108, 111, 112, 101, 73, 72, -1};
int WordSteve[] = {65, 53, 51, 49, 48, -1};
int WordSally[] = {60, 82, 81, 50, 71, -1};
int WordXmasEve[] = {120, 119, 96, 95, 41, 42, 43, -1};
int WordXmasday[] = {120, 119, 96, 95, 8, 7, 6, -1};

/* Delta stuff ToDo in case of flicker
typedef struct {
  int state;
  int  
} lightstate;

struct lightstate lightson = {
  {0, 
};
*/
int flag = 0; //used for display effects to stop it showing more than once
//define colours
uint32_t Black = pixels.Color(0,0,0);
uint32_t White = pixels.Color(255,255,255);
uint32_t Green = pixels.Color(0,255,0);
uint32_t Red = pixels.Color(255,0,0);
uint32_t Gold = pixels.Color(255,204,0);
uint32_t Grey = pixels.Color(30,30,30);
uint32_t Blue = pixels.Color(0,0,255);
//About colours
uint32_t whiteblue = pixels.Color(255,255,255);
uint32_t lightblue = pixels.Color(153,204,255);
uint32_t midblue = pixels.Color(0,102,204);
uint32_t darkblue = pixels.Color(0,0,255);
//coffee
uint32_t Brown = pixels.Color(153,102,051);
//ME!
uint32_t Pink = pixels.Color(255,153,153);
#define DELAY_TIME 5000 /// 5s
bool delayRunning = false; // true if still waiting for delay to finish
unsigned long delayStart = 0; // the time the delay started
int curr_min = 0;
int curr_hour = 0;
bool firstDisp = true;
// Words
struct phrase {
  int minute1;
  int minute2;
  char *wordphrase[];
};
struct minphrase {
  int minute1;
  int minute2;
  char words[100];
};
#define T_IT 1
#define T_IS 2
#define T_HAS 3
#define T_NEARLY 4
#define T_HALF 5
#define T_PAST 6
#define T_TO 7
#define T_QUARTER 8
#define T_MINUTES 9
#define T_OCLOCK 10
#define T_TWENTY 11
#define T_MINTEN 12
#define T_MINFIVE 13
#define T_HOUR 14

struct minphrase arr_words[37] = {
  {0,-1, "IT IS <<HOUR>> O'CLOCK"},
  {1,2, "IT HAS JUST GONE <<HOUR>> O'CLOCK"},
  {3,4, "IT IS NEARLY FIVE MINUTES PAST <<HOUR>>"},
  {5,-1, "IT IS FIVE MINUTES PAST <<HOUR>>"},
  {6,7, "IT HAS JUST GONE FIVE MINUTES PAST <<HOUR>>"}, 
  {8,9, "IT IS NEARLY TEN MINUTES PAST <<HOUR>>"},
  {10,-1, "IT IS TEN MINUTES PAST <<HOUR>>"},
  {11,12, "IT HAS JUST GONE TEN MINUTES PAST <<HOUR>>"},
  {13,14, "IT IS NEARLY QUARTER PAST <<HOUR>>"},
  {15,-1, "IT IS QUARTER PAST <<HOUR>>"},
  {16,17, "IT HAS JUST GONE QUARTER PAST <<HOUR>>"},
  {18,19, "IT IS NEARLY TWENTY MINUTES PAST <<HOUR>>"},
  {20,-1, "IT IS TWENTY MINUTES PAST <<HOUR>>"},
  {21,22, "IT HAS JUST GONE TWENTY MINUTES PAST <<HOUR>>"}, 
  {23,24, "IT IS NEARLY TWENTY FIVE MINUTES PAST <<HOUR>>"},
  {25,-1, "IT IS TWENTY FIVE MINUTES PAST <<HOUR>>"},
  {26,27, "IT HAS JUST GONE TWENTY FIVE MINUTES PAST <<HOUR>>"},
  {28,29, "IT IS NEARLY HALF PAST <<HOUR>>"},
  {30,-1, "IT IS HALF PAST <<HOUR>>"},
  {31,32, "IT HAS JUST GONE HALF PAST <<HOUR>>"},
  {33,34, "IT IS NEARLY TWENTY FIVE MINUTES TO <<HOUR>>"},
  {35,-1, "IT IS TWENTY FIVE MINUTES TO <<HOUR>>"},
  {36,37, "IT HAS JUST GONE TWENTY FIVE MINUTES TO <<HOUR>>"}, 
  {38,39, "IT IS NEARLY TWENTY MINUTES TO <<HOUR>>"},
  {40,-1, "IT IS TWENTY MINUTES TO <<HOUR>>"},
  {41,42, "IT HAS JUST GONE TWENTY MINUTES TO <<HOUR>>"}, 
  {43,44, "IT IS NEARLY QUARTER TO <<HOUR>>"},
  {45,-1, "IT IS QUARTER TO <<HOUR>>"},
  {46,47, "IT HAS JUST GONE QUARTER TO <<HOUR>>"}, 
  {48,49, "IT IS NEARLY TEN MINUTES TO <<HOUR>>"},
  {50,-1, "IT IS TEN MINUTES TO <<HOUR>>"},
  {51,52, "IT HAS JUST GONE TEN MINUTES TO <<HOUR>>"}, 
  {53,54, "IT IS NEARLY FIVE MINUTES TO <<HOUR>>"},
  {55,-1, "IT IS FIVE MINUTES TO <<HOUR>>"},
  {56,57, "IT HAS JUST GONE FIVE MINUTES TO <<HOUR>>"}, 
  {58,59, "IT IS NEARLY <<HOUR>> O'CLOCK"},
  {-1, -1, "X"}
};
/*
Days  - TODO still needs fixing the overlap !
03-Mar  HAPPY BIRTHDAY ADAM
01-Oct  HAPPY BIRTHDAY ALEX
29-Sep  HAPPY BIRTHDAY HARRY
04-Mar  HAPPY BIRTHDAY GEORGE
30-Jun  HAPPY BIRTHDAY STEVE
03-Nov  HAPPY BIRTHDAY SALLY
24-Dec  XMAS EVE
25-Dec  XMAS DAY
*/
struct specialList {
  int specday;
  int specmonth;
  char specialType[10];
};

#define T_ADAM "ADAM"
#define T_ALEX "ALEX"
#define T_HARRY "HARRY"
#define T_GEORGE "GEORGE"
#define T_STEVE "STEVE"
#define T_SALLY "SALLY"
#define T_XMAS_EVE "XMAS EVE"
#define T_XMAS_DAY "XMAS DAY"

static specialList specialDays[] = {
    {3,3, T_ADAM},
    {1,10, T_ALEX},
    {29,9, T_HARRY},
    {4, 3, T_GEORGE},
    {30,6, T_STEVE},
    {4,11, T_SALLY},
    {24,12, T_XMAS_EVE},
    {25,12, T_XMAS_DAY}, 
    {-1,-1, "XXXXX"}
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");
  Wire.begin();
  pixels.begin();
  doNTP();
  blank();
  pixels.setBrightness(MAXBRIGHT);
  test(); //run basic screen tests
  displayTime(); // display the clock data on the Serial Monitor  and then LEDS,
  curr_min = minute;
  curr_hour = hour;
  // Set to now
  doMinute(hour, minute);  

  delayStart = millis();   // start delay
  delayRunning = true; // not finished yet
} //end setup

void displayWord(int wordType)
{
   switch(wordType) {
      case T_IT:
           Serial.println("-IT-");
           lightup(WordIt, White);
        break;
      case T_IS:
           Serial.println("-IS-");
           lightup(WordIs, White);
           break;
       case T_NEARLY:
            Serial.println("-NEARLY-");
            lightup(WordNearly, White);
        break;
        case T_HALF:
            Serial.println("-HALF-");
            lightup(WordHalf, White);
        break;
        case T_PAST:
            Serial.println("-PAST-");
            lightup(WordPast, White);
        break;        
        case T_HAS:
            Serial.println("-HAS JUST GONE-");
            lightup(WordHasjustgone, White);
        break;      
        case T_TO:
            Serial.println("-TO-");
            lightup(WordTo, White);
        break;        
        case T_QUARTER:
            Serial.println("-QUARTER-");
            lightup(WordQuarter, White);
        break;   
        case T_TWENTY:
            Serial.println("-TWENTY-");
            lightup(WordTwenty, White);          
        break;    
        case T_MINTEN:
            Serial.println("-TEN-");
            lightup(WordMinTen, White);
        break;             
        case T_MINFIVE:
            Serial.println("-FIVE-");
            lightup(WordMinFive, White);
        break;       
        case T_MINUTES:
            Serial.println("-MINUTES-");
            lightup(WordMinutes, White);
        break;               
        case T_OCLOCK:
            Serial.println("-OCLOCK-");
            lightup(WordOclock, White);
        break;           
   } //end switch

} // end dispWord

void parseLine(char *wLine, const char *srch, int tType)
{
     char *pch = NULL;
     pch = strstr(wLine, srch);
     if(pch != NULL) {
         if(strcmp("<<HOUR>>", srch) != 0) {
             displayWord(tType);
         } else {
             // Set Hour
             doHour(hour, minute);     
         }
     }    
}

void doSelWipe() 
{
  // Force selective wipe - maybe a delta in future
  lightup(WordIt, Black);
  lightup(WordIs, Black);
  lightup(WordHasjustgone, Black);
  lightup(WordNearly, Black);
  lightup(WordHalf, Black);
  lightup(WordPast, Black);
  lightup(WordTo, Black);
  lightup(WordQuarter, Black);
  lightup(WordTwenty, Black);
  lightup(WordMinTen, Black);
  lightup(WordMinFive, Black);
  lightup(WordMinutes, Black);
  lightup(WordOclock, Black);
};

void doMinute(int hour, int minute)
{ 
   int disphour;
   char dispWord[100];

  // Do Special times and then return
  if(hour == 12 && minute == 0) {
    Serial.println("IT IS Midday");
    doSelWipe(); 
    lightup(WordIt, White);
    lightup(WordIs, White);
    lightup(WordMidday, White);
    return;
  } else if (hour == 0&& minute == 0) {
    Serial.println("IT IS Midnight");
    doSelWipe(); 
    lightup(WordIt, White);
    lightup(WordIs, White);
    lightup(WordMidnight, White);
    return;
  } else {
     lightup(WordMidnight, Black);
     lightup(WordMidday, Black);
  }
  
   for(int i=0; arr_words[i].minute1 >= 0;i++) {
 //      Serial.print(arr_words[i].minute1);     
       if(arr_words[i].minute1 == minute | arr_words[i].minute2 == minute) {
           if(arr_words[i].minute2 != minute | firstDisp == true) {
               firstDisp = false;
               // Force clear on minute words
               doSelWipe();
               //Found the phrase parse it and send to LEDS don't change if it's subsequent minute
               Serial.println(arr_words[i].words);
               parseLine(arr_words[i].words, "IT", T_IT);
               parseLine(arr_words[i].words, "IS", T_IS);
               parseLine(arr_words[i].words, "HAS", T_HAS);
               parseLine(arr_words[i].words, "NEARLY", T_NEARLY);
               parseLine(arr_words[i].words, "HALF", T_HALF);
               parseLine(arr_words[i].words, "QUARTER", T_QUARTER);
               parseLine(arr_words[i].words, "PAST", T_PAST);
               parseLine(arr_words[i].words, "TO", T_TO);
               parseLine(arr_words[i].words, "O'CLOCK", T_OCLOCK);
               parseLine(arr_words[i].words, "MINUTES", T_MINUTES);
               //    "Setting any minutes"
               parseLine(arr_words[i].words, "TWENTY", T_TWENTY);
               parseLine(arr_words[i].words, "TEN", T_MINTEN);    
               parseLine(arr_words[i].words, "FIVE", T_MINFIVE);  
               // Parse Hours
               parseLine(arr_words[i].words, "<<HOUR>>", T_HOUR);  
           }
           else {
              Serial.println("Second Min with same string");
           } //endif minute2
      } //endif
   } //endfor
}  // End doMinute

void blankHour()
{
      lightup(WordOne, Black);
      lightup(WordTwo, Black);
      lightup(WordThree, Black);
      lightup(WordFour, Black);
      lightup(WordFive, Black);
      lightup(WordSix, Black);
      lightup(WordSeven, Black);
      lightup(WordEight, Black);
      lightup(WordNine, Black);
      lightup(WordTen, Black);
      lightup(WordEleven, Black);
      lightup(WordTwelve, Black);
};

void doHour(int hour, int minute)
{
   int disphour;
   Serial.println("Setting Hour");
   if(minute >= 33) {
          disphour = hour+1;
          if(minute == 33) { // Hour flipped over
               blankHour();
          };
   } else {
          disphour = hour;
   }
   // Clear hour lights - shopuld be done in main loop
   //  blankHour();
   
  switch (disphour) {
    case 1:
    case 13:
      lightup(WordOne, Red);
      break;
    case 2:
    case 14:
      lightup(WordTwo, Red);
      break;
    case 3:
    case 15:
      lightup(WordThree, Red);
      break;
    case 4:
    case 16:
      lightup(WordFour, Red);
      break;
    case 5:
    case 17:
      lightup(WordFive, Red);
      break;
    case 6:
    case 18:
      lightup(WordSix, Red);
      break;
    case 7:
    case 19:
      lightup(WordSeven, Red);
      break;
    case 8:
    case 20:
      lightup(WordEight, Red);
      break;
    case 9:
    case 21:
      lightup(WordNine, Red);
      break;
    case 10:
    case 22:
      lightup(WordTen, Red);
      break;
    case 11:
    case 23:
      lightup(WordEleven, Red);
      break;
    case 00:
    case 24: // for midnight wraparound to xxx hour
    case 12:
      lightup(WordTwelve, Red);
      break;
      }// end of case statement
}
bool doSpecial()
{
   // Assumed to be call once a day (or on init)
   Serial.println("Special Days"); 
 /* blank any Specials
   lightup(WordHappybirthday, Black);
   lightup(WordXmasEve, Black);
   lightup(WordXmasday, Black);
   lightup(WordAdam, Black);
   lightup(WordAlex, Black);
   lightup(WordHarry, Black);
   lightup(WordGeorge, Black);
   lightup(WordSteve, Black);
   lightup(WordSally, Black);
*/
//   Serial.printf("Day %d Month %d\n", dayOfMonth, month); 

   for(int i=0; specialDays[i].specday > 0;i++) 
   {
//      Serial.printf("Special Day %d Month %d\n", specialDays[i].specday, specialDays[i].specmonth ); 
      if(specialDays[i].specday == dayOfMonth && specialDays[i].specmonth == month)
      {
          Serial.println("Found Special Days"); 
          // save current state of LEDS
          pushState();
          blank();
          if(strcmp(specialDays[i].specialType, T_ADAM) == 0) {
             lightup(WordHappybirthday, Gold);
             lightup(WordAdam, Gold);
             return true;
          }  
          if(strcmp(specialDays[i].specialType, T_ALEX) == 0) {
             lightup(WordHappybirthday, Blue);
             lightup(WordAlex, Blue);
             return true;
          }
          if(strcmp(specialDays[i].specialType, T_GEORGE) == 0) {
             lightup(WordHappybirthday, Green);
             lightup(WordGeorge, Green);
             return true;
          } 
          if(strcmp(specialDays[i].specialType, T_SALLY) == 0) {
             lightup(WordHappybirthday, Pink);
             lightup(WordSally, Pink);
              return true;
          } 
           if(strcmp(specialDays[i].specialType, T_STEVE) == 0) {
             lightup(WordHappybirthday, darkblue);
             lightup(WordSteve, darkblue);
             return true;
          } 
          if(strcmp(specialDays[i].specialType, T_HARRY) == 0) {
             lightup(WordHappybirthday, Red);
             lightup(WordHarry, Red);
             return true;
          }
          if(strcmp(specialDays[i].specialType, T_XMAS_EVE) == 0) {
             lightup(WordXmasEve, whiteblue);
             return true;
          }   
          if(strcmp(specialDays[i].specialType, T_XMAS_DAY) == 0) {
             lightup(WordXmasday, whiteblue);
             return true;
          }                                                              
      } else {
         // Do nothing

      }
   } // end for
   
   return false;
} // end doSpecial



void loop()
{
  // Main Loop
  // check if delay has timed out after 10sec == 10000mS
  if (delayRunning && ((millis() - delayStart) >= DELAY_TIME)) {
     Serial.println("Delay Timer Expired");
     delayStart += DELAY_TIME; // this prevents drift in the delays
     displayTime(); // display the clock data on the Serial Monitor  and then LEDS,

     if(hour != curr_hour) {
        Serial.println("Hour has changed");
        // clear previous
        blankHour();
        if(hour == 0) {
          Serial.println("Midnight so reset NTP");
          doNTP();
         }
     } 
     curr_hour = hour;

    if(minute != curr_min) {
      Serial.println("Minute has changed");
      TimeOfDay(); //set brightness dependant on time of day
      doMinute(hour, minute);  
    } else {
      // Min not changed
//      Serial.printf("Seconds are %d\n", second);
      if(isSpecial == false) {
          if(second >= 30 && second <=39 ) {
          // check for Special days
// Test code for specialDays
// dayOfMonth = 3;
// month =3;
// End test         
          isSpecial = doSpecial();
          }
      }
//      Serial.println(isSpecial);
      if(isSpecial == true) {
        if(second >= 40 && second <=49) {
           Serial.println("Restoring normal time");
           popState();
           pixels.show();
           isSpecial = false;
        }
      }
    } 
    curr_min = minute;
    
  }
//  Other loop code here if required . . .
//  Serial.println("Run Other Code");  

  
   
} // End of main loop

/*********************************************************************/

void TimeOfDay() {
//Used to set brightness dependant of time of day - lights dimmed at night
//monday to thursday and sunday

if ((dayOfWeek == 6) | (dayOfWeek == 7)) {
  if ((hour > 0) && (hour < 8)) {
    pixels.setBrightness(NIGHTBRIGHT);
  }
  else {
    pixels.setBrightness(MAXBRIGHT); 
  }
}
else {
  if ((hour < 6) | (hour >=22)) {
    pixels.setBrightness(NIGHTBRIGHT);
  }
  else {
    pixels.setBrightness(MAXBRIGHT);
  }
}
} // End TimeOfDay

void doNTP() {

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
//init and get the time from NTP Server standard routine doesn't work in UK correctly
//  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
// The above seems broken in the UK - set Summertime too early in March
// Try TZ method - https://remotemonitoringsystems.ca/time-zone-abbreviations.php

  configTime(0, 0, ntpServer);
  setenv("TZ", "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00",1);
  tzset();

  printLocalTime();

  // Loop until sensible time > 2019 is set
  while (now < EPOCH_1_1_2019)
  {
    now = time(nullptr);
    delay(500);
    Serial.print("*");
  }
  
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void displayTime()
{
//  Serial.println("Displaying time");
/*
  time(&now);
  timeinfo = localtime(&now);
*/
  if(!getLocalTime(&timeinfo)){
    Serial.println("Display time Failed to obtain time");
    return;
  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon+1; /// Seems to start at 0
  dayOfMonth = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
  dayOfWeek = timeinfo.tm_wday;

//  Serial.print(hour);
//  Serial.println(minute);
//  delay(500);
} // End DisplayTime

void popState() {
  // Restore the state of the array
  for (int x = 0; x < NUMPIXELS; ++x) {
      pixels.setPixelColor(x, pixelState[x]);
  }
  pixels.show();
}
void pushState() {
  // Save the state of the array
  for (int x = 0; x < NUMPIXELS; ++x) {
      pixelState[x]= pixels.getPixelColor(x);
//      Serial.print(pixelState[x]);
  }
}

void lightup(int Word[], uint32_t Colour) {
for (int x = 0; x < pixels.numPixels() + 1; x++) {
  if(Word[x] == -1) {
    pixels.show();
    break;
  } //end of if loop
  else {
    pixels.setPixelColor(Word[x], Colour);
//    pixels.show(); // Don't think this is needed as set at the end of the word array and reduces flicker
  } // end of else loop
} //end of for loop
}

void blank() {
//clear the decks
   for (int x = 0; x < NUMPIXELS; ++x) {
     pixels.setPixelColor(x, Black);
   }
   pixels.show();
}

void wipe() {
  Serial.println("Wipe");

  for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Blue);
    delay(5);
    pixels.show();
    }
//  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(5);
    pixels.show();
  }
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Green);
    delay(5);
    pixels.show();
    }
//  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(5);
    pixels.show();
  }
   for (int x = 0; x < NUMPIXELS; ++x) {
    pixels.setPixelColor(x, Red);
    delay(5);
    pixels.show();
   }
//  delay(50);  
  for (int x = NUMPIXELS; x > -1; --x) {
    pixels.setPixelColor(x, Black);
    delay(5);
    pixels.show();
  }

  for (int x = 0; x < NUMPIXELS; x=x+2) {
    pixels.setPixelColor(x, Red);
    delay(10);
    pixels.show();
   }
 //  delay(500);
  blank();

}

void test() {
  Serial.println("Test Routine");
  blank();
  wipe();
  blank();
  flash();
}

void flash() {
  Serial.println("Flash");
  blank();

for (int y = 0; y< 10; ++y) {
    for (int x = 0; x < NUMPIXELS; x=x+2) {
      pixels.setPixelColor(x, Pink);
    }
    pixels.setBrightness(100);
    pixels.show();
//    delay(20);
    blank();
//    delay(20);

    for (int x = 1; x < NUMPIXELS; x=x+2) {
      pixels.setPixelColor(x, Pink);  
    }
    pixels.setBrightness(100);
    pixels.show();
    delay(10);
    blank();
    delay(10);
}
blank();
}
