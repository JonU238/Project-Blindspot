#include <PID_v1.h>
#include <TinyGPS++.h>
#include <math.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID SteerPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

TinyGPSPlus gps;

struct WayPoint
{
  float lat;
  float lon;
  float maxDistance;//Meters
};

struct GpsData {
  float latitude;
  float longitude;
  float alt;
};

Servo steerServo;
Servo dropServo;

WayPoint destination;
//CompassVVV
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified();

//Dropper values
  int dropALT = 1000;
  int openAngle = 180;
  int closeAngle = 0;
  bool droppingTime = false;


void setup(){
  //Serial Setup
  Serial.begin(9600);
  Serial1.begin(9600);

  //Servo setip
  steerServo.attach(2);
  dropServo.attach(3);

  //initialize the variables we're linked to
  Input = 0;
  Setpoint = 0;

  //turn the PID on
  SteerPID.SetMode(AUTOMATIC);
  //Program in location 45.81905218493044, -82.47359070390377
  destination.lat = 45.806;
  destination.lon = -82.477;

  
  
}

GpsData oldData = GetGPSData();
GpsData cData = GetGPSData();

void loop(){
  cData = GetGPSData();
  Serial.println(compHeading());
  //Dropper detection
  if(cData.alt>dropALT){
    droppingTime = true;
  }
  if(droppingTime){
    dropServo.write(openAngle);
  }
  else{
    dropServo.write(closeAngle);
  }
//END DROPPER SECTION

  Input = (HeadingBetweenTwoCoordinates(cData.latitude,cData.longitude,destination.lat,destination.lon)-compHeading());
  if(abs(Input)>180){Input = 360-Input;}///WTF is this for???(i wrote this code at 2-3am)
  Setpoint = 0;
  SteerPID.Compute();
  if(Output>180){Output=180;}
  if(Output<0){Output=0;}
  steerServo.write(Output);
  oldData = cData;
  //DebugVVV
  Serial.print("Output: ");
  Serial.println(Output);
  Serial.print("Input:");
  Serial.println(Input);
  Serial.print("Setpoint: ");
  Serial.println(Setpoint); 

}

GpsData GetGPSData(){
  bool newD = false;
  for(unsigned long start = millis();millis()-start<1000;){
    if (Serial1.available() > 0 && !newD) {
      if (gps.encode(Serial1.read())) {
        if (gps.location.isValid()) {
          Serial.print("Latitude: ");
          Serial.println(gps.location.lat(), 8);
          Serial.print("Longitude: ");
          Serial.println(gps.location.lng(), 8);
          newD = true;
        } else {
          //Serial.println("GPS data not valid");
        }
      }
    }
  }
  //Packaging the data to go back to the function
  GpsData cdata;
  cdata.latitude = gps.location.lat();
  cdata.longitude = gps.location.lng();
  cdata.alt = gps.altitude.meters();
  return cdata;
}


float HeadingBetweenTwoCoordinates(float lat1,float lon1,float lat2,float lon2){
  float deltay = (lat2-lat1);//in meters
  float deltax = (lon2-lon1);//in meters
  float heading = atan2(deltay,deltax);
  //Serial.println("Heading:"+String(heading*(180.0/3.1415926)));
  return heading;
}

float distanceBetweenTwoCoordinates(float lat1,float lon1,float lat2,float lon2){
  return(sqrt(pow(((lat2-lat1)*111319.4),2)+pow(((lon2-lon1)*111319.4),2)));
}

float compHeading(){
  sensors_event_t event; 
  mag.getEvent(&event);
  float heading = atan2(event.magnetic.x, event.magnetic.y);///Try switching x and y and see if it fixes your weird angle error
  float declinationAngle = 0.19;
  heading += declinationAngle;
  // Correct for when signs are reversed.
  if(heading < 0){
    heading += 2*PI;
  }
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI){
    heading -= 2*PI;
    }
return heading;
}