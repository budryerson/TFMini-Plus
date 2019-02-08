// Sketch by Bud Ryerson - 29JAN2019
// Testing the Benewake TFMini Plus time-of-flight Lidar ranging sensor
//
// NOTE: The TFMini Plus is a unique product. The various Arduino Libraries
// for the TFMini are NOT compatible with the TFMini Plus

/*
Command format:
Byte0  Byte1   Byte2   Byte3 to Len-2  Byte Len-1
0x5A   Length  Cmd ID  Payload if any   Checksum

Standard output format:
Byte0  Byte1  Byte2   Byte3   Byte4   Byte5   Byte6   Byte7   Byte8
0x59   0x59   Dist_L  Dist_H  Flux_L  Flux_H  Temp_L  Temp_H  CheckSum_
Data Frame Header character = Hex 0x59, Decimal 89, or "Y"
*/

#include <TFMPlus.h>  // Add the TFMini Plus Library
TFMPlus tfmP;         // Create a TFMini Plus object

#include "printf.h"

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(10, 11); // RX, TX

uint16_t tfDist;
uint16_t tfFlux;
uint16_t tfTemp;
uint16_t loopCount;
uint8_t firmVer[ 3];   // Three digit firmware version

void setup()
{
  Serial.begin( 115200);   // Intialize terminal serial port
  Serial1.begin( 115200);  // Initialize device serial port.
  tfmP.begin( &Serial1);   // Initialize device library object and...
                           // pass device serial port to the object.
  printf_begin();          // Initailize printf.

  tfDist = 0;              // Clear data variables.
  tfFlux = 0;
  tfTemp = 0;
  loopCount = 0;           // Reset loop counter.
  delay(500);              // And wait half a second.

  // Command device to Get Firmware Version and...
  // pass back the version number as a three byte array.
  if( tfmP.getFirmwareVersion( firmVer, sizeof( firmVer)))
	{
	  printf( "Firmware Version: %1u.%1u.%1u\r\n", firmVer[ 0], firmVer[ 1], firmVer[ 2]);
	}
 	else printf( "Command to Get Firmware Version has failed.\r\n");

  delay( 500);              // And wait another half a second.
}

void loop()
{
  printf( "Loop:%02u", loopCount);            // Display the loop count
  if( tfmP.getData( tfDist, tfFlux, tfTemp))  // Get data from device
  {
    printf( "  Dist:%04u", tfDist);           // Display the distance
    if( loopCount >= 20)                      // And every twenty loops...
    {
      printf( "  Flux:%05u", tfFlux);         // display the intensity...
      printf( "  Temp:%04u",  tfTemp);        // and the temperature...
      loopCount = 0;                          // and reset the loop counter.
    }
  }
  printf("\r\n");
  loopCount++;
  delay(10);
}
