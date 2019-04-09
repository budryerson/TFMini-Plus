/* File Name: TFMPlus.h
 * Described: Arduino Library for the Benewake TFMini Plus Lidar sensor
 *            The TFMini Plus is a unique product, and the various
 *            TFMini Libraries are not compatible with the Plus.
 * Developer: Bud Ryerson
 * Inception: v0.2.0 - 31 JAN 2019 
 *            v1.0.0 - 25 FEB 2019 - Initial release
 * Last work:
 * v1.0.1 - 09MAR19 - 'build()' function always returned TRUE.
 *    Corrected to return FALSE if serial data is not available.
 *    And other minor corrections to textual descriptionms.
 * v1.1.0 - 13MAR19 - To simplify, all interface functions now
 *    return boolean.  Status code is still set and can be read.
 *    'testSum()' is deleted and 'makeSum()' is used instead.
 *    Example code is updated.
 * v1.1.1 - 14MAR19 - Two commands: RESTORE_FACTORY_SETTINGS
 *    and SAVE_SETTINGS were not defined correctly.
 * v1.2.1 - 02APR19 - Rewrote 'getData()' function to make it faster
 *    and more robust when serial read skips a byte or fails entirely.
 * v1.3.1 - 08APR19 - Redefined commands to include response length.
      **********************     IMPORTANT    ************************
      ****  Changed name of 'buildCommand()' to 'sendCommand()'.  ****
      ****************************************************************
 *
 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate. The device will begin returning
 * measurement data immediately on power up.
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
 * 'sendCommand( cmnd, param)' sends a 32bit command code (cmnd)
 *  and a 32bit parameter value (param). Returns TRUE/FALSE and
 *  sets a one byte status code.
 *  Commands are selected from the library's list of defined commands.
 *  Parameter values can be entered directly (115200, 250, etc) but
 *  for safety, they should be chosen from the Library's defined lists.
 *  An incorrect value can render the device uncomminicative.
 *
 */

#ifndef TFMPLUS_H       // Guard to compile only once
#define TFMPLUS_H

#include <Arduino.h>    // Always include this. It is important.

// Defines
#define TFMP_FRAME_SIZE         9   // The output data frame size is 9 bytes
#define TFMP_REPLY_SIZE         8   // The longest co0mmand reply is 8 bytes

// Timeout Limits for various functions
#define TFMP_MAX_READS           20   // readData() sets SERIAL error
#define MAX_BYTES_BEFORE_HEADER  20   // getData() sets HEADER error
#define MAX_ATTEMPTS_TO_MEASURE  20

// System Error Status
#define READY                 0
#define SERIAL                1
#define HEADER                2
#define CHECKSUM              3
#define TIMEOUT               4
#define PASS                  5
#define FAIL                  6
#define MEASUREMENT_OK       10

/* - - - - - - - - -  TFMini Plus  - - - - - - - - -
  Standard output data frame format:
  Byte0  Byte1  Byte2   Byte3   Byte4   Byte5   Byte6   Byte7   Byte8
  0x59   0x59   Dist_L  Dist_H  Flux_L  Flux_H  Temp_L  Temp_H  CheckSum_
  Data Frame Header character: Hex 0x59, Decimal 89, or "Y"

  Command format:
  Byte0  Byte1   Byte2   Byte3 to Len-2  Byte Len-1
  0x5A   Length  Cmd ID  Payload if any   Checksum
 - - - - - - - - - - - - - - - - - - - - - - - - - */

// Command definition format:
// 0x     00       00       00       00
//     one byte  command  command   reply
//     payload   number   length    length
#define    OBTAIN_FIRMWARE_VERSION    0x00010407   // returns 3 byte firmware version
#define    TRIGGER_DETECTION          0x00040400   // must set frame rate to zero first
                                                   // returns a 9 byte data frame
#define    SYSTEM_RESET               0x00020405   // returns a 1 byte pass/fail (0/1)
#define    RESTORE_FACTORY_SETTINGS   0x00100405   //           "
#define    SAVE_SETTINGS              0x00110405   // must follow every command to modify parameters
                                                   // also returns a 1 byte pass/fail (0/1)
#define    SET_FRAME_RATE             0x00030606   // returns echo of command
#define    SET_BAUD_RATE              0x00060808   //           "
#define    STANDARD_FORMAT_CM         0x01050505   //           "
#define    PIXHAWK_FORMAT             0x02050505   //           "
#define    STANDARD_FORMAT_MM         0x06050505   //           "
#define    ENABLE_OUTPUT              0x00070505   //           "
#define    DISABLE_OUTPUT             0x01070505   //           "

// Command Parameters
#define    BAUD_9600          0x002580   // expression of baud rate
#define    BAUD 14400         0x003840   // in hexidecimal
#define    BAUD_19200         0x004B00
#define    BAUD_56000         0x00DAC0
#define    BAUD_115200        0x01C200
#define    BAUD_460800        0x070800
#define    BAUD_921600        0x0E1000

#define    FRAME_0            0x0000  // expression of frame rate
#define    FRAME_1            0x0001  // as hex number
#define    FRAME_2            0x0002
#define    FRAME_5            0x0003
#define    FRAME_10           0x000A
#define    FRAME_20           0x0014
#define    FRAME_25           0x0019
#define    FRAME_50           0x0032
#define    FRAME_100          0x0064
#define    FRAME_125          0x007D
#define    FRAME_200          0x00C8
#define    FRAME_250          0x00FA
#define    FRAME_500          0x01F4
#define    FRAME_1000         0x03E8

// Object Class Definitions
class TFMPlus
{
  public:
    TFMPlus();
    ~TFMPlus();

    uint8_t version[ 3];   // save firmware version
    uint8_t status;        // save system error status

    // Return T/F whether serial data available, set error status if not.
    bool begin( Stream *streamPtr);
    // Read device data and pass back three values
    bool getData( uint16_t &dist, uint16_t &flux, uint16_t &temp);
    // Build and send a command, and check response
    bool sendCommand( uint32_t cmnd, uint32_t param);

    // for testing purposes, print frame or reply data and status
    void printStatus( bool isFrameData);

  private:
    Stream* pStream;      // pointer to the device serial stream

    // The data buffers are one byte longer than necessary
    // becasue we read one byte into the last position, then shift
    // the whole mess left by one position after each read.
    uint8_t frame[ TFMP_FRAME_SIZE + 1];
    uint8_t reply[ TFMP_REPLY_SIZE + 1];
    
    //uint8_t makeSum( uint8_t *data);   // make a checksum byte from *data

    // for testing - print frame or reply data as hex characters
    void printFrame();
    void printReply();
};

#endif
