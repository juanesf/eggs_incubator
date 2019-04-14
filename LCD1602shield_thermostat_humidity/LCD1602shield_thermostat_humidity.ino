/*
original THERMOSTAT with HUMIDITY CONTROL use classical LCD1602
at http://nicuflorica.blogspot.ro/2015/08/termostat-cu-control-umiditate.html
ver.1m0 - 28.08.2015, Craiova - Romania, Europe, Earth
actual use LCD1602 shield 
ver.1s0 - 15.11.2017, Craiova - Romania 
ver.2s0 - use EEPROM memory to store value
ver.3s0 - progresive backlight control
ver.3s1 - add LEFT and RIGHT buttons

niq_ro write this sketch using parts of other sketches :)
http://www.tehnic.go.ro
http://nicuflorica.blogspot.ro/
http://arduinotehniq.blogspot.com/
*/

//2019 mod by juanesf

// http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/
#include <EEPROM.h>

#include <LiquidCrystal.h>  //this library is included in the Arduino IDE
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
/*   shield                           -------------------
                                      |  LCD  | Arduino |
                                      -------------------
 LCD RS pin to digital pin 8          |  RS   |   D8    |
 LCD Enable pin to digital pin 9      |  E    |   D9    |
 LCD D4 pin to digital pin 4          |  D4   |   D4    |
 LCD D5 pin to digital pin 5          |  D5   |   D5    |
 LCD D6 pin to digital pin 6          |  D6   |   D6    |
 LCD D7 pin to digital pin 7          |  D7   |   D7    |
 LCD R/W pin to ground                |  R/W  |   GND   |
                                      -------------------
*/

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};


// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
#include "DHT.h"
#define DHTPIN 13     // what pin we're connected to 13
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// if is just sensor:
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 4k7 or 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

// define pins for relay or leds
#define heater 11
#define humidifier 12
#define ventilation 3
#define extraction 2

// define variable

float Temp, t1, tes, dete;   // variable for temperature
int Hum, hass, dehas;        // variable for humiditiy


// part for menu: http://nicuflorica.blogspot.ro/2015/05/termostat-cu-afisaj-led.html
/*
#define BUT1 6    // - switch
#define BUT2 7    // + switch
#define BUT3 8    // MENU switch
*/

byte menu = 0; // if MENU = 0 is clasical 
                // if MENU = 1 is for temperature set (tes)
                // if MENU = 2 is for dt temperature (dete)
                // if MENU = 3 is for humidity set (hass)
                // if MENU = 4 is for dh humidity (dehas)
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// https://nicuflorica.blogspot.ro/2017/08/iluminat-de-fundal-progresiv-pentru.html
#define backlight 10   // backlight
int comienzo = 0;  // time in second when push the button
int actual = 0;   // actual time in second
int lastPress;  // sore time when push last time the button
int timeBacklight = 5; // time for maximum value
byte minBacklight = 25;  // minimum value for backlight
byte maxBacklight = 255; // maximum value for backlight
byte valBacklight = 25;  // value for backlight
byte valStep = 5;  // step for decrease light

void setup()
{

// Initialize LCD display  
 lcd.begin(16, 2);
// create custom symbol
lcd.createChar(0, grad);

// Initialize DHT sensor
  dht.begin();

// define outputs
 pinMode(heater, OUTPUT);  
 pinMode(humidifier, OUTPUT);
 pinMode(ventilation, OUTPUT);
 pinMode(extraction, OUTPUT);

// set the default state for outputs
  digitalWrite(heater, LOW);
  digitalWrite(humidifier, LOW);
  digitalWrite(ventilation, LOW);
  digitalWrite(extraction, LOW);

/*
// set push buttons for menu
  pinMode(BUT1, INPUT);
  pinMode(BUT2, INPUT);
  pinMode(BUT3, INPUT);
  
  digitalWrite(BUT1, HIGH); // pull-ups on
  digitalWrite(BUT2, HIGH);
  digitalWrite(BUT3, HIGH);
*/
  lcd.setCursor(0,0);  
  lcd.print("thermostat with");
  
  lcd.setCursor(0,1);  
  lcd.print("humidity control");

delay(3000);
lcd.clear();

  lcd.setCursor(0,0);  
  lcd.print("Mod sketch");
  
  lcd.setCursor(6,1);  
  lcd.print("by juanesf");
delay(3000);
lcd.clear();

/*
// preset value for temperature and humidity
tes = 28.9;   
dete = 0.2;
hass = 58;
dehas = 2;
*/

/*
// just first time... after must put commnent (//)
EEPROM.write(201,0);    // tset1
EEPROM.write(202,245);  // tset2
EEPROM.write(203,5);    // dt x 10
EEPROM.write(204,45);   // hass
EEPROM.write(205,5);    // dehas
 */
  
byte tset1 = EEPROM.read(201);
byte tset2 = EEPROM.read(202);
tes = 256 * tset1 + tset2;  // recover the number
tes = tes/10;
dete = EEPROM.read(203);
dete = dete/10;
hass = EEPROM.read(204);
dehas = EEPROM.read(205);  

analogWrite(backlight, valBacklight); 
comienzo = millis()/1000;  // store value for begining of program

}

void loop()
{

lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnSELECT:
     {
     menu = menu + 1;
     delay(1000);
     lcd_key = btnNONE;
     lcd.clear();
     break;
     }
     case btnNONE:
     {
     break;
     }
   }


if (menu >= 5) menu = 0;

if (menu == 0)
{

lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnSELECT:
     {
     menu = 1;
     delay(1000);
     lcd_key = btnNONE;
     lcd.clear();
     break;
     }
     case btnNONE:
     {
     break;
     }
   }

   // Reading temperature or humidity 
 Hum = dht.readHumidity();

 Temp = dht.readTemperature(); 
// int Temp = dht.readTemperature(); 

delay(500);

/*
  lcd.setCursor(0,0);  
  lcd.print("t = ");
  lcd.setCursor(5,0);  
  lcd.print(Temp,1);
  lcd.print("^C  ");
  
   
  lcd.setCursor(0,1);  
  lcd.print("h = ");
  lcd.setCursor(5,1);  
  lcd.print(Hum);
  lcd.print("%rh  ");
*/

// part from http://nicuflorica.blogspot.ro/2014/10/ceas-rtc-cu-ds1307-si-date-mediu-cu.html
 lcd.setCursor(0, 0);
  // lcd.print("t=");
 if (Temp < 0) 
 {t1=-Temp;}
 else t1=Temp;
  
    if ( t1 < 10)
   {
     lcd.print(" "); 
   }
   if (Temp>0) lcd.print("+"); 
   if (Temp==0) lcd.print(" "); 
   if (Temp<0) lcd.print("-");
   lcd.print(t1,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(10, 0);
  // lcd.print("H=");
   lcd.print(Hum);
   lcd.print("%RH");

/*
   lcd.setCursor(1, 1);
   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");
  lcd.setCursor(10, 1);
  // lcd.print("H=");
  lcd.print(hass);
  lcd.print("%RH");
*/

if (Temp > tes) 
 {
 digitalWrite(heater, LOW);
 digitalWrite(extraction, HIGH);
 digitalWrite(ventilation, HIGH);
   lcd.setCursor(0, 1);
   lcd.print("ALTO ");
 } 
if (tes - dete > Temp)
  {
   digitalWrite(heater, HIGH);
   digitalWrite(extraction, LOW);
   digitalWrite(ventilation, LOW);
   lcd.setCursor(0, 1);
   lcd.print("BAJO");
 } 
if ((Temp <= tes) and (tes - dete <= Temp))
 {
   lcd.setCursor(0, 1);
   lcd.print("OK !  ");
 } 


 if (Hum > hass)
 {
 digitalWrite(humidifier, LOW);
 digitalWrite(extraction, HIGH);
 digitalWrite(ventilation, HIGH);
   lcd.setCursor(10, 1);
   lcd.print("ALTO ");
 }
 if (Hum < hass - dehas)
 {
   digitalWrite(humidifier, HIGH);
   digitalWrite(extraction, LOW);
   digitalWrite(ventilation, LOW); 
   lcd.setCursor(10, 1);
   lcd.print("BAJO");
 }
 if ((Hum <= hass) and (Hum >= hass - dehas))
 {  lcd.setCursor(10, 1);
   lcd.print("OK !  ");
}

actual = millis()/1000;
if ((actual - lastPress > timeBacklight) &&  (valBacklight > minBacklight))  // daca a trecut timpul de aprindere lumina backlight la maxim si nu s-a stins la minim
{
  valBacklight = valBacklight - valStep;   // nivel aprindere in scadere
  if (valBacklight < minBacklight) valBacklight = minBacklight; // nu trebuie sa scada valaorea sub minim
  analogWrite(backlight, valBacklight);
  lcd.print(valBacklight);
}
} // last line for MENU = 0


if (menu == 1) {
  while (menu == 1) {
//   teset(tset);
   lcd.setCursor(0, 0);
   lcd.print("Temperature SET:");
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");
   
lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
    case btnLEFT:
     {
     menu = 0;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
   case btnUP:
     {
     tes = tes + 0.1;
     delay(250);
int tes2 = tes*10;  
byte tset1 = tes2 / 256;
byte tset2 = tes2 - tset1 * 256;
EEPROM.write(201, tset1);  // partea intreaga
EEPROM.write(202, tset2);   // rest
     break;
     }
   case btnDOWN:
     {
     tes = tes - 0.1;
int tes2 = tes*10;  
byte tset1 = tes2 / 256;
byte tset2 = tes2 - tset1 * 256;
EEPROM.write(201, tset1);  // partea intreaga
EEPROM.write(202, tset2);   // rest
     delay(250);
     break;
     }
     case btnSELECT:
     {
     menu = 2;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
     case btnRIGHT:
     {
     menu = 2;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
     case btnNONE:
     {
     break;
     }
   }

     }
   delay (100);
}   // end loop for MENU = 1


if (menu == 2) {
 // if (tmax <= tmin) tmax = tmin + 10;
   while (menu ==2) {
 //    maxim(dt/10); // this is number to diplay
 //    dete(dt); // this is number to diplay
   lcd.setCursor(0, 0);
   lcd.print("hyst.temp. SET:");
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("C  ");

 lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
     case btnLEFT:
     {
     menu = 1;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
   case btnUP:
     {
     dete = dete + 0.1;
if (dete > 2.) dete = 2.;
EEPROM.write(203,dete*10);   
     delay(250);
     break;
     }
   case btnDOWN:
     {
     dete = dete - 0.1;
if (dete < 0.1) dete = 0.1;
EEPROM.write(203,dete*10);   
     delay(250);
     break;
     }
   case btnSELECT:
     {
     menu = 3;
     lcd_key = btnNONE;
     delay(1000);
     lcd.clear();
     break;
     }
     case btnRIGHT:
     {
     menu = 3;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
     case btnNONE:
     {
     break;
     }
   }

//   delay(15);
if (dete < 0.1) dete = 0.1;
    }
}  // end loop for MENU = 2


if (menu == 3) {
  while (menu == 3) {
//   teset(tset);
   lcd.setCursor(0, 0);
   lcd.print("Humidity SET:");
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass,1);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

 lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
     case btnLEFT:
     {
     menu = 2;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
   case btnUP:
     {
     hass = hass + 1;
if (hass > 99) hass = 99;     
EEPROM.write(204,hass);    
     delay(250);
     break;
     }
   case btnDOWN:
     {
     hass = hass - 1;
if (hass < 1) hass = 1;     
EEPROM.write(204,hass);    
     delay(250);
     break;
     }
   case btnSELECT:
     {
     menu = 4;
     lcd_key = btnNONE;
     delay(1000);
     lcd.clear();
     break;
     } 
     case btnRIGHT:
     {
     menu = 4;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
     case btnNONE:
     {
     break;
     }
   }
  
//  delay(15);
     }
   delay (100);
}  // end loop for MENU =3


if (menu == 4) {
   while (menu ==4) {
   lcd.setCursor(0, 0);
   lcd.print("hyst.humid. SET:");
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("%RH");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
   lcd.write(byte(0));
   lcd.print("%RH");

 lcd_key = read_LCD_buttons();  // read the buttons
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnLEFT:
     {
     menu = 3;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
   case btnUP:
     {
     dehas = dehas + 1;
if (dehas > 99) dehas = 99;
EEPROM.write(205,dehas); 
     delay(250);
     break;
     }
   case btnDOWN:
     {
     dehas = dehas - 1;
if (dehas < 1) dehas = 1;     
EEPROM.write(205,dehas); 
     delay(250);
     break;
     }
   case btnSELECT:
     {
      menu = 5;
      lcd_key = btnNONE;
     delay(1000);
     lcd.clear();
     break;
     }
     case btnRIGHT:
     {
     menu = 5;
     delay(1000);
     lcd.clear();
     lcd_key = btnNONE;
     break;
     }
     case btnNONE:
     {
     break;
     }
   }
 
//   delay(15);
if (dehas < 1) dehas = 1;
    }

}



}   // last line in main loop

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
if (adc_key_in < 1000)
{
 valBacklight = maxBacklight;
      analogWrite(backlight, valBacklight);  // backlight at maximum
      delay(50);     
      lastPress = millis()/1000;  // store last time whem button is pushed
}
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 //if (adc_key_in < 50)   return btnRIGHT;  
 //if (adc_key_in < 250)  return btnUP; 
 //if (adc_key_in < 450)  return btnDOWN; 
 //if (adc_key_in < 650)  return btnLEFT; 
 //if (adc_key_in < 850)  return btnSELECT;  

 // For V1.0 comment the other threshold and use the one below:
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   

 return btnNONE;  // when all others fail, return this...
}
