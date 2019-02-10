/* File Name: bene07.ino
 * Developer: Bud Ryerson
 * Inception: 29 JAN 2019
 * Last work: 09 FEB 2019
 * 
 * Description: Arduino sketch to test the Benewake TFMini Plus
 * time-of-flight Lidar ranging sensor using the TFMPlus library.
 *
 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate.
 *
 * NOTE: The TFMini Plus is a unique product and the various Arduino
 * Libraries for the TFMini are NOT compatible with the TFMini Plus.
 */

#include <TFMPlus.h>  // Add the TFMini Plus Library
TFMPlus tfmP;         // Create a TFMini Plus object

#include "printf.h"

//#include <SoftwareSerial.h>      // untested software serial library
//SoftwareSerial mySerial(10, 11); // RX, TX Pins

uint16_t tfDist;       // Distance measurement variable
uint16_t tfFlux;       // Luminous flux or intensity of return signal 
uint16_t tfTemp;       // Temperature in hundreths of a degrees Centigrade
uint16_t loopCount;    // Loop counter
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

  // Try five times to Get Firmware Version and pass back...
  // the version number (firmVer) as a three byte array.
  for( uint8_t fvi = 1; fvi < 6; ++fvi)
  {
	  if( tfmP.getFirmwareVersion( firmVer, sizeof( firmVer)))
	  {
		  printf( "Firmware Version: %1u.%1u.%1u\r\n", firmVer[ 0], firmVer[ 1], firmVer[ 2]);
      break;  // Break out of loop if successful.
	  }
	  else
	  {
	    printf( "Command to Get Firmware Version failed. "); // Display error message...
	    printf( "Attempt: %u\r\n", fvi);                     // and attempt number.
	  }
    delay(100);  // Wait to try again
  }
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
