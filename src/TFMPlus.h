/* File Name: TFMPlus.h
 * Described: Arduino Library for testing the Benewake TFMini Plus Lidar sensor
 *            The TFMini Plus is a unique product, and the various
 *            TFMini Libraries are not compatible with the Plus.
 * Developer: Bud Ryerson
 * Inception: 31 JAN 2019 v0.2.0
 * Last work: 25 FEB 2019 v0.3.0
 *
 * Description: Arduino library for the Benewake TFMini Plus
 * time-of-flight Lidar ranging sensor.
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
 *
 * Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
 */

#ifndef TFMPLUS_H       // Guard to compile only once
#define TFMPLUS_H

#include <Arduino.h>    // Always remember to include this. It is important.

// Defines

// The output data frame size is 9 bytes
#define TFMPLUS_FRAME_SIZE           9

// Timeout Limits for various functions
#define MAX_READS_BEFORE_SERIAL     20
#define MAX_BYTES_BEFORE_HEADER     10
#define MAX_ATTEMPTS_TO_MEASURE     20

// System Status and Error Condition
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

// Commands: one byte payload, command number, command length  
#define    OBTAIN_FIRMWARE_VERSION    0x000104   // returns 3 byte firmware version
#define    TRIGGER_DETECTION          0x000404   // must set frame rate to zero first
                                                 // returns a 9 byte data frame
#define    SYSTEM_RESET               0x000204   // returns a 1 byte pass/fail (0/1)
#define    RESTORE_FACTORY_SETTINGS   0x000A04   //           "
#define    SAVE_SETTINGS              0x000B04   // must follow every command to modify parameters
                                                 // also returns a 1 byte pass/fail (0/1)
#define    SET_FRAME_RATE             0x000306   // returns echo of command
#define    SET_BAUD_RATE              0x000608   //           "
#define    STANDARD_FORMAT_CM         0x010505   //           "
#define    PIXHAWK_FORMAT             0x020505   //           "
#define    STANDARD_FORMAT_MM         0x060505   //           "
#define    ENABLE_OUTPUT              0x000705   //           "
#define    DISABLE_OUTPUT             0x010705   //           "

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

    uint8_t version[ 3];   // firmware version
    uint8_t status;        // system or error status

    bool begin( Stream *streamPtr);   // pass reference to the serial port
    void printStatus();
    uint8_t buildCommand( uint32_t cmnd, uint32_t param);
    uint8_t getData( uint16_t &dist, uint16_t &flux, uint16_t &temp);

  private:
    Stream* pStream;    // pointer to the device serial stream

    bool readData( uint8_t &rdByte);             // try MAX_READS_BEFORE_SERIAL times
    bool testSum( uint8_t *data, uint8_t len);   // test the checksum byte of the data frame
    uint8_t makeSum( uint8_t *data);             // make a checksum byte for command data
    void putCommand( uint8_t *data);
    uint8_t getResponse();
    void printData( uint8_t *data, uint8_t len); // for test - display hex characters

    struct dataStruct
    {
      union
      {
        uint8_t data[ 9];
        struct
        {
          uint8_t head[ 2];
          uint8_t dist[ 2];
          uint8_t flux[ 2];
          uint8_t temp[ 2];
          uint8_t chck;
        };
      };
	  };

};

#endif
