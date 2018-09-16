#include <Time.h>
#include <TimeLib.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DS1302RTC.h>
#include <stdlib.h>
#include <ESP8266wifi.h>
//relay pins
#define role1 47    //Spare tank Water Pump
#define role2 45    //Heater
#define role3 43    //Cooler fan
#define role4 41
#define role5 46
#define role6 44
#define role7 42
#define role8 40
//button pins
#define btnmenu 2     //button pin for entermenu
#define btnup 3     //button pin for up
#define btndown 4     //button pin for down
#define btnleft 5     //button pin for left
#define btnright 6    //button pin for right
#define led_relay 22
#define flo_relay 23
#define DEBUGMODE 1
//configure wifi
#define esp8266_reset_pin 7 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)
#define ESPSERVER_PORT "80"
#define SERVER_PORT "1880"
#define SERVER_IP "192.168.1.100"
#define SSID "Guler"
#define PASSWORD "melisglr2403$1"
//#define wifi Serial2

unsigned long previousupdate = 0;
unsigned long currentmillis = 0;

//LCD definitions
LiquidCrystal_I2C lcd(0x27, 16, 2); //define 16x2 LCD
//RTC definitions
uint8_t CE_PIN = 32;    //rtc enable/rst pin
uint8_t IO_PIN = 34;    //rtc IO pin
uint8_t SCLK_PIN = 36;  //rtc SCLK pin
DS1302RTC rtc (CE_PIN, IO_PIN, SCLK_PIN);
//sensor definitions
OneWire ds(11);         //temperature sensor definition
DallasTemperature sensors(&ds); //temperature sensor definition
#define trigPin 12        //Ultrasonic sensor pins
#define echoPin 13        //Ultrasonic sensor pins
#define sump 9          //Sump water sensor pin
#define buzzer 10         //buzzer pin
#define sens_power 24     //sensor power pin
ESP8266wifi wifi(Serial2, Serial2, esp8266_reset_pin, Serial);
uint8_t wifi_started = false;

//variables
String tus = " ";     //pressed key variable
int max_tone  = 0;    //alarm on maximum level
int min_tone  = 1;    //alarm on minimum level
int menu_item = 1;
int lcd_led = 1;      //lcd backlight status
int menu_1_stat = 2;    //menu1 (Lamps)default value 2:auto
int menu_3_stat = 2;    //menu3 (spare water)default value 2:auto
int menu_4_stat = 2;    //menu4 (heater) default value 2:auto
int menu_5_stat = 2;    //menu5 (fan) default value 2:auto
String led_stat = "";
String flo_stat = "";
String role4_stat = "off";
String role5_stat = "off";
String role6_stat = "off";
String role7_stat = "off";
String role8_stat = "off";
int led_on = 6; //led on hour
int led_off = 22; //led off hour
int flo_on = 6; //florescent on hour
int flo_off = 19; //florescent off hour
int say = 1;
boolean nv = true;
float temperature;
int level;
time_t zaman;
unsigned long eskiZaman = 0;
String response = "";
String postdata = "";
// TCP Commands
const char RST[] PROGMEM = "RST";
const char IDN[] PROGMEM = "*IDN?";

void setup()
{
//#if defined(DEBUGMODE)
  Serial.begin(9600);
  debuglog("akvaryumkontrol_140417");
//#endif
  lcd.init();
  lcd.begin(16, 2);     // start lcd library
  lcd.backlight();      //turn lcd backlight on
  lcd.setCursor(0, 0);
  lcd.print("Version 17.04.14");
  delay (1000);
  lcd.print("8-Setup started ");
  debuglog("lcd ok");
  pinMode(trigPin, OUTPUT);   //ultrasonic distance measurement trigger pin
  pinMode(echoPin, INPUT);    //ultrasonic distance measurement echo pin
  pinMode(role1, OUTPUT);
  pinMode(role2, OUTPUT);
  pinMode(role3, OUTPUT);
  pinMode(role4, OUTPUT);
  pinMode(role5, OUTPUT);
  pinMode(role6, OUTPUT);
  pinMode(role7, OUTPUT);
  pinMode(role8, OUTPUT);
  pinMode(led_relay, OUTPUT);
  pinMode(flo_relay, OUTPUT);
  pinMode(sens_power, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("7-LCD         OK");
  digitalWrite(2, HIGH);    //set menu pin high
  digitalWrite(3, HIGH);    //set up pin high
  digitalWrite(4, HIGH);    //set down pin high
  digitalWrite(5, HIGH);    //set left pin high
  digitalWrite(6, HIGH);    //set right pin high
  digitalWrite(sump, HIGH); //set sump sensor pin high
  digitalWrite(sens_power, HIGH); //set sensor power on
  lcd.setCursor(0, 0);
  lcd.print("6-Pinconfig   OK");
  debuglog("pinconfig ok");
  //setTime(22, 12, 00, 02, 03, 2016);
  //rtc.set(now());
  setSyncProvider(rtc.get);
  zaman = rtc.get();
  lcd.setCursor(0, 0);
  lcd.print("5-RTC         OK");
  debuglog("rtcget ok");
  setTime(zaman);
  sensors.begin(); //start temp sensors
  lcd.setCursor(0, 0);
  lcd.print("4-Sensors     OK");
  // start HW serial for ESP8266 (change baud depending on firmware)
  Serial2.begin(9600);
  while (!Serial);
  Serial.println("Starting wifi");
  wifi.setTransportToTCP();// this is also default
  wifi.endSendWithNewline(true); // Will end all transmissions with a newline and carrage return ie println.. default is true
  wifi_started = wifi.begin();
  if (wifi_started)
  {
    lcd.setCursor(0, 0);
    lcd.print("3-Wifi        OK");
    wifi.connectToAP(SSID, PASSWORD);
    wifi.connectToServer(SERVER_IP, SERVER_PORT);
    wifi.startLocalServer(ESPSERVER_PORT);
    lcd.setCursor(0, 0);
    lcd.print("2-Server      OK");
  }
  else
  {
    // ESP8266 isn't working..
  }
  lcd.setCursor(0, 0);
  lcd.print("1-Setup       OK");
  debuglog("setup tamamlandı");
}
void tusbekle()
{
  tus = " ";
  while (tus == " ")
  {
    if(digitalRead(btnup) == LOW)
    {
      tus = "up";
      eskiZaman = millis();
      break;
    };
    if(digitalRead(btndown) == LOW)
    {
      tus = "down";
      eskiZaman = millis();
      break;
    };
    if(digitalRead(btnright) == LOW)
    {
      tus = "right";
      eskiZaman = millis();
      break;
    };
    if(digitalRead(btnleft) == LOW)
    {
      tus = "left";
      eskiZaman = millis();
      break;
    };
    if(digitalRead(btnmenu) == LOW)
    {
      tus = "menu";
      eskiZaman = millis();
      break;
    };
    if (millis() - eskiZaman > 10000)
    {
      tus = "left";
      eskiZaman = millis();
      break;
    };
  }

}
void entermenu ()
{
  menu_item = 1;
  eskiZaman = millis();
llp:
  lcd.clear();
  switch (menu_item)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("   Aydinlatma   ");
      lcd.setCursor(0, 1);
      lcd.print("  Ac/Kapat/Oto  ");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Aydinlatma Zaman");
      lcd.setCursor(0, 1);
      lcd.print("     Ayari      ");
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Yedek Su Pompasi");
      lcd.setCursor(0, 1);
      lcd.print("  Ac/Kapat/Oto  ");
      break;
    case 4:
      lcd.setCursor(0, 0);
      lcd.print(" Isitici kontrol");
      lcd.setCursor(0, 1);
      lcd.print("  Ac/Kapat/Oto  ");
      break;
    case 5:
      lcd.setCursor(0, 0);
      lcd.print("   Fan kontrol  ");
      lcd.setCursor(0, 1);
      lcd.print("  Ac/Kapat/Oto  ");
      break;
    case 6:
      lcd.setCursor(0, 0);
      lcd.print(" Yedek su alarmi");
      lcd.setCursor(0, 1);
      lcd.print("   Ac / Kapat   ");
      break;
    case 7:
      lcd.setCursor(0, 0);
      lcd.print(" Motor1 kontrol ");
      break;
    case 8:
      lcd.setCursor(0, 0);
      lcd.print(" Role 5 kontrol ");
      break;
    case 9:
      lcd.setCursor(0, 0);
      lcd.print(" Role 6 kontrol ");
      break;
    case 10:
      lcd.setCursor(0, 0);
      lcd.print(" Role 7 kontrol ");
      break;
    case 11:
      lcd.setCursor(0, 0);
      lcd.print(" Role 8 kontrol ");
      break;
    case 12:
      lcd.setCursor(0, 0);
      lcd.print(" LCD Aydinlatma ");
      lcd.setCursor(0, 1);
      lcd.print("   Ac / Kapat   ");
      break;
    case 13:
      lcd.setCursor(0, 0);
      lcd.print(" Saat Ayarla    ");
      lcd.setCursor(0, 1);
      lcd.print("     Ayarla     ");
      break;
  }
  tusbekle();
  if (tus == "right" and menu_item == 1)
  {
    menu1();
  };
  if (tus == "right" and menu_item == 2)
  {
    menu2();
  };
  if (tus == "right" and menu_item == 3)
  {
    menu3();
  };
  if (tus == "right" and menu_item == 4)
  {
    menu4();
  };
  if (tus == "right" and menu_item == 5)
  {
    menu5();
  };
  if (tus == "right" and menu_item == 6)
  {
    menu6();
  };
  if (tus == "right" and menu_item == 7)
  {
    menu7();
  };
  if (tus == "right" and menu_item == 8)
  {
    menu8();
  };
  if (tus == "right" and menu_item == 9)
  {
    menu9();
  };
  if (tus == "right" and menu_item == 10)
  {
    menu10();
  };
  if (tus == "right" and menu_item == 11)
  {
    menu11();
  };
  if (tus == "right" and menu_item == 12)
  {
    menu12();
  };
  if (tus == "right" and menu_item == 13)
  {
    menu13();
  };
  if (tus == "right" and menu_item == 14)
  {
    return;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "up")
  {
    menu_item--;
  };
  if (menu_item == 0)
  {
    menu_item = 13;
  };
  if (tus == "down")
  {
    menu_item++;
  };
  if (menu_item >= 14)
  {
    menu_item = 1;
  };
  delay (500);
  goto llp;
}
void menu1()  //LED kontrol
{

m1lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aydinlatma");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  switch (menu_1_stat)
  {
    case 0:
      lcd.print("Kapali");
      break;
    case 1:
      lcd.print("Acik");
      break;
    case 2:
      lcd.print("Auto");
      break;
  };
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    menu_1_stat = menu_1_stat + 1;
    if (menu_1_stat >= 3)
    {
    menu_1_stat = 0;
  };
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m1lp;
}
void menu2()  //Aydinlatma zaman ayari
{
setledon:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Led Acilis: ");
  lcd.print(led_on);
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    goto setledoff;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    led_on = led_on + 1;
  };
  if (tus == "down")
  {
    led_on = led_on - 1;

  };
  goto setledon;
setledoff:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Led Kapanis: ");
  lcd.print(led_off);
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    goto setfloon;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    led_off = led_off + 1;
  };
  if (tus == "down")
  {
    led_off = led_off - 1;

  };

  goto setledoff;
setfloon:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FLO acilis: ");
  lcd.print(flo_on);
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    goto setflooff;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    flo_on = flo_on + 1;
  };
  if (tus == "down")
  {
    flo_on = flo_on - 1;

  };
  goto setfloon;
setflooff:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FLO Kapanis: ");
  lcd.print(flo_off);
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    return;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    flo_off = flo_off + 1;
  };
  if (tus == "down")
  {
    flo_off = flo_off - 1;

  };
  goto setflooff;
}
void menu3()  //Yedek Su Pompasi
{
m3lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Yedek Su Pmp");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  switch (menu_3_stat)
  {
    case 0:
      lcd.print("Kapali");
      break;
    case 2:
      lcd.print("Auto");
      break;
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    menu_3_stat = menu_3_stat + 2;
    if (menu_1_stat >= 3)
    {
      menu_1_stat = 0;
    };

  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m3lp;
}
void menu4()  //Isitici kontrol
{
m4lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Isitici");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  switch (menu_4_stat)
  {
    case 0:
      lcd.print("Kapali");
      break;
    case 1:
      lcd.print("Acik");
      break;
    case 2:
      lcd.print("Auto");
      break;
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    menu_4_stat = menu_4_stat + 1;
    if (menu_4_stat >2)
    {
      menu_4_stat = 0;
    }
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m4lp;
}
void menu5()  //Fan kontrol
{
m5lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fan");
  lcd.setCursor(0, 1);
  lcd.print("> ");
  switch (menu_5_stat)
  {
    case 0:
      lcd.print("Kapali");
      break;
    case 1:
      lcd.print("Acik");
      break;
    case 2:
      lcd.print("Auto");
      break;
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    menu_5_stat = menu_5_stat + 1;
    if (menu_5_stat >2)
    {
      menu_5_stat = 0;
    }
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m5lp;
}
void menu6()  //Yedek su alarmi
{
m6lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Su az Alarmi");
  lcd.setCursor(0, 1);
  if (min_tone == 1)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (min_tone == 0)
    {
      min_tone = 1;
    }
    else
    {
      min_tone = 0;
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m6lp;
}
void menu7()  //Motor 1 kontrol
{
m7lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("role4");
  lcd.setCursor(0, 1);
  if (digitalRead (role4) == HIGH)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (digitalRead (role4) == HIGH)
    {
      digitalWrite(role4, LOW);
      role4_stat="off";
    }
    else
    {
      digitalWrite(role4, HIGH);
      role4_stat="on";
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m7lp;
}
void menu8()  //Role 5 kontrol
{
m8lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("role5");
  lcd.setCursor(0, 1);
  if (digitalRead (role5) == HIGH)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (digitalRead (role5) == HIGH)
    {
      digitalWrite(role5, LOW);
      role5_stat="off";
    }
    else
    {
      digitalWrite(role5, HIGH);
      role5_stat="on";
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m8lp;
}
void menu9()  //Role 6 kontrol
{
m9lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("role6");
  lcd.setCursor(0, 1);
  if (digitalRead (role6) == HIGH)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (digitalRead (role6) == HIGH)
    {
      digitalWrite(role6, LOW);
      role6_stat="off";
    }
    else
    {
      digitalWrite(role6, HIGH);
      role6_stat="on";
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m9lp;
}
void menu10() //Role 7 kontrol
{
m10lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("role7");
  lcd.setCursor(0, 1);
  if (digitalRead (role7) == HIGH)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (digitalRead (role7) == HIGH)
    {
      digitalWrite(role7, LOW);
      role7_stat="off";
    }
    else
    {
      digitalWrite(role7, HIGH);
      role7_stat="on";
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m10lp;
}
void menu11() //Role 8 kontrol
{
m11lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("role8");
  lcd.setCursor(0, 1);
  if (digitalRead (role8) == HIGH)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (digitalRead (role8) == HIGH)
    {
      digitalWrite(role8, LOW);
      role8_stat="off";
    }
    else
    {
      digitalWrite(role8, HIGH);
      role8_stat="on";
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m11lp;
}
void menu12() //LCD Aydinlatma
{
m12lp:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Aydinlatma");
  lcd.setCursor(0, 1);
  if (lcd_led == 1)
  {
    lcd.print("> Acik");
  }
  else
  {
    lcd.print("> Kapali");
  }
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    if (lcd_led == 1)
    {
      lcd.noBacklight();
      lcd_led = 0;
    }
    else
    {
      lcd.backlight();
      lcd_led = 1;
    }
  }
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  goto m12lp;
}
void menu13() //Tarih Saat ayarla ToDo
{
setsaat:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saat: ");
  int cur_hour = hour();
  if (hour() < 10)
  {
    lcd.print("0");
    lcd.print(hour());
  }
  else
  {
    lcd.print(hour());
  };
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    goto setdak;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    cur_hour = hour() + 1;
    setTime(cur_hour, minute(), 00, day(), month(), year());
    rtc.set(now());
  };
  if (tus == "down")
  {
    cur_hour = hour() - 1;
    setTime(cur_hour, minute(), 00, day(), month(), year());
    rtc.set(now());
  };
  goto setsaat;

setdak:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dakika: ");
  int cur_min = minute();
  if (minute() < 10)
  {
    lcd.print("0");
    lcd.print(minute());
  }
  else
  {
    lcd.print(minute());
  };
  delay (500);
  tusbekle();
  if (tus == "right")
  {
    return;
  };
  if (tus == "left")
  {
    return;
  };
  if (tus == "menu")
  {
    return;
  };
  if (tus == "up")
  {
    cur_min = cur_min + 1;
    setTime(hour(), cur_min, 00, day(), month(), year());
    rtc.set(now());
  };
  if (tus == "down")
  {
    cur_min = cur_min - 1;
    setTime(hour(), cur_min, 00, day(), month(), year());
    rtc.set(now());
  };
  goto setdak;
}
int getwaterlevel()
{
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 58.3); //cm min 6,5cm 46,5cm
  level = (distance / 3.25);
  if (level < 2 and max_tone == 1)
  {
    tone (buzzer, 30);
  }
  return level;
}
void checklights()
{
  switch (menu_1_stat)
  {
    case 0:
      digitalWrite(led_relay, HIGH);
      led_stat="off";
      digitalWrite(flo_relay, HIGH);
      flo_stat="off";
      break;
    case 1:
      digitalWrite(led_relay, LOW);
      led_stat="on";
      digitalWrite(flo_relay, LOW);
      flo_stat="on";
      break;
    case 2:
      if (hour() > led_on and hour() < led_off)
      {
        digitalWrite(led_relay, LOW);
      }
      else
      {
        digitalWrite(led_relay, HIGH);
      };
      if (hour() > flo_on and hour() < flo_off)
      {
        digitalWrite(flo_relay, LOW);
      }
      else
      {
        digitalWrite(flo_relay, HIGH);
      };
      led_stat="auto";
      flo_stat="auto";
      break;
  };

}
void checktemperature()
{
gettemp:
  sensors.requestTemperatures(); //get temperature to display
  delay(2000);
  temperature = sensors.getTempCByIndex(0);
  if (temperature > 0 and temperature < 24.0)
  {
    if ( menu_4_stat == 2)
    {
      digitalWrite(role2, HIGH);   //heater on
    }
    if ( menu_5_stat == 2)
    {
      digitalWrite(role3, LOW);   //cooler fan off
    }
  }
  if (temperature > 24.0 and temperature < 26.5)
  {
    if ( menu_4_stat == 2)
    {
      digitalWrite(role2, LOW);   //heater off
    }
    if ( menu_5_stat == 2)
    {
      digitalWrite(role3, LOW);   //cooler fan off
    }
  }
  if (temperature > 26.5)
  {
    if ( menu_4_stat == 2)
    {
      digitalWrite(role2, LOW);   //heater off
    }
    if ( menu_5_stat == 2)
    {
      digitalWrite(role3, HIGH);   //cooler fan on
    }
  }
  if (temperature < 1)          //if heat is 0.0 then reset sensors
  {
    digitalWrite(sens_power, LOW);
    delay(1000);
    digitalWrite(sens_power, HIGH);
    goto gettemp;
  }
  switch (menu_4_stat)
  {
      case 0:
        digitalWrite(role2, LOW);
        break;
      case 1:
        digitalWrite(role2, HIGH);
        break;
  }
  switch (menu_5_stat)
  {
      case 0:
        digitalWrite(role3, LOW);
        break;
      case 1:
        digitalWrite(role3, HIGH);
        break;
  }
}
void checkwaterlvl()
{
  switch (menu_3_stat)
  {
    case 0:
      digitalWrite(role1, LOW);
      break;
    case 1:
      digitalWrite(role1, HIGH);
      break;
    case 2:
      if (digitalRead(sump) == LOW)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Su ekleniyor... ");
        digitalWrite(role1, HIGH); // Water pump on
        delay (1000);
        digitalWrite(role1, LOW); // Water pump off
        //delay(1000);
      };
      break;

  }
}
void debuglog(String logtext)
{
#if defined(DEBUGMODE)
  Serial.print ("-->");
  Serial.println(logtext);
#endif
}
void senddatatoserver()
{
  char sicaklik[5] = "";
  dtostrf (temperature, 2, 2, sicaklik);
  postdata = "role1=";  //spare water pump
  switch (menu_3_stat)
  {
    case 0:
    postdata += "off";
    break;
    case 1:
    postdata += "on";
    break;
    case 2:
    postdata += "auto";
    break;
  }
  postdata += "&role2=";  //heater
  switch (menu_4_stat)
  {
    case 0:
    postdata += "off";
    break;
    case 1:
    postdata += "on";
    break;
    case 2:
    postdata += "auto";
    break;
  }
  postdata += "&role3=";  //fan
  switch (menu_5_stat)
  {
    case 0:
    postdata += "off";
    break;
    case 1:
    postdata += "on";
    break;
    case 2:
    postdata += "auto";
    break;
  }
  postdata += "&role4=";
  postdata += role4_stat;
  postdata += "&role5=";
  postdata += role5_stat;
  postdata += "&role6=";
  postdata += role6_stat;
  postdata += "&role7=";
  postdata += role7_stat;
  postdata += "&role8=";
  postdata += role8_stat;
  postdata += "&ledstat=";
  postdata += led_stat;
  postdata += "&flostat=";
  postdata += flo_stat;
  postdata += "&temp=";
  postdata += sicaklik;
  postdata += "&waterlvl=";
  postdata += String(level);
  debuglog("Sending data...");
  //debuglog ("POST /akvaryum HTTP/1.0\r\nContent-Length: " + String(postdata.length()) + "\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n" + postdata);
  wifi.send (SERVER, "POST /akvaryum HTTP/1.1\r\nContent-Length: " + String(postdata.length()) + "\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n" + postdata, true);

}
void loop()
{
  //debuglog("checking lights");
  checklights(); //check lighting status
  //debuglog("checking temperature");
  checktemperature();//check temperature
  //debuglog("checking waterlvl");
  checkwaterlvl();//check sump water level
  noTone(buzzer);//silence buzzer
  lcd.clear();
  lcd.setCursor(0, 0); //get temperature to display
  lcd.print (temperature, 1);
  lcd.print ("\xDF");
  lcd.print("C");
  lcd.setCursor(8, 0); //show wifi status
  //debuglog("checking wifi");
  if(wifi.isConnectedToAP())
  {
    lcd.print ("\xB7");
  }
  else
  {
    lcd.print ("x");
  }
  lcd.setCursor(11, 0);//show date time
  if (hour() < 10)
  {
    lcd.print("0");
    lcd.print(hour());
  }
  else
  {
    lcd.print(hour());
  }
  if (nv)
  {
    lcd.print(":");
    nv = false;
  }
  else
  {
    lcd.print(" ");
    nv = true;
  }
  if (minute() < 10)
  {
    lcd.print("0");
    lcd.print(minute());
  }
  else
  {
    lcd.print(minute());
  }
  level = getwaterlevel ();//get water level to display
  lcd.setCursor(0, 1);
  for (int x = 0; x < 17 ; x++)
  {
    if (x <= (16 - level)) lcd.print("\xFF");
    if (x > (16 - level)) lcd.print("\xDB");
  }
  for (int say = 1; say < 2000; say ++) //wait for 2sec for a key press
  {
    if (digitalRead(2) == LOW)
    {
      entermenu();
    }
    delay(1);
  }
  currentmillis = millis();
  if (((currentmillis - previousupdate) > 30000)) //send data if 30 sec passed
  {
    senddatatoserver(); //send data to server
    previousupdate =  currentmillis;
  }

}
