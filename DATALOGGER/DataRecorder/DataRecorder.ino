#include <Adafruit_BME280.h>
#include <Adafruit_BNO08x.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_HMC5883_U.h>
const int chipSelect = 3;//For sdcard CS (this can be any open digital pin yo)

//I2C addresses
#define SCREEN_ADDRESS 0x3C
#define BME280_OUTSIDE_ADDRESS 0x76
#define BME280_INSIDE_ADDRESS 0x77
#define DOF_ADDRESS 0x4A
#define Compass_ADDRESS 0x1E

//Screen setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool SDCARDWORKING = false;

//GPS setup
#define RX_PIN 6
#define TX_PIN 7
SoftwareSerial gpsSerial(RX_PIN, TX_PIN);
TinyGPSPlus gps;
bool GPSWORKING = false;



//BME280 setup
Adafruit_BME280 bmeInside;
bool BMEIWORKING = false;
Adafruit_BME280 bmeOutside;
bool BMEOWORKING = false;
//End BME280

//BNO0x setup
Adafruit_BNO08x  bno08x(-1);
bool BNO08xWORKING = false;
sh2_SensorValue_t sensorValue;
//BNO0x end

//Servoboard
int sunServoPin = 2;
int TTStart = 69540;

//File for sdCard to write to, maybe this should be the date and time?? idk
String logFileName = "datalog.txt";



struct GpsData {
  float latitude;
  float longitude;
  float alt;
  int sats;
  float time;//seconds sence the day began
};

struct RotationData {
  float iHat;
  float jHat;
  float kHat;
};


void setup() {
  Serial.begin(9600);
  //SD card Setup
  pinMode(chipSelect,OUTPUT);//This is for the sdcard spi interface
  if (!SD.begin(chipSelect)) {SDCARDWORKING = false;Serial.println("SD fail to begin");}
  else{SDCARDWORKING = true;}//This should flag that sd card did not initalize
  //End SD card setup

  //BME 280 Setup
  if(!bmeInside.begin(BME280_INSIDE_ADDRESS)){BMEIWORKING = false;}
  else{BMEIWORKING = true;}
  if(!bmeOutside.begin(BME280_OUTSIDE_ADDRESS)){BMEOWORKING = false;}
  else{BMEOWORKING = true;}
  //END BME 280 Setup

  //GPS setup
  gpsSerial.begin(9600);
  //End GPS setup

  //Screen setup
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //End screen setup

  //9dof setup
  bno08x.begin_I2C();
  bno08x.enableReport(SH2_GAME_ROTATION_VECTOR);

  //9dof end setup
  //servosetup
  pinMode(sunServoPin,OUTPUT);
  

}
//Data Format Time, 
void loop() {
  //GPSstuff
  GpsData vlGPSData = GetGPSData(); 
  
  if(vlGPSData.time>TTStart || 1==(int(vlGPSData.time/60)%5)){
    digitalWrite(sunServoPin,HIGH);
  }
  else{
    digitalWrite(sunServoPin,LOW);
  }
  
  //BME 280 data collection

  float Itemp = bmeInside.readTemperature();
  float IPres = bmeInside.readPressure();
  float IHum = bmeInside.readHumidity();
  float Otemp = bmeOutside.readTemperature();
  float OPres = bmeOutside.readPressure();
  float OHum = bmeOutside.readHumidity();
//time  lat lon alt sats Itemp Ipres Ihum  Otemp Opress  OHum  xorentation yorentation zorentation
  String writeString = String(vlGPSData.time,0) + "\t"+ String(vlGPSData.latitude,8) + "\t"+ String(vlGPSData.longitude,8) +"\t" +String(vlGPSData.alt,8) +"\t" + String(vlGPSData.sats,0) + "\t"+ String(Itemp,1)+"\t"+String(IPres,1)+"\t"+String(IHum,1)+"\t"+String(Otemp,1)+"\t"+String(OPres,1)+"\t"+String(OHum,1)+"\t"+String(GetRotationData().iHat,5)+"\t"+String(GetRotationData().jHat,5)+"\t"+String(GetRotationData().kHat,5);
  WriteData(writeString);
  Serial.println(writeString);
  DisplayStatus();
  Serial.println("*******************************");

}


void WriteData(String dataToWrite){
  File dataFile = SD.open(logFileName, FILE_WRITE);// opening SD card file
  if (dataFile) {
    dataFile.println(dataToWrite);//Print to the datafile
    dataFile.close();//Close file
    // print to the serial port too:
    Serial.println("Wrote Data sucsesfuly");
    Serial.println(dataToWrite);
  }
  else{
    Serial.println("Failed to write to sdcard");
    SDCARDWORKING = false;
  }//THIS NEEDS TO INDICATE ON THE SCREEN THAT IT FAILED
}

void DisplayStatus(){
  display.clearDisplay();
  Serial.println("Drawing on screen");
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);
  display.println("SDCARD status: "+String(SDCARDWORKING));
  display.println("GPS status: "+String(GPSWORKING));
  display.print("BMEO: "+String(BMEOWORKING)+"  ");
  display.println("BMEI: "+String(BMEIWORKING));
  display.println("9DOF: "+String(BNO08xWORKING));
  display.display();
}


RotationData GetRotationData(){
RotationData cR;
bno08x.getSensorEvent(&sensorValue);
cR.iHat = sensorValue.un.gameRotationVector.i;
cR.jHat = sensorValue.un.gameRotationVector.j;
cR.kHat = sensorValue.un.gameRotationVector.k;
BNO08xWORKING = true;
return cR;
}

//THis neeeds testing to figure out how quick you can grab data
GpsData GetGPSData(){
  bool newD = false;
  for(unsigned long start = millis();millis()-start<1000;){
    if (gpsSerial.available() > 0 && !newD) {
      if (gps.encode(gpsSerial.read())) {
        if (gps.location.isValid()) {
          Serial.print("Latitude: ");
          Serial.println(gps.location.lat(), 8);
          Serial.print("Longitude: ");
          Serial.println(gps.location.lng(), 8);
          newD = true;
          GPSWORKING = true;
        } else {
          Serial.println("GPS data not valid");
          GPSWORKING = false;
        }
      }
    }
  }
  //Packaging the data to go back to the function
  GpsData cdata;
  cdata.latitude = gps.location.lat();
  cdata.longitude = gps.location.lng();
  cdata.alt = gps.altitude.meters();
  cdata.sats = gps.satellites.value();
  cdata.time = (gps.time.hour()*60*60)+(gps.time.minute()*60)+gps.time.second();
  Serial.print("time: ");
  Serial.println(cdata.time);
  return cdata;
}
