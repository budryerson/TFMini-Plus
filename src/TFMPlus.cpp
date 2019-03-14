/* File Name: TFMPlus.cpp
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
 * 'buildCommand( cmnd, param)' sends a 32bit command code (cmnd)
 *  and a 32bit parameter value (param). Returns TRUE/FALSE and
 *  sets a one byte status code.
 *  Commands are selected from the library's list of defined commands.
 *  Parameter values can be entered directly (115200, 250, etc) but
 *  for safety, they should be chosen from the Library's defined lists.
 *  An incorrect value can render the device uncomminicative.
 *
 * Inspired by the TFMini Arduino Library of Peter Jansen (11 DEC 2017)
 */

#include <TFMPlus.h>

// Constructor
  TFMPlus::TFMPlus(){}
  TFMPlus::~TFMPlus(){}

// Return T/F if receiving serial data from device.
// And set system status to provide more information.
bool TFMPlus::begin(Stream *streamPtr)
{
  pStream = streamPtr;          // Store reference to stream/serial object
  delay( 10);                   // Delay for device data in serial buffer.
  if( (*pStream).available())   // If data present...
  {
    status = READY;             // set status to READY
    return true;                // and return TRUE.
  }
  else                          // Otherwise...
  {
     status = SERIAL;           // set status to SERIAL error
     return false;              // and return false.
  }
}

// Read one byte. Return TRUE/FALSE and set
// system error status. Try 20 times. This
// can wait up to 20 milliseconds for data.
bool TFMPlus::readData( uint8_t &rdByte)
{
  for( uint8_t i = 0; i < TFMP_MAX_READS; ++i)
  {
    if( (*pStream).available())       // If data is availabele...
    {
      rdByte = (*pStream).read();     // read one byte...
      status = READY;                 // set status to READY...
      return true;                    // and return "true".
    }
    delay(1);        // wait for a milliseond
  }
  status = SERIAL;   // If no serial data then set error...
  return false;      // and return "false".
}

// Return a checksum byte creted from the contents of 'data'.
// Length of 'data' is given in the second byte: 'data[1]'.
uint8_t TFMPlus::makeSum( uint8_t *data)
{
    uint8_t len;  // length of data
    if( data[ 0] == 0x5A) len = data[1] - 1;
      else len = TFMP_FRAME_SIZE - 1;
    uint16_t Sum = 0;   // Sum up all but last byte of data.
    for( uint8_t i = 0; i < len; i++) Sum += data[ i];
    return uint8_t(Sum);
}

// Read one complete frame of data from the device.
bool TFMPlus::getData( uint16_t &dist, uint16_t &flux, uint16_t &temp)
{
  uint8_t frame[ TFMP_FRAME_SIZE];  // initialize data frame
  memset( frame, 0, sizeof( frame));   // reset all bytes of frame to 0

  // Step 1 - Read serial bytes from device or return SERIAL error
  // if too many tries.  Continue until header code is detected
  // or return HEADEER error if too many bytes are read.
  int numCharsRead = 0;      // initalize char counter
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
    // then set HEADEER error and return false
    if( numCharsRead > MAX_BYTES_BEFORE_HEADER)
    {
      status = HEADER;
      return false;
    }
  }

  // Step 2 - Read rest of bytes to fill data frame
  // or halt and return 'false' if data runs out.
  // 'readData()' routine sets status byte
  for( int i = 2; i < TFMP_FRAME_SIZE; i++)
  {
    if( !readData( frame[ i])) return false;
  }

  // Step 3 - Get low order byte of sum of all bytes but
  // last in the frame and compare it to last byte of frame.
  if( makeSum( frame) != frame[ TFMP_FRAME_SIZE - 1])
  {
    status = CHECKSUM;
    return false;
  }

  // Step 4 - Interpret frame data
  dist = frame[ 2] + ( frame[ 3] << 8);
  flux = frame[ 4] + ( frame[ 5] << 8);
  temp = frame[ 6] + ( frame[ 7] << 8);

  status = READY;
  return true;
}

//  Get response to 'putCommand()'
bool TFMPlus::getResponse()
{
  uint8_t reply[ 8];                          // Initialize a response frame
  memset( reply, 0, sizeof( reply));          // Set all bytes of frame to zero

  // Step 1 - raad one byte and test for header character
  if( !readData( reply[ 0])) return false;
  if( reply[ 0] != 0x5A)
  {
    status = HEADER;       // If header not detected...
    return false;
  }
  // Step 2 - read next byte and use as frame length
  if( !readData( reply[ 1])) return false;
  for( uint8_t i = 2; i < reply[ 1]; i++)
  {
    if( !readData( reply[ i])) return false; // Read the rest of the reply.
  }
  
  // Step 3 - Create a checksum byte and compare
  // with the last byte of the response frame.
  if( makeSum( reply) != reply[ reply[ 1] - 1])
  {
      status = CHECKSUM;  // If not correct set CHECKSUM
      return false;       // and return 'false'.
  }
  
  // Step 4 - Interpret different command responses.
  // Case command number is 2, 10 or 11...
  if( reply[ 2] == 2 || reply[ 2] == 10 || reply[ 2] == 11)
  {
    if( reply[ 3] == 1)        // check PASS/FAIL byte...
    {
      status = FAIL;
      return false;            // then return 'pass'.
    }
  }
  else if( reply[ 2] == 1)     // Case command number is 1...
  {
      version[ 0] = reply[5];  //set firmware version.
      version[ 1] = reply[4];
      version[ 2] = reply[3];
  }
  
  status = READY;
  return true;
}

// Write command data to the device
void TFMPlus::putCommand( uint8_t *data)
{
  while( (*pStream).available()) (*pStream).read();  // flush input buffer
  (*pStream).flush();                                // flush output buffer
  for( uint8_t i = 0; i < data[ 1]; i++) (*pStream).write( data[ i]);
}

// Create the proper command byte array, put the command,
// get a repsonse, and return the status
bool TFMPlus::buildCommand( uint32_t cmnd, uint32_t param)
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

// - - - - -    For testing purposes    - - - - - -
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
