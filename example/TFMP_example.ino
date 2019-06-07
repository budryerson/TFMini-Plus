/* File Name: TFMP_example.ino
 * Developer: Bud Ryerson
 * Inception: 29 JAN 2019
 * Last work: 07 JUN 2019
 *
 * Description: Arduino sketch to test the Benewake TFMini Plus
 * time-of-flight Lidar ranging sensor using the TFMPlus library.
 *
 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate. The device will begin returning
 * measurement data right away:
 *   Distance in centimeters,
 *   Signal strength in arbitrary units,
 *   and an encoded number for Temperature in degrees centigrade.

   **********************     IMPORTANT    ************************
   ****  Changed name of 'buildCommand()' to 'sendCommand()'.  ****
   ****************************************************************

 * Use the 'sendCommand' to send commands and return a status code.
 * Commands are selected from the library's list of defined commands.
 * Parameters can be entered directly (115200, 250, etc) but for
 * safety, they should be chosen from the library's defined lists.
 *
 */

#include <TFMPlus.h>  // Include TFMini Plus Library v1.3.4
TFMPlus tfmP;         // Create a TFMini Plus object

#include "printf.h"   // May not work corrrectly with Intel devices

//#include <SoftwareSerial.h>       // alternative software serial library
//SoftwareSerial mySerial(10, 11);  // Choose the correct RX, TX Pins

uint16_t tfDist;       // Distance measurement in centimeters (default)
uint16_t tfFlux;       // Luminous flux or intensity of return signal
uint16_t tfTemp;       // Temperature in degrees Centigrade (coded)
uint16_t loopCount;    // Loop counter (1-20)

/*  - - - - -  A few useful Lidar commands    - - - - - */

// Try three times to get the firmware version number
// which is saved as 'tfmp.version', a three byte array.
void firmwareVersion()
{
    for( uint8_t fvi = 1; fvi < 4; ++fvi)
    {
        if( tfmP.sendCommand( OBTAIN_FIRMWARE_VERSION, 0))
        {
            // If successful, display the version number...
            printf( "Lidar firmware version: %1u.%1u.%1u\r\n",
                tfmP.version[ 0], tfmP.version[ 1], tfmP.version[ 2]);
            break;      // and brreak out of loop.
        }
        else
        {
            // If not successful, display the attempt number
            // and the error: HEADER, CHERCKSUM, SERIAL, tec.
            printf( "Get firmware version failed. "); // Display error message...
            printf( "Attempt: %u", fvi);              // attempt number..
            tfmP.printStatus( false);                 // and error status.
            printf("\r\n");
        }
        delay(100);  // Wait to try again
    }
}

void factoryReset()
{
    printf( "Lidar factory reset ");
    if( tfmP.sendCommand( RESTORE_FACTORY_SETTINGS, 0))
    {
        printf( "passed.\r\n");
    }
    else
    {
        printf( "failed.");
        tfmP.printStatus( false);
    }
}

void frameRate( uint16_t rate)
{
    printf( "Lidar frame rate ");
    if( tfmP.sendCommand( SET_FRAME_RATE, rate))
    {
        printf( "set to %2uHz.\r\n", rate);
    }
    else
    {
        printf( "command failed.");
        tfmP.printStatus( false);
    }
}

void saveSettings()
{
    printf( "Lidar device settings ");
    if( tfmP.sendCommand( SAVE_SETTINGS, 0))
    {
        printf( "saved.\r\n");
    }
    else
    {
        printf( "not saved.");
        tfmP.printStatus( false);
    }
}

/*  - - -   End of useful Lidar commands   - - - -   */

void setup()
{
    Serial.begin( 115200);   // Intialize terminal serial port
    delay(20);               // Give port time to initalize
    printf_begin();          // Initialize printf.
    printf("\r\nTFMPlus Library Example - 07JUN2019\r\n");  // say 'hello'

    Serial2.begin( 115200);  // Initialize TFMPLus device serial port.
    delay(20);               // Give port time to initalize
    tfmP.begin( &Serial2);   // Initialize device library object and...
                             // pass device serial port to the object.

    // Send commands to device during setup.
    firmwareVersion();
    frameRate(20);
    //saveSettings();
    //factoryReset();

    // Initialize the variables for this example
    loopCount = 0;         // Reset loop counter.
    tfDist = 0;            // Clear device data variables.
    tfFlux = 0;
    tfTemp = 0;

    delay(500);            // And wait for half a second.
}

// Use the 'getData' function to pass back device data.
void loop()
{
  // 1. On each loop, try up to five tries to get a good data frame.
  for( uint8_t fvi = 1; fvi < 6; ++fvi)
  {
      printf( "Loop:%02u", loopCount);           // Display the loop count.
      if( tfmP.getData( tfDist, tfFlux, tfTemp)) // Get data from the device.
      {
        printf( " Dist:%04u", tfDist);           // Display the distance.
        break;                                   // Escape this sub-loop
      }
      else                        // If the command fails...
      {
        tfmP.printStatus( true);  // display the error.
      }
  }

  // 2. Every twenty loops, show the two additional values.
  if( loopCount >= 20)
  {
    // Display signal strength in arbitrary units.
    printf( " Flux:%05u", tfFlux);

    // Decode temperature data and display as degrees Centigrade.
    uint8_t tfTempC = uint8_t(( tfTemp / 8) - 256);
    printf( " Temp:% 2uC", tfTempC);

    // Convert temperature and display as degrees Farenheit.
    uint8_t tfTempF = uint8_t( tfTempC * 9.0 / 5.0) + 32.0;
    printf( "% 3uF", tfTempF);

    loopCount = 0;                   // Reset loop counter.
  }

  // 3. Finish up and advance the loop counter
  printf("\r\n");  // Send CR/LF to terminal
  loopCount++;
  delay(10);       // Delay to match the 100Hz data frame rate.
}
