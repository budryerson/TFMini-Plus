/* File Name: bene07.ino
 * Developer: Bud Ryerson
 * Inception: 29 JAN 2019
 * Last work: 21 FEB 2019
 *
 * Added temperature conversion
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


void setup()
{
  Serial.begin( 115200);   // Intialize terminal serial port
  delay(20);               // Give port time to initalize
  printf_begin();          // Initialize printf.
  printf("\r\nTFMPlus Device Library - 21FEB2019\r\n");

  Serial1.begin( 115200);   // Initialize device serial port.
  delay(20);                // Give port time to initalize
  while( !Serial1.available())
  {
      printf("Device serial port \"Serial1\" not available.\r\n");
      printf("Wait 3 seconds, and check again .\r\n");
      delay( 3000);      // Delay 3 seconds
  }
  tfmP.begin( &Serial1); // Initialize device library object and...
                         // pass device serial port to the object.
  tfDist = 0;            // Clear device data variables.
  tfFlux = 0;
  tfTemp = 0;
  loopCount = 0;         // Reset loop counter.
  delay(500);            // And wait half a second.

  // Try five times to Get Firmware Version, passing back...
  // the version number (firmVer) as a three byte array.
  uint8_t frmVer[ 3];   // Three byte firmware version
  for( uint8_t fvi = 1; fvi < 6; ++fvi)
  {
	  if( tfmP.getFirmwareVersion( frmVer))
	  {
		  printf( "Firmware Version: %1u.%1u.%1u\r\n", frmVer[ 0], frmVer[ 1], frmVer[ 2]);
      break;  // Break out of loop if successful.
	  }
	  else
	  {
	    printf( "Get Firmware Version failed. "); // Display error message...
	    printf( "Attempt: %u", fvi);              // attempt number..
      tfmP.printStatus();                       // and error status.
      printf("\r\n");
	  }
    delay(100);  // Wait to try again
  }

  delay( 500);   // And wait another half a second to begin loop
}

void loop()
{
  printf( "Loop:%02u", loopCount);           // Display the loop count.
  if( tfmP.getData( tfDist, tfFlux, tfTemp)) // Get data from device.
  {
    printf( " Dist:%04u", tfDist);          // Display the distance.
    if( loopCount >= 20)                     // And every twenty loops...
    {
      printf( " Flux:%05u", tfFlux);        // display signal strength...
      uint8_t tfTempC = uint8_t(( tfTemp / 8) - 256);
      printf( " Temp:% 2uC", tfTempC);      // display temperature as °C...
      uint8_t tfTempF = uint8_t( tfTempC * 9.0 / 5.0) + 32.0;
      printf( "% 3uF", tfTempF);            // and again as °F.
      loopCount = 0;                        // Reset the loop counter.
    }
  }
  else tfmP.printStatus();
  printf("\r\n");  // Send CR/LF to terminal
  loopCount++;
  delay(10);       // Delay to match the 100Hz data frame rate.
}
