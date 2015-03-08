#include <LGPS.h>
#include <LDateTime.h>
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>

datetimeInfo t;
gpsSentenceInfoStruct info;
char buff[256];
char filename[256];
unsigned int rtc;

#define Drv LSD           // use SD card

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);

    pinMode(1, OUTPUT);
    digitalWrite(1, LOW);    // turn the LED off by making the voltage LOW
    
//    while(!Serial.available());         // input any thing to start


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
    Serial.print("Saving to ");
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
//        Serial.print(name);
        if(name.startsWith("GPS") && name.endsWith(".txt")){
//          Serial.println("is a GPS file!");
          unsigned int f_id = atoi(name.substring(3, name.length()-4).c_str());
          if(file_id < f_id){
            file_id = f_id;
          }
        }else{
//          Serial.println("is not a GPS file.");
        }
        entry.close();
    }
    file_id ++;
    sprintf(filename, "GPS%d.txt", file_id);
}

int state = HIGH;

void loop()
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  LFile dataFile = Drv.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    digitalWrite(1, state);   // turn the LED on (HIGH is the voltage level)
    state = !state;
    LDateTime.getRtc(&rtc);
    LGPS.getData(&info);
    dataFile.print(rtc);
    dataFile.print(",");
    dataFile.print((char*)info.GPGGA);
    dataFile.close();
    Serial.print(rtc);
    Serial.print(",");
    Serial.print((char*)info.GPGGA);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(3000);
}









