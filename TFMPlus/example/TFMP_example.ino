/* File Name: bene07.ino
 * Developer: Bud Ryerson
 * Inception: 29 JAN 2019
 * Last work: 25 FEB 2019
 *
 * Description: Arduino sketch to test the Benewake TFMini Plus
 * time-of-flight Lidar ranging sensor using the TFMPlus library.
 *
 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate. The device will begin returning
 * measurement data right away:
 * Distance in centimeters,
 * signal strength in arbitrary units,
 * and an encoded number for temperature in degrees centigrade.
 *
 * Use the 'buildCommand' to send commands and return a status code.
 * Commands are selected from the library's list of defined commands.
 * Parameters can be entered directly (115200, 250, etc) but for
 * safety, they should be chosen from the library's defined lists.
 *
 * NOTE: The TFMini Plus is a unique product and the various Arduino
 * Libraries for the TFMini are NOT compatible with the TFMini Plus.
 */

#include <TFMPlus.h>  // Add the TFMini Plus Library
TFMPlus tfmP;         // Create a TFMini Plus object

#include "printf.h"

//#include <SoftwareSerial.h>      // untested software serial library
//SoftwareSerial mySerial(10, 11); // RX, TX Pins

uint16_t tfDist;       // Distance measurement in centimeters (default)
uint16_t tfFlux;       // Luminous flux or intensity of return signal
uint16_t tfTemp;       // Temperature in degrees Centigrade (coded)
uint16_t loopCount;    // Loop counter (1-20)

void setup()
{
  Serial.begin( 115200);   // Intialize terminal serial port
  delay(20);               // Give port time to initalize
  printf_begin();          // Initialize printf.
  printf("\r\nTFMPlus Device Library - 25FEB2019\r\n");  // say 'hello'

  Serial1.begin( 115200);  // Initialize device serial port.
  delay(20);               // Give port time to initalize
  while( !Serial1.available())
  {
      printf("Device serial port \"Serial1\" not available.\r\n");
      printf("Will wait 3 seconds and try again .\r\n");
      delay( 3000);      // Delay 3 seconds
  }
  tfmP.begin( &Serial1); // Initialize device library object and...
                         // pass device serial port to the object.

  tfDist = 0;            // Clear device data variables.
  tfFlux = 0;
  tfTemp = 0;
  loopCount = 0;         // Reset loop counter.

  delay(500);            // And wait half a second.

  // Try five times to Get the firmware version number
  // as 'tfmp.version', a three byte array.
  for( uint8_t fvi = 1; fvi < 6; ++fvi)
  {
    tfmP.status = tfmP.buildCommand( OBTAIN_FIRMWARE_VERSION, 0);
	  if( tfmP.status == READY)
	  {
		  printf( "Firmware Version: %1u.%1u.%1u\r\n",
		    tfmP.version[ 0], tfmP.version[ 1], tfmP.version[ 2]);
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
  tfmP.status = tfmP.getData( tfDist, tfFlux, tfTemp); // Get data from device.
  if( tfmP.status == READY)
  {
    printf( " Dist:%04u", tfDist);          // Display the distance.
    if( loopCount >= 20)                    // And every twenty loops...
    {
      printf( " Flux:%05u", tfFlux);        // display signal strength...
      uint8_t tfTempC = uint8_t(( tfTemp / 8) - 256); // decode temperature...
      printf( " Temp:% 2uC", tfTempC);      // display temp as °C...
      uint8_t tfTempF = uint8_t( tfTempC * 9.0 / 5.0) + 32.0;
      printf( "% 3uF", tfTempF);            // and again as °F.
      loopCount = 0;                        // Reset loop counter.
    }
  }
  else tfmP.printStatus();
  printf("\r\n");  // Send CR/LF to terminal
  loopCount++;
  delay(10);       // Delay to match the 100Hz data frame rate.
}
