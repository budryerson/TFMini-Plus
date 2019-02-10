/*  File Name: TFMPlus.h
 *  Described: Arduino Library for testing the Benewake TFMini Plus Lidar sensor
 *             The TFMini Plus is a unique product, and the various
 *             TFMini Libraries are not compatible with the Plus.
 *  Developer: Bud Ryerson
 *  Inception: 31 JAN 2019
 *  Last work: 09 FEB 2019
 *
 *  Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
 */

#ifndef TFMPLUS_H       // Guard to compile only one time
#define TFMPLUS_H

#include <Arduino.h>    // Always remember this. It is very important.

// Timeouts
#define MAX_TRYS_TO_READ_DATA       30
#define MAX_BYTES_BEFORE_HEADER     30
#define MAX_MEASUREMENT_ATTEMPTS    20

// States
#define READY                 0
#define HEADER                1
#define CHECKSUM              2
#define TOO_MANY_TRIES        3
#define NO_SERIAL             4
#define OKAY                 10

/* - - - - - - - - -  Data Format  - - - - - - - - -
  Standard output data frame format is nine bytes,
  including twoe header bytes and one checksum byte.

  Byte0  Byte1  Byte2   Byte3   Byte4   Byte5   Byte6   Byte7   Byte8
  0x59   0x59   Dist_L  Dist_H  Flux_L  Flux_H  Temp_L  Temp_H  CheckSum_

  Data Frame Header character is Hex 0x59, Decimal 89, or "Y"
  - - - - - - - - - - - - - - - - - - - - - - - - - */
#define DATA_FRAME_SIZE       9

/* - - - - - - - -  Command Format  - - - - - - - - -
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

    uint16_t dist;
    uint16_t flux;
    uint16_t temp;

    bool begin(Stream* _streamPtr);
    bool getData( uint16_t& dist, uint16_t& flux, uint16_t& temp);
    // Send command and pass back version number or return false.
    bool getFirmwareVersion( uint8_t *data, uint8_t len);
    // Send command to reset device to store parameters.
    bool setSystemReset();
    // Send command to reset device to default/factory settings.
    bool setFactoryReset();
    bool setStandardOutputMode();

  private:
    Stream* streamPtr;

    uint16_t distance;
    uint16_t strength;
    uint16_t temperature;
    int errorState;

    void flushReadBuffer();
    bool readData( uint8_t &rdByte);
    void readFrame();
    bool checkSum( uint8_t *data, uint8_t dataLen);
    bool takeMeasurement();
    void sendCommand( uint8_t *data, uint8_t len);
    bool getCommandReply( uint8_t *data, uint8_t len);
    void printData( uint8_t *data, uint8_t dataLen);
    void displayError();

    // 1 Command to obtain device Firmware Version
    uint8_t getVersionCmd[ 4]  = { 0x5A, 0x04, 0x01, 0x5F};
    // 2 Command to reset device to System Settings
    uint8_t systemResetCmd[ 4] = { 0x5A, 0x04, 0x02, 0x60};
    // 3 Command to set device Frame Rate to Zero
    uint8_t frmRate000Cmd[ 6]  = { 0x5A, 0x06, 0x03, 0x00, 0x00, 0x63};
    // 3 Command to set device Frame Rate to 100
    uint8_t frmRate100Cmd[ 6]  = { 0x5A, 0x06, 0x03, 0x64, 0x00, 0xC7};
    // 5 Command to set device to Standard Output Mode
    uint8_t stdOutModeCmd[ 5]  = { 0x5A, 0x05, 0x05, 0x01, 0x65};
    // 7 Command to Disable/Enable device Data Output
    uint8_t disableOutputCmd[ 5] = { 0x5A, 0x05, 0x07, 0x00, 0x66};
    uint8_t  enableOutputCmd[ 5] = { 0x5A, 0x05, 0x07, 0x01, 0x66};
    // 10 Command to Reset device to Factory Settings
    uint8_t factoryResetCmd[ 4]  = { 0x5A, 0x04, 0x10, 0x6E};
    // 11 Command to Save Parameters - Must follow every command to modify parameters!
    uint8_t saveParamCmd[ 4]  = { 0x5A, 0x04, 0x11, 0x6F};

};

#endif
