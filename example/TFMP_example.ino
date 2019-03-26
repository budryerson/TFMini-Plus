/* File Name: TFMP_example.ino
 * Developer: Bud Ryerson
 * Inception: 29 JAN 2019
 * Last work: 25 MAR 2019 - corrected a 'bbTfmp' call to 'tfmP' in the setup()
 *
 * Description: Arduino sketch to test the Benewake TFMini Plus
 * Lidar ranging sensor using the TFMPlus library v1.1.0.
 *
 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate.
 *
 * 'begin()' function passes a serial stream to library and
 *  returns TRUE/FALSE whether serial data is available.
 *  Function also sets a public one byte status code.
 *  Status codes are defined within the library.
 *
 * 'getData( dist, flux, temp)' passes back measuremnent data
 *  • dist = distance in centimeters,
 *  • flux = signal strength in arbitrary units, and
 *  • temp = an encoded number in degrees centigrade
 *  Function returns TRUE/FALSE whether completed without error.
 *  Error, if any, is saved as a one byte 'status' code.
 *
 * 'buildCommand( cmnd, param)' sends a 32bit command code (cmnd)
 *  and a 32bit parameter value (param). Returns TRUE/FALSE and
 *  sets a one byte status code.
 *  Commands are selected from the Library's list of defined commands.
 *  Parameter values can be entered directly (115200, 250, etc) but
 *  for safety, they should be chosen from the library's defined lists.
 *  An incorrect value can render the device uncomminicative.
 *
 */

#include <TFMPlus.h>  // Add the TFMini-Plus Library
TFMPlus tfmP;         // Create a TFMini-Plus object

#include "printf.h"

// - - - Software Serial Library was tested with the Arduino - - - 
// - - - ProMini and worked correctly at a 115200 baud rate. - - -
//#include <SoftwareSerial.h>      // alternative software serial library
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
  printf( "\r\n");  
  printf("TFMPlus Device Library v1.1.1 - 14MAR2019");  // say 'hello'
  printf( "\r\n");

  //  setup TFMini-Plus Lidar sensor
  Serial1.begin( 115200);  // Initialize device serial port.
  delay(20);               // Give port time to initalize

  // Initialize lidar device object and pass serial port.
  // Display message whether Lidar device is available
  printf("Lidar device serial port \"Serial1\" ");
  if( tfmP.begin( &Serial1)) printf( "initalized.");
    else printf( "not available.");
  printf( "\r\n");

  tfDist = 0;            // Clear device data variables.
  tfFlux = 0;
  tfTemp = 0;
  loopCount = 0;         // Reset loop counter.

  delay(500);            // And wait half a second.

// - - - -  Examples of using the 'buildCommand()' function  - - - -

  // Try five times to get the firmware version number
  // which is saved as 'tfmp.version', a three byte array.
  for( uint8_t fvi = 1; fvi < 6; ++fvi)
  {
    if( tfmP.buildCommand( OBTAIN_FIRMWARE_VERSION, 0))
	  {
      // If successful, display the version number...
		  printf( "Firmware Version: %1u.%1u.%1u\r\n",
		    tfmP.version[ 0], tfmP.version[ 1], tfmP.version[ 2]);
      break;      // and brreak out of loop.
	  }
	  else
	  {
      // If not successful, display the attempt number
      // and the error: HEADER, CHERCKSUM, SERIAL, tec.
	    printf( "Get Firmware Version failed. "); // Display error message...
	    printf( "Attempt: %u", fvi);              // attempt number..
      tfmP.printStatus();                       // and error status.
      printf("\r\n");
	  }
    delay(100);  // Wait to try again
  }

/*      if( bbTfmp.buildCommand( RESTORE_FACTORY_SETTINGS, 0) == 5)
        {
          printf( "Lidar device system reset pass.");
        }
        else printf( "Lidar device system reset fail.");
        printf( "\r\n");
*/

/*      if( bbTfmp.buildCommand( SET_FRAME_RATE, FRAME_20))
        {
          printf( "Lidar device frame rate set to 20Hz.");
        }
        else printf( "Lidar device frame rate command failed.");
        printf( " Status = %2u", bbTfmp.status);
        printf( "\r\n");
*/

  delay( 500);   // And wait another half a second to begin loop
}

void loop()
{
  // Excample of using the 'getData' function to pass back data. Show the
  // loop counmt and the range (distance), and every twenty loops, show
  // all the data.  Not the formula for decoding temperature (C° = (t/8) - 256).
  printf( "Loop:%02u", loopCount);                     // Display the loop count.
  if( tfmP.getData( tfDist, tfFlux, tfTemp)) // Get data from the device.
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
  loopCount++;     // AAdvance the loop counter
  delay(10);       // Delay to match the 100Hz data frame rate.
}
