/*
Measures 2 things, the depth of the tank and whether the water input is running or not.  Records the info on SD
and uploads it to xivley.com.  


Key examples I am pulling code from:
* TimeNTP: uses wifi to get date info from internet
* xively_wifiDatastreamupload: how to upload data live to xively for remote monitoring
* datalogger:  example of how to write data to SD card


8/5/2014:  This code works!  It gets internet time once, then uses the interal clocks to increment.  does not attempt to log to the internet
8/7/2014:  logged well for 3 days.  Fixed tank specs and cleaned up data format.
*/

// Defines for Wifi ///////////////////////////////////
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "Rob"; //  your network SSID (name) 
char pass[] = "5125764654";    // your network password (use for WPA, or use as key for WEP)
//int keyIndex = 0;            // your network key Index number (needed only for WEP)
//
int status = WL_IDLE_STATUS;

// defines for the SD logging /////////////////////////////
#include <SD.h>
const int chipSelect = 4;
/////////////////////////////////////////////////////////////
// defines for timer //////////////////////////////
//#include <Time.h>
//#include <Wire.h>
//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
////////////////////////////////////////////////////

// defines for the sonic sensor://////////////////////////
#include <NewPing.h>

#define TRIGGER_PIN  2      //can be any i/o
#define ECHO_PIN     5      //can be any i/o
#define MAX_DISTANCE 150
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
////////////////////////////////////////////////////////////







/////////////////////////////////////////////////////////
//pins used:  wifi shield: 7,10, 11,12,13.  SD card 4

// Defines for the water sensor (just a wire...) //////////
const int water_input_sensor=8;      //can be any i/o
//////////////////////////////////////////////////////////


// physical tank specs.  Record here /////////////////////
const float f_empty_distance=69;         //in cm
const float f_full_distance=11;            //in cm
const float f_min_depth=5;

float f_tank_range;
const int i_tank_capacity_gallons=5500;

//////////////////////////////////////////////////////////

// other constants here ////////////////////////////////
const int reading_delay=1000;            //frequency of readings, in ms

/////////////////////////////////////////////

////////////////////internet time defines here////////////////////
unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
//////////////////////////////////////////////////

float f_depth=0;
float f_percent=0;
float f_my_sonar_reading=0;
float f_level_in_G=0;

//int depth=0;
//int percent=0;
//int my_sonar_reading=0;
String dataString;
//String s_filename = "data807.csv";
int temp_int;

unsigned long running_timer=0;
int first_time=0;
unsigned long epoch;  
long elapsed_time=0;
long last_millis=0;

//*******************************   SETUP    *************************************
void setup() {
  Serial.begin(9600);
  
 // setup SD card
 Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  // end SD card setup
  
  
  pinMode(water_input_sensor, INPUT);      
  f_tank_range=f_empty_distance-f_full_distance+f_min_depth;    // the working range of the tank, in cm 
  
  //set up the clock
//  setSyncProvider(RTC.get);   // the function to get the time from the RTC
//  if(timeStatus()!= timeSet) 
//     Serial.println("Unable to sync with the RTC");
//  else
//     Serial.println("RTC has set the system time");     
    
    
// //connect to wifi:   
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  } 
  Serial.println("Connected to wifi");
  printWifiStatus();   
  
  
  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
  
   pinMode(water_input_sensor, INPUT);           // set pin to input
   digitalWrite(water_input_sensor, HIGH);       // turn on pullup resistors

  
}

//*******************************  END SETUP    *************************************

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  MAIN LOOP   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void loop() {





if (first_time==0) {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Serial.println("first time executing");
sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  //Serial.println( Udp.parsePacket() );
  if ( Udp.parsePacket() ) {
    //Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = " );
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    
    const unsigned long onehundredtenYears=3471264720UL;
    // subtract seventy years:
    epoch = secsSince1900 - onehundredtenYears;
    // print Unix time:
    Serial.println(epoch);    // seconds since Jan 1 2010
    elapsed_time=millis()/1000;
    last_millis=millis()/1000;
    first_time=1;
    
    //print a header to the file
    File dataFile = SD.open("data807.csv", FILE_WRITE);
    Serial.println("new data run starting");
    Serial.println("Time stamp,Raw,depth,percent,level,water_sensor");
    dataFile.close();
  }
  
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //end if first time
  delay(reading_delay);
  
   File dataFile = SD.open("data807.csv", FILE_WRITE);
   
   
   dataString="";
   // if the file is available, write to it:
   if (dataFile) {
      
  
    // do the actual measurement at hand:
    float uS = sonar.ping();                          //get sonar reading
    
    f_my_sonar_reading=(uS / US_ROUNDTRIP_CM);
    temp_int=int(f_my_sonar_reading);
  

long temp;

    temp=millis()/1000;

 
    elapsed_time=temp-last_millis;
    last_millis=temp;

    
    epoch+=elapsed_time;
    dataString += epoch;
    dataString += ",";
    dataString += String(temp_int);
    dataString += ",";
    f_depth= f_tank_range-(f_my_sonar_reading-f_full_distance)+f_min_depth;
    temp_int=f_depth;
    dataString += temp_int;
    f_percent=100*f_depth/f_tank_range;
    dataString += ",";
    temp_int=f_percent;
    dataString += temp_int;
    
    f_level_in_G=i_tank_capacity_gallons*(f_percent);
    temp_int=f_level_in_G;
    dataString += ",";
    dataString += temp_int;
    
    
    dataString += ",";
    dataString += digitalRead(water_input_sensor);
   
    Serial.println(dataString);            //print debug
    dataFile.println(dataString);           // print to file   
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file");
    
    
  } 
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   END MAIN LOOP @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@




// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm \n");
}

