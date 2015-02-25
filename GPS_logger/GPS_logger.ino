/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 
 
   port to LinkIt ONE
 by Loovee
 2014-10-12

 This example code is in the public domain.

 */
#include <LGPS.h>
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>

gpsSentenceInfoStruct info;
char buff[256];
char filename[256];

//#define Drv LFlash          // use Internal 10M Flash
 #define Drv LSD           // use SD card


static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr, LFile &dataFile)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude;
  double longitude;
  int tmp, hour, minute, second, satellites_num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(7, GPGGAstr);
    satellites_num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "%d,%10.4f,%10.4f,%2d:%2d:%2d", satellites_num, latitude, longitude, hour, minute, second);
    dataFile.println(buff); 
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Cannot get data"); 
  }
}

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);



    Serial.println("Initializing SD card...");
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(10, OUTPUT);

    // see if the card is present and can be initialized:
    LTask.begin();
    Drv.begin();
    LGPS.powerOn();
    Serial.println("LGPS Power on, and waiting ..."); 
    determine_filename();
    Serial.println("filename: ");
    Serial.println(filename);
    delay(3000);
    Serial.println("card initialized.");
}
int file_id = 0;
void determine_filename(){
    LFile root;
    root = Drv.open("/");
    while(true) {
        LFile entry =  root.openNextFile();
        if (! entry) {
            // no more files
            break;
        }
        String name = entry.name();
        Serial.print(name);
        if(name.startsWith("GPS") && name.endsWith(".txt")){
          Serial.println("is a GPS file!");
          unsigned int f_id = atoi(name.substring(3, name.length()-4).c_str());
          if(file_id < f_id){
            file_id = f_id;
          }
        }else{
          Serial.println("is not a GPS file.");
        }
        entry.close();
    }
    file_id ++;
    sprintf(filename, "GPS%d.txt", file_id);
}

void loop()
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  LFile dataFile = Drv.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    LGPS.getData(&info);
    parseGPGGA((const char*)info.GPGGA, dataFile);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(3000);
}









