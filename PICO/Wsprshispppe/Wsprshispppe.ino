//
// Simple JT65/JT9/JT4/FT8/WSPR/FSQ beacon for Arduino, with the Etherkit
// Si5351A Breakout Board, by Jason Milldrum NT7S.
//
// Transmit an abritrary message of up to 13 valid characters
// (a Type 6 message) in JT65, JT9, JT4, a type 0.0 or type 0.5 FT8 message,
// a FSQ message, or a standard Type 1 message in WSPR.
//
// Connect a momentary push button to pin 12 to use as the
// transmit trigger. Get fancy by adding your own code to trigger
// off of the time from a GPS or your PC via virtual serial.
//
// Original code based on Feld Hell beacon for Arduino by Mark
// Vandewettering K6HX, adapted for the Si5351A by Robert
// Liesenfeld AK6L <ak6l@ak6l.org>.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>
#include <TinyGPS++.h>
#include "Wire.h"

// Mode defines
#define WSPR_TONE_SPACING       146          // ~1.46 Hz
#define WSPR_DELAY              683          // Delay value for WSPR
#define WSPR_DEFAULT_FREQ       14096900UL
//#define WSPR_DEFAULT_FREQ       14097200UL
#define DEFAULT_MODE            MODE_WSPR
// Hardware defines

// Enumerations
enum mode {MODE_WSPR};

// Class instantiation
Si5351 si5351;
JTEncode jtencode;

// Global variables For wspr
unsigned long freq;
char message[] = "KC1MOL FN42";
char call[] = "KC1MOL";
char loc[] = "FN42";
uint8_t dbm = 27;
uint8_t tx_buffer[255];
enum mode cur_mode = DEFAULT_MODE;
uint8_t symbol_count;
uint16_t tone_delay, tone_spacing;

//GPS stuffVV
TinyGPSPlus gps;

struct GpsData {
  float latitude;
  float longitude;
  float alt;
  int sats;
  int time;//seconds sence the day began
};
// Loop through the string, transmitting one character at a time.
void encode()
{
  uint8_t i;
  // Reset the tone to the base frequency and turn on the output
  si5351.output_enable(SI5351_CLK0, 1);

  for(i = 0; i < symbol_count; i++)
  {
      si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
      delay(tone_delay);
  }

  // Turn off the output
  si5351.output_enable(SI5351_CLK0, 0);
}

void set_tx_buffer()
{
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 255);

  // Set the proper frequency and timer CTC depending on mode
  jtencode.wspr_encode(call, loc, dbm, tx_buffer);
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  // Initialize the Si5351
  // Change the 2nd parameter in init if using a ref osc other
  // than 25 MHz
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);//POWER CAN BE CHANGED HERE

  // Set the mode to use
  cur_mode = MODE_WSPR;

  // Set the proper frequency, tone spacing, symbol count, and
  // tone delay depending on mode
  freq = WSPR_DEFAULT_FREQ;
  symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;

  // Set CLK0 output
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0); // Disable the clock initially

  // Encode the message in the transmit buffer
  // This is RAM intensive and should be done separately from other subroutines
  set_tx_buffer();
}

void loop()
{
  GpsData gpsd = GetGPSData(); 
  int TTS = (120-(gpsd.time%120))+2;
//debuggging
  Serial.println("************************************");
  Serial.print("Seconds sence start of day: ");
  Serial.println(gpsd.time);
  Serial.print("TTS!: ");
  Serial.println(TTS);
  jtencode.latlon_to_grid(gpsd.latitude,gpsd.longitude,loc);
  //jtencode.latlon_to_grid(34.7,-104.6,loc);
  Serial.print("MAIDENHEAD SQUARE: ");
  Serial.println(loc);
  delay(TTS*1000);
  set_tx_buffer();
  Serial.println("Starting now");
  encode();
  Serial.println("finished sending");
}

GpsData GetGPSData(){
  //bool newD = false;
  Serial1.read();
  for(unsigned long start = millis();millis()-start<1000;){
    if (Serial1.available() > 0 ) {//&& !newD
      if (gps.encode(Serial1.read())) {
        if (gps.location.isValid()) {
          Serial.print("Latitude: ");
          Serial.println(gps.location.lat(), 8);
          Serial.print("Longitude: ");
          Serial.println(gps.location.lng(), 8);
          //newD = true;
        } else {
          Serial.println("GPS data not valid");
        }
      }
    }
  }
  //Packaging the data to go back to the function
  GpsData cdata;
  cdata.latitude = gps.location.lat();
  cdata.longitude = gps.location.lng();
  cdata.alt = gps.altitude.meters();
  cdata.time = (gps.time.minute()*60)+gps.time.second();
  Serial.println(cdata.time);
  return cdata;
}