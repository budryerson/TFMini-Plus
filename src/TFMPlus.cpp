/* File Name: TFMiniPlus.cpp
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

#include <TFMPlus.h>

// Constructor
  TFMPlus::TFMPlus(){}
  TFMPlus::~TFMPlus(){}

bool TFMPlus::begin(Stream *streamPtr)
{
  pStream = streamPtr;        // Store reference to stream/serial object
  //setStandardOutputMode();  // Set standard output mode
  status = READY;             // Set status to READY
  return true;
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
    Serial.print(" Error: ");
    if( status == SERIAL)        Serial.print( "SERIAL");
    else if( status == HEADER)   Serial.print( "HEADER");
    else if( status == CHECKSUM) Serial.print( "CHECKSUM");
    else if( status == TIMEOUT)  Serial.print( "TIMEOUT");
    else if( status == PASS)  Serial.print( "PASS");
    else if( status == FAIL)  Serial.print( "FAIL");
    else Serial.print( "OTHER");
//    Serial.println();
  }
}

// Write command data to the device
void TFMPlus::putCommand( uint8_t *data)
{
  while( (*pStream).available()) (*pStream).read();  // flush input buffer
  (*pStream).flush();                                // flush output buffer
  for( uint8_t i = 0; i < data[ 1]; i++) (*pStream).write( data[ i]);
}

uint8_t TFMPlus::getResponse()
{
  uint8_t reply[ 8];                          // Initialize a reply array
  memset( reply, 0, sizeof( reply));          // Set all bytes of array to zero  

  if( !readData( reply[ 0])) return SERIAL;   // Read the header byte.
  if( reply[ 0] != 0x5A) return HEADER;       // If header not detected...
  if( !readData( reply[ 1])) return SERIAL;   // Read the length byte.
  for( uint8_t i = 2; i < reply[ 1]; i++)
  {
    if( !readData( reply[ i])) return SERIAL; // Read the rest of the reply.
  }
  if( !testSum( reply, ( reply[ 1] - 1)))     // If last byte not correct...
  {
      return CHECKSUM;
  }
  // If command number is 2, 10 or 11...
  if( reply[ 2] == 2 || reply[ 2] == 10 || reply[ 2] == 11)
  {
    if( reply[ 3] == 0)               // and fourth byte is zero...
    {
      return PASS;                    // then return 'pass'...
    }
    else return FAIL;                 // or else 'fail'.
  }
  else if( reply[ 2] == 1)            // If get firmware version (1)...
  {
      version[ 0] = reply[5];
      version[ 1] = reply[4];
      version[ 2] = reply[3];     
  }
  return READY;
}

// Read one frame of data from the device.
uint8_t TFMPlus::getData( uint16_t &dist, uint16_t &flux, uint16_t &temp)
{
  uint8_t frame[ TFMPLUS_FRAME_SIZE];  // initialize data frame
  memset( frame, 0, sizeof( frame));   // reset all bytes of frame to 0

  // Step 1:
  // Read serial bytes from device or return 'serial' error if too many tries.
  // Read until header is detected or return 'header' error if too many chars are read.
  int numCharsRead = 0;      // initalize char counter
  while( 1)
  {
    if( !readData( frame[ 1])) return SERIAL;
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
    if( numCharsRead > MAX_BYTES_BEFORE_HEADER) return HEADER;
  }

  // Step 2:
  // Read more bytes to fill Plus data frame or
  // return 'serial' error if buffer runs out.
  for( int i = 2; i < TFMPLUS_FRAME_SIZE; i++)
  {
    if( !readData( frame[ i])) return SERIAL;
  }

  // Step 2A: Test the checksum byte
  // Last byte in the frame is an 8-bit checksum
  if( !testSum( frame, ( sizeof(frame) - 1))) return CHECKSUM;

  // Step 3: Interpret frame
  dist = frame[ 2] + ( frame[ 3] << 8);
  flux = frame[ 4] + ( frame[ 5] << 8);
  temp = frame[ 6] + ( frame[ 7] << 8);

  return READY;
}

// Return a checksum byte creted from the contents of 'data'.
// Length of 'data' is given in the second byte: 'data[1]'.
uint8_t TFMPlus::makeSum( uint8_t *data)
{
    uint16_t Sum = 0;   // Sum up all but last byte of data.
    for( uint8_t i = 0; i < ( data[ 1] - 1); i++) Sum += data[ i];
    return uint8_t(Sum);
}

// Create the proper command byte array, put the command,
// get a repsonse, and return the status
uint8_t TFMPlus::buildCommand( uint32_t cmnd, uint32_t param)
{
  uint8_t cmndData[ 8];               // Initialize an 8 byte command array
  memset( cmndData, 0, 8);            // Clear the array.

  cmndData[ 0] = 0x5A;                // First byte is the header character.
  memcpy( &cmndData[1], &cmnd, 3);    // Next three bytes are length, command
                                      // number and a one byte parameter
                                      // encoded as a 32 bit unsigned integer.
  if( cmndData[2] == 3)                 // If the command is to Set Frame Rate...
  {
    memcpy( &cmndData[ 3], &param, 2);  // include the 2 byte Frame Rate parameter.
  }
  else if( cmndData[2] == 6)            // If the command is to Set Baud Rate...
  {
    memcpy( &cmndData[ 3], &param, 4);  // include the 3 byte Baud Rate parameter.
  }

  cmndData[ cmndData[ 1] - 1] = makeSum( cmndData);  // Put a checksum as last byte of command

//   printData( cmndData, 8);   // for testing puroses
  putCommand( cmndData);
  return getResponse();
}
