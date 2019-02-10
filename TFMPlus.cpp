/*  File Name: TFMiniPlus.cpp
 *  Described: Arduino Library for testing the Benewake TFMini Plus Lidar sensor
 *             The TFMini Plus is a unique product, and the various
 *             TFMini Libraries are not compatible with the Plus.
 *  Developer: Bud Ryerson
 *  Inception: 31 JAN 2019
 *  Last work: 09 FEB 2019
 *
 *  Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
*/

#include <TFMPlus.h>

// Constructor
  TFMPlus::TFMPlus(){}
  TFMPlus::~TFMPlus(){}

boolean TFMPlus::begin(Stream* _streamPtr)
{
  streamPtr = _streamPtr;  // Store reference to stream/serial object
  distance  = 0;           // Clear reported values
  strength  = 0;
  temperature = 0;

  //setStandardOutputMode();  // Set standard output mode
  errorState = READY;              // Set state to READY
  return true;
}

// clear data input buffer
void TFMPlus::flushReadBuffer()
{
  while( streamPtr -> available()) streamPtr -> read();
}

// Try to read one byte from device and return status
bool TFMPlus::readData( uint8_t &rdByte)
{
  for( uint8_t rdTry = 0; rdTry < MAX_TRYS_TO_READ_DATA; ++rdTry) // Try to read 30 times
  {
    if( streamPtr -> available())       // If data is availabele...
    {
      rdByte = streamPtr -> read();     // read one byte...
      return true;                      // and return success.
    }
  }
  errorState = NO_SERIAL;  // If no data available after max tries, set error...
  return false;       // and return failure.
}

void TFMPlus::readFrame()
{
  uint8_t frameData[9];
  while( streamPtr -> available() < 9);   // wait until data is availabele
  for( uint8_t fdi = 0; fdi < 9; ++fdi)
  {
    frameData[ fdi] = streamPtr -> read();   // read one byte
   //   frameData[ fdi] = fdi;
  }
  printf(" Data:");
  for( uint8_t pdi = 0; pdi < 9; pdi++) printf(" %02x", frameData[ pdi]);
}

// Sum up all but the last byte of data and
// compare low byte of Sum with last byte of data
bool TFMPlus::checkSum( uint8_t *data, uint8_t len)
{
    uint16_t chkSum = 0;   // Add up all data bytes but the last.
    for( uint8_t csi = 0; csi < len; csi++) chkSum += data[ csi];

    if( (uint8_t)chkSum == data[ len]) // If the Low byte of chkSum matches...
    {                                  // the last byte of data frame...
      return true;                     // and return success.
    }
    else
    {
       errorState = CHECKSUM;               // Else set error state...
       return false;                   // and return failure.
    }
}

// Print the Hex value of each bytye of data
void TFMPlus::printData( uint8_t *data, uint8_t dataLen)
{
  printf(" Data:");
  for( uint8_t pdi = 0; pdi < dataLen; pdi++) printf(" %02x", data[ pdi]);
}


void TFMPlus::displayError()
{
  Serial.flush();
  Serial.print("TFMPlus Error: ");
  if( errorState == NO_SERIAL)           Serial.print( "NO SERIAL");
  else if( errorState == HEADER)         Serial.print( "HEADER");
  else if( errorState == CHECKSUM)       Serial.print( "CHECKSUM");
  else if( errorState == TOO_MANY_TRIES) Serial.print( "TOO MANY TRIES");
  else if( errorState == READY)          Serial.print( "READY");
  else if( errorState == OKAY)           Serial.print( "OKAY");
  Serial.println();

  errorState = READY;
}


// Public: The main function to measure distance.
bool TFMPlus::getData( uint16_t& dist, uint16_t& flux, uint16_t& temp)
{
  int numMeasurementAttempts = 0;
  // Loop until takeMeasurement returns a "0" or
  // break if too many measurements are atteempted
  while( 1)
  {
    if( takeMeasurement())
    {
      dist = distance;
      flux = strength;
      temp = temperature;
      errorState = OKAY;
      return true;
    }
    else ++numMeasurementAttempts;
    if( numMeasurementAttempts > MAX_MEASUREMENT_ATTEMPTS)
    {
      errorState = TOO_MANY_TRIES;
      break;
    }
  }

  if( errorState != OKAY)
  {
    dist = 0;
    flux = 0;
    temp = 0;
    Serial.flush();
    displayError();
    errorState = READY;
  }
  displayError();
  return false;
}


// Private: Handles the low-level bits of communicating with the TFMini, and detecting some communication errors.
bool TFMPlus::takeMeasurement()
{
  int numCharsRead = 0;
  uint8_t frame[ DATA_FRAME_SIZE];
  frame[ 0] = 0;

  // Step 1:
  // Read serial stream until the TFMPlus header sequence is detected,
  // too many bytes are read or a read timeout occurs.
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

    // Too many bytes read without a header,
    // set error and return failure.
    if( numCharsRead > MAX_BYTES_BEFORE_HEADER)
    {
      errorState = HEADER; // Set error state
      return false;        // Stop and return as failed
    }
  }

  // Step 2:
  // Read the rest of the data frame from the device...
  // and return an error if serial buffer runs out.
  for( int i = 2; i < DATA_FRAME_SIZE; i++)
  {
    if( !readData( frame[ i])) return false;
  }

  // Step 3:
  // Return error if checksum not correct.
  if( !checkSum( frame, ( sizeof(frame) - 1)))
  {
	 return false;
  }

  // Step 3:
  // Interpret data frame and return success.
  distance  =   frame[ 2] + ( frame[ 3] << 8);
  strength  =   frame[ 4] + ( frame[ 5] << 8);
  temperature = frame[ 6] + ( frame[ 7] << 8);

  errorState = OKAY;
  return true;    // Return success
}


// Send command data to the device
// Number of bytes is given in second byte of the command
void TFMPlus::sendCommand( uint8_t *data, uint8_t len)
{
  Serial1.flush();                 // Flush serial output buffer
  flushReadBuffer();               // Flush serial input buffer.

  for( uint8_t i = 0; i < data[ 1]; i++) streamPtr -> write( data[ i]);
}


bool TFMPlus::getCommandReply( uint8_t *data, uint8_t len)
{
//  uint8_t replyDat[ 8];       // Command response data array

  while( streamPtr -> available() < len);
  for( uint8_t i = 0; i < len; i++) data[ i] = streamPtr -> read();

  if( ( data[ 0] == 0x5A) && ( data[ 1] == len))  // If header detected...
  {
    if( checkSum( data, (len - 1)))
    {
      return true;
    }
    else errorState = CHECKSUM;
  }
  else errorState = HEADER;

  displayError();
  return false;

/*
  while( 1)
  {
    if( !readDat( replyDat[ 1])) return false;   // Try 30x to read a byte
    if( (replyDat[ 0] == 0x5A) && (replyDat[ 1] == len))  // If header detected...
    {
      break;                 // break to finish reading the frame.
    }
    else                     // If not 0x5A and command length in first two positions...
    {
      frame[ 0] = frame[ 1]; // move 2nd byte to 1st pos and continue reading.
      numCharsRead++;        // Advance the character counter.
    }
    // Too many bytes read without a header,
    // set error and return failure.
    if( numCharsRead > MAX_BYTES_BEFORE_HEADER)
    {
      errorState = HEADER; // Set error state
      return false;        // Stop and return as failed
    }
  }
*/

}


// Reset device to saved parameters.
bool TFMPlus::setSystemReset()
{
  uint8_t srData[ 5];
  sendCommand( systemResetCmd, sizeof( systemResetCmd));
  if( getCommandReply( srData, 5)) return false;
  printf( "System Reset: "); 
  if( srData[ 3] == 1)
  {
    printf( "failed");
    return false;
  }
  else
  {
    printf( "success");
    return true;
  }
}


// Reset device to default/factory parameters.
bool TFMPlus::setFactoryReset()
{
  uint8_t frData = 0;
  sendCommand( factoryResetCmd, sizeof( factoryResetCmd));
  if( !getCommandReply( frData, 1)) return false;
  return true;
}

// Set to "standard" output mode: 9-byte, centimeter range
bool TFMPlus::setStandardOutputMode()
{
  sendCommand( stdOutModeCmd, sizeof( stdOutModeCmd));
  delay(100);
  return true;
}

// Send Get Firmware Version command to device and, if successful,
// return the version number as a three byte array.
bool TFMPlus::getFirmwareVersion( uint8_t *data, uint8_t len)
{
  uint8_t verDat[ 7];              // Version response data array

  Serial1.flush();                 // Flush serial output buffer
  flushReadBuffer();               // Flush serial input buffer.

  for( uint8_t i = 0; i < 4; i++) streamPtr -> write( getVersionCmd[ i]);

  if( getCommandReply( verDat, 7))
  {
      data[ 0] = verDat[5];
      data[ 1] = verDat[4];
      data[ 2] = verDat[3];
      return true;
  }
  else
  {
    displayError();
    return false;
  }
}


