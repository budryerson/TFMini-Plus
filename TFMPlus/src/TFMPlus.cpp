/* File Name: TFMiniPlus.cpp
 * Described: Arduino Library for testing the Benewake TFMini Plus Lidar sensor
 *            The TFMini Plus is a unique product, and the various
 *            TFMini Libraries are not compatible with the Plus.
 * Developer: Bud Ryerson
 * Inception: 31 JAN 2019
 * Last Work: 21 FEB 2019 -  incorporateed getMeasurment into getData
 *                           only try to get data once
 *
 * Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
 */

#include <TFMPlus.h>

// Constructor
  TFMPlus::TFMPlus(){}
  TFMPlus::~TFMPlus(){}

boolean TFMPlus::begin(Stream* streamPtr)
{
  pStream = streamPtr;        // Store reference to stream/serial object
  //setStandardOutputMode();  // Set standard output mode
  status = READY;             // Set status to READY
  return true;
}

// Clear data input buffer.
bool TFMPlus::flushReadBuffer()
{
  while( (*pStream).available()) (*pStream).read();
}

// Try to read one byte and return status
bool TFMPlus::readData( uint8_t &rdByte)
{
  for( uint8_t i = 0; i < MAX_READS_BEFORE_SERIAL; ++i)
  {
    if( (*pStream).available())       // If data is availabele...
    {
      rdByte = (*pStream).read();     // read one byte...
      return true;                    // and return "true".
    }
    delay(1);        // wait for a milliseond
  }

  status = SERIAL;   // If no serial data then set error...
  return false;      // and return "false".
}

// Sum up all but the last byte of data and
// compare low byte of Sum with last byte of data
bool TFMPlus::testSum( uint8_t *data, uint8_t len)
{
    uint16_t Sum = 0;   // Sum all but the last byte of data.
    for( uint8_t i = 0; i < len; i++) Sum += data[ i];
    // Return whether the Low byte of the Sum
    // matches the last byte of the data.
    return (uint8_t)Sum == data[ len];
}

// Create a checksum byte from the all the bytes of the command
uint8_t TFMPlus::makeSum( uint8_t *data, uint8_t len)
{
    uint16_t Sum = 0;   // Sum up 'len' bytes of data.
    for( uint8_t i = 0; i < len; i++) Sum += data[ i];
    return uint8_t(Sum);
}


// Print the Hex value of each bytye of data
void TFMPlus::printData( uint8_t *data, uint8_t len)
{
  printf(" Data:");
  for( uint8_t i = 0; i < len; i++) printf(" %02x", data[ i]);
}

void TFMPlus::printStatus()
{
  if( status == READY)          Serial.print( " - READY");
  else
  {
    Serial.print("  Error: ");
    if( status == SERIAL)        Serial.print( "SERIAL");
    else if( status == HEADER)   Serial.print( "HEADER");
    else if( status == CHECKSUM) Serial.print( "CHECKSUM");
    else if( status == TIMEOUT)  Serial.print( "TIMEOUT");
//    Serial.println();
  }
}

// Write command data to the device
void TFMPlus::putCommand( uint8_t *data, uint8_t len)
{
  flushReadBuffer();               // flush data input buffer
  (*pStream).flush();              // flush command output buffer
  for( uint8_t i = 0; i < len; i++) (*pStream).write( data[ i]);
}


bool TFMPlus::getCommandReply( uint8_t *data, uint8_t len)
{
  while( (*pStream).available() < len);           // wait for read buffer to fill
  for( uint8_t i = 0; i < len; i++) data[ i] = (*pStream).read();
  if( ( data[ 0] == 0x5A) && ( data[ 1] == len))  // If header detected...
  {
    if( testSum( data, len - 1))                  // and if checksum opkay...
    {
      return true;
    }
    else status = CHECKSUM;
  }
  else status = HEADER;
  return false;
}

// Set to "standard" output mode: 9-byte, centimeter range
bool TFMPlus::setStandardOutput()
{
  putCommand( stdOutMode, sizeof( stdOutMode));
  uint8_t outDat[ 5];          // Create a data array for the command response.
  if( getCommandReply( outDat, sizeof( outDat)))
  {
      return true;
  }
  return false;
}

// Perform a system reset on device
bool TFMPlus::setSystemReset()
{
  putCommand( systemReset, sizeof( systemReset));
  uint8_t srDat[ 5];          // Create a data array for the command response.
  if( getCommandReply( srDat, sizeof( srDat)))
  {
      return true;
  }
  return false;
}



bool TFMPlus::getFirmwareVersion( uint8_t *data)
{
  putCommand( getFirmVer, sizeof( getFirmVer));    // Send command to device.

  uint8_t verDat[ 7];          // Create a data array for the command response.
  memset( verDat, 0, sizeof( verDat));
  if( getCommandReply( verDat, sizeof( verDat)))
  {
      data[ 0] = verDat[5];
      data[ 1] = verDat[4];
      data[ 2] = verDat[3];
      return true;
  }
  return false;
}


// Read one frame of data from the device.
bool TFMPlus::getData( uint16_t& dist, uint16_t& flux, uint16_t& temp)
{
  int numCharsRead = 0;
  uint8_t frame[ TFMPLUS_FRAME_SIZE];
  memset( frame, 0, sizeof( frame));  // reset all bytes of frame to 0

  // Step 1:
  // Try to read serial bytes from device or
  // return 'serial' error if too many tries.
  // Continue reading until header is detected or
  // return 'header' error if too many characters are read.
  while( 1)
  {
    if( !readData( frame[ 1])) return false;
    if( (frame[ 0] == 0x59) && (frame[ 1] == 0x59))  // If header detected...
    {
      break;                 // break to finish reading the frame.
    }
    else                     // If not 0x59 in first two positions...
    {
      frame[ 0] = frame[ 1]; // move 2nd byte to 1st pos and continue reading.
      numCharsRead++;        // Advance character counter.
    }

    // If more than MAX BYTES are read without detecting a frame header,
    // then return 'header' error
    if( numCharsRead > MAX_BYTES_BEFORE_HEADER)
    {
      status = HEADER;
      return false;
    }
  }

  // Step 2:
  // Read more bytes to fill Plus data frame or
  // return 'serial' error if buffer runs out.
  for( int i = 2; i < TFMPLUS_FRAME_SIZE; i++)
  {
    if( !readData( frame[ i])) return false;
  }

  // Step 2A: Test the checksum byte
  // Last byte in the frame is an 8-bit checksum
  if( !testSum( frame, ( sizeof(frame) - 1)))
  {
    status = CHECKSUM;
    return false;
  }

  // Step 3: Interpret frame
  dist = frame[ 2] + ( frame[ 3] << 8);
  flux = frame[ 4] + ( frame[ 5] << 8);
  temp = frame[ 6] + ( frame[ 7] << 8);

  status = READY;
  return true;    // Return success
}