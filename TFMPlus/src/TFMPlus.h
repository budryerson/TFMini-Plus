/* File Name: TFMPlus.h
 * Described: Arduino Library for testing the Benewake TFMini Plus Lidar sensor
 *            The TFMini Plus is a unique product, and the various
 *            TFMini Libraries are not compatible with the Plus.
 * Developer: Bud Ryerson
 * Inception: 31 JAN 2019
 * Last work: 21 FEB 2019 - added future struct code
 *
 * Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
 */

#ifndef TFMPLUS_H       // Guard to compile only once
#define TFMPLUS_H

#include <Arduino.h>    // Always remember to include this. It is important.

// Defines
#define TFMPLUS_BAUDRATE   115200
#define TFMPLUS_DEBUGMODE  1

// The frame size is nominally 9 characters, but don't include
// the first two 0x59's that mark the start of the frame
#define TFMPLUS_FRAME_SIZE           9

// Timeout Limits
#define MAX_READS_BEFORE_SERIAL     20
#define MAX_BYTES_BEFORE_HEADER     10
#define MAX_ATTEMPTS_TO_MEASURE     20

// System Status
#define READY                 0
#define SERIAL                1
#define HEADER                2
#define CHECKSUM              3
#define TIMEOUT               4
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

// Object Class Definitions
class TFMPlus
{
  public:
    TFMPlus();
    ~TFMPlus();

    bool begin( Stream* streamPtr);
    bool getData( uint16_t& dist, uint16_t& flux, uint16_t& temp);
    void printStatus();

    // Command function calls
    bool setSystemReset();
    bool getFirmwareVersion( uint8_t *data);  // send pointer to return reply data
    bool setStandardOutput();
    // Commands function not yet implemented
    //    bool factoryReset();
    //    uint16_t setFrameRate( uint16_t frameRate);
    //    uint32_t setBaudRate( uint32_t baudRate);
    //    void enableDataOutput();
    //    void disableDataOutput();
    //    void externalTrigger();  // set frame rate to zero first
    //    bool saveSettings();     // must follow every command that modifies parameters

  private:
    Stream* pStream;
    int status;

    bool readData( uint8_t &rdByte);
    bool flushReadBuffer();
    bool testSum( uint8_t *data, uint8_t len);     // test the checksum byte of the data frame
    uint8_t makeSum( uint8_t *data, uint8_t len);  // make a checksum byte for the command
    void putCommand( uint8_t *data, uint8_t len);
    bool getCommandReply( uint8_t *data, uint8_t len);
    void printData( uint8_t *data, uint8_t len);

    // List of Commnad data by Command Number
    // 1) Obtain Firmware Version
    uint8_t getFirmVer[ 4] =  { 0x5A, 0x04, 0x01, 0x5F};
    // 2) Reset to System Settings
    uint8_t systemReset[ 4] = { 0x5A, 0x04, 0x02, 0x60};
    // 3) Set Frame Rate to 0
    uint8_t frmRate000[ 6] =  { 0x5A, 0x06, 0x03, 0x00, 0x00, 0x63};
    // 3 Command to Set Frame Rate to 100
    uint8_t frmRate100[ 6] =  { 0x5A, 0x06, 0x03, 0x64, 0x00, 0xC7};
    // 5 Command to Set Standard Output Mode
    uint8_t stdOutMode[ 5] =  { 0x5A, 0x05, 0x05, 0x01, 0x65};
    // 7 Command to Disable/Enable Data Output
    uint8_t disableOutput[ 5] = { 0x5A, 0x05, 0x07, 0x00, 0x66};
    uint8_t  enableOutput[ 5] = { 0x5A, 0x05, 0x07, 0x01, 0x66};
    // 10 Command to Reset device to Factory Settings
    uint8_t factoryReset[ 4] = { 0x5A, 0x04, 0x10, 0x6E};
    // 11 Command to Save Parameters - Must follow every command to modify parameters!
    uint8_t saveParam[ 4] =   { 0x5A, 0x04, 0x11, 0x6F};

};

#endif
