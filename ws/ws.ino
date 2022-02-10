/* WWSK Weatherstation
 *  
 * Features
 *  - Wind speed
 *  - Wind direction
 *  - Barometic pressure
 *  - Temperature
 *  
 *  All information will be displayed on an LCD display
 *  and regularly sent to a webserver for more advanced display and historic data
 *  
 */

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <avr/wdt.h>
   
// Define 
//#define DEBUG		//If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG		//Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif


// LCD I2C
LiquidCrystal_I2C  lcd(0x3f,2,1,0,4,5,6,7); // 0x3F is the I2C bus address for the LCD

const int WindSpeedPin = A0;
const int WindDirectionPin = A1;
const int TemperaturePin = A2;

// Ethernet card

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xDA, 0xFC };
const char server[] = "ws.wwsk.be";

// BMP280 Pressure and temperature sensor

Adafruit_BMP280 bmp;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

int WindSpeedSensorValue = 0;       // Raw input from analog port
int WindSpeedAvgCount = 0;          // Count number of measurements in order to calculate average 

float WindSpeedValue = 0;           // Speed measured in knots from 0-62,98
float WindSpeedValueMax = 0;        // Max Speed measured during interval
float WindSpeedValueSum = 0;        // Sum of all speed measured during interval

int WindDirectionSensorValue = 0;
int WindDirectionValue = 0;
const char *CardinalDirections[] = {"  N","NNO"," NO","ONO","  O","OZO"," ZO","ZZO","  Z","ZZW"," ZW","WZW","  W","WNW"," NW","NNW","  N"};
byte CardinalDirectionIndex = 0;

float Temperature = 0;
float Pressure = 0;

// Interrupt variables
volatile int tc = 0 ;                   // timer counter
volatile boolean TimeToSend = false;    // Boolean to store 
char SendError = ' ';                   // Character to display on LCD in case of network issues
byte ErrorCount = 0;                    // Keep track of number of failures

// String manipulation variables
char lcdstring[17];
char datastring[80];
char spd[6];
char gst[6];
char temp[6];
char prs[8];


void setup() {
  // put your setup code here, to run once:

  // initialize serial communications at 9600 bps
  #ifdef DEBUG
    Serial.begin(9600);
    DPRINTLN("Debugging");
  #endif
  
  // Disable watchdog, to avoid continuous reset loops
  wdt_disable();
   
  // activate LCD module
  
  DPRINTLN("Activate LCD");
  
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear ();

  DPRINTLN("Activate LCD success");

  DPRINTLN("Activate BMP");
  
  if (!bmp.begin(0x76,0x58)) {  
    lcd.println(F("Could not initialize BMP280 sensor"));
    delay(9000);
  }
  
  DPRINTLN("Activate BMP succes");
  
  read_pressure();
  read_temperature();
  
  DPRINTLN("Start Network");
  
  lcd.println(F("Start Network   "));
      
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    wdt_enable(WDTO_8S);        // Enable watchdog, arduino will automatically reset after 8s if it can't find the DHCP server
    lcd.clear ();
    lcd.println(F("DHCP Failure    "));
    delay(9000);
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  
  DPRINTLN("Start Network success");
  
  lcd.clear ();
  
  //Set interrupt timer to 4 seconds
  
  noInterrupts();
  
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 65535;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts();//allow interrupts

  // Enable the watchdog
  
  wdt_enable(WDTO_8S);
  
}

ISR(TIMER1_COMPA_vect){
  //timer1 interrupt,wait for one minute and set boolean
  
  if (tc < 13){
    tc++ ;
    DPRINTLN(tc);
  }
  else{
    tc=0;
    TimeToSend=true;
  }
}


void read_wind_speed() {
  // read the analog value and convert to knots
  
  WindSpeedSensorValue = analogRead(WindSpeedPin);
  //WindSpeedSensorValue = constrain(WindSpeedSensorValue,81,412);	// constrain the value between 0.4 and 2 V
  WindSpeedSensorValue = constrain(WindSpeedSensorValue,4,1023);  // zero meter, compensate 0.18
  //WindSpeedValue = ((WindSpeedSensorValue - 81)/5.25);				    // convert to knots
  WindSpeedValue = ((WindSpeedSensorValue - 4)/16.195);            // convert to knots
  WindSpeedValueMax = max(WindSpeedValueMax,WindSpeedValue);		  // keep track of the highest windspeed
  WindSpeedValueSum = WindSpeedValueSum + WindSpeedValue;         // keep sum of all measurements to calculate average
  WindSpeedAvgCount = WindSpeedAvgCount + 1;                      // keep number of measurements
  
  dtostrf(WindSpeedValue, 4, 2, spd);     // float to string
  dtostrf(WindSpeedValueMax, 4, 2, gst);  // float to string
  
  DPRINTLN(F("WindSpeed/SensorValues/Max/Sum/Count"));
  DPRINTLN(WindSpeedSensorValue);
  DPRINTLN(WindSpeedValueMax);
  DPRINTLN(WindSpeedValueSum);
  DPRINTLN(WindSpeedAvgCount);
}

void read_wind_direction() {
  // read the analog value and convert to degrees 0 to 360
  
  WindDirectionSensorValue = analogRead(WindDirectionPin);
  WindDirectionSensorValue = constrain(WindDirectionSensorValue,82,410);	// constrain the value between 0.4 and 2 V
  WindDirectionValue = ((WindDirectionSensorValue - 82)*45/41);				// convert to degrees
  CardinalDirectionIndex = (map(WindDirectionValue, 0, 360, 0, 32) + 1)/2 ;
}

void read_pressure() {
  // read the atmospheric values
  
  Pressure = bmp.readPressure()/100;

  dtostrf(Pressure, 4, 0, prs);  // float to string
  
  DPRINTLN(F("Pressure"));
  DPRINTLN(Pressure);
}


void read_temperature() {
  // read the atmospheric values
  
  Temperature = (((analogRead(TemperaturePin) /204.8)  -0.517) / 0.01);
  
  dtostrf(Temperature, 3, 1, temp);     // float to string
    
  DPRINTLN(F("Temperature"));
  DPRINTLN(Temperature);
}


void print_values() {
  
  lcdstring[0] = (char)0;
  lcd.home (); // set cursor to 0,0 and clears the screen
  snprintf(lcdstring,17,"W: %s kn %s%c",spd,CardinalDirections[CardinalDirectionIndex],SendError);
  lcd.print(lcdstring);
  lcdstring[0] = (char)0;
  lcd.setCursor (0,1);        // go to start of 2nd line
  snprintf(lcdstring,17,"%s hPa %s%cC",prs,temp,(char)223);
  lcd.print(lcdstring); 
}

void send_data() {

  read_pressure();
  read_temperature();
  
  dtostrf((WindSpeedValueSum/WindSpeedAvgCount), 4, 2, spd);     // Calculate average speed and convert to string
  WindSpeedValueSum = 0;  // reset values
  WindSpeedAvgCount = 0;  // reset values

  DPRINTLN(F("Avg speed"));
  DPRINTLN(spd);
  
  if (ErrorCount > 60 ) {
    delay(9000);  // Force reset
  }  
  sprintf(datastring,"GET /insertdata.php?S=%s&G=%s&D=%i&T=%s&P=%s HTTP/1.1",spd,gst,WindDirectionValue,temp,prs);
  DPRINTLN(datastring);  
  DPRINTLN("Connecting");
  if (client.connect(server, 80)) {
    client.println(datastring);
    client.print(F("Host: "));
    client.println(server);
    client.println(F("Connection: close"));
    client.println();
    SendError = ' ';
    ErrorCount = 0;
    DPRINTLN("Data sent");
    }
    else {
      //you didn't get a connection to the server:
      DPRINTLN("Connect failed");
      SendError = 244;
      ErrorCount++;
    }
    client.stop();
    WindSpeedValueMax=0;
    TimeToSend=false;
}

void loop() {
  // put your main code here, to run repeatedly:
  wdt_reset();
  read_wind_speed();
  delay(250);
  read_wind_direction();
  delay(250);
  //read_temperature_and_pressure();
  print_values();
  if (TimeToSend) {
    send_data();
  }
  delay(50);
}
