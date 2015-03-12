#include <LGPS.h>
#include <LDateTime.h>
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <LBattery.h>
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

datetimeInfo t;
gpsSentenceInfoStruct info;
char buff[256];
char filename[256];
unsigned int rtc;
unsigned long scheduled_file_write = 0;

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define Drv LSD           // use SD card

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setBacklight(OFF);
  lcd.noDisplay();
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
  scheduled_file_write = millis();
}
int file_id = 0;
void determine_filename() {
  LFile root;
  root = Drv.open("/");
  while (true) {
    LFile entry =  root.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    String name = entry.name();
    //        Serial.print(name);
    if (name.startsWith("GPS") && name.endsWith(".txt")) {
      //          Serial.println("is a GPS file!");
      unsigned int f_id = atoi(name.substring(3, name.length() - 4).c_str());
      if (file_id < f_id) {
        file_id = f_id;
      }
    } else {
      //          Serial.println("is not a GPS file.");
    }
    entry.close();
  }
  file_id ++;
  sprintf(filename, "GPS%d.txt", file_id);
}
int state = HIGH;
unsigned long int curr_time;
long long int light_off_time = -1;
void loop()
{
  curr_time = millis();
  LDateTime.getTime(&t);
  lcd.setCursor(0, 0);
  lcd.printf("%02d%02d%02d %02d:%02d:%02d", t.year % 100, t.mon, t.day, t.hour, t.min, t.sec);
  lcd.setCursor(0, 1);
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.setBacklight(WHITE);
    lcd.display();
    light_off_time = curr_time+5000;
  }
  if (LBattery.isCharging()) {
    lcd.printf("battery:%d%%+", LBattery.level());
  } else {
    lcd.printf("battery:%d%% ", LBattery.level());
  }
  if (light_off_time > 0 && light_off_time < curr_time){
    lcd.setBacklight(OFF);
    lcd.noDisplay();
    light_off_time = -1;
  }
  if (curr_time > scheduled_file_write) {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    LFile dataFile = Drv.open(filename, FILE_WRITE);

    lcd.setCursor(15,1);
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
      lcd.print('O');
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
      lcd.print('X');
    }
    scheduled_file_write += 3000;
  }
}









