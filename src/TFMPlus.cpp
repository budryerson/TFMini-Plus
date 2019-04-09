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
 * v1.1.1 - 14MAR19 - Two commands: RESTORE_FACTORY_SETTINGS
 *    and SAVE_SETTINGS were not defined correctly.
 * v1.2.1 - 02APR19 - Rewrote 'getData()' function to make it faster
 *    and more robust when serial read skips a byte or fails entirely.
 * v1.3.1 - 08APR19 - Redefined commands to include response length
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

bool TFMPlus::getData( uint16_t &dist, uint16_t &flux, uint16_t &temp)
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1 - Get data from the device.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Set a one second timer to timeout if HEADER never appears
    // or serial data never becomes available
    uint32_t serialTimeout = millis() + 1000;
    frame[ 0] = 0;         // clear just the first header byte
    // Continuously read one byte into the end of the frame buffer
    // and then left shift the whole buffer until the two HEADER
    // bytes appear as thte first two bytes of the frame.
    while( (frame[ 0] != 0x59) || (frame[ 1] != 0x59))
    {
        if( (*pStream).available())
        {
            // Read one byte into the framebuffer's
            // last plus one position.
            frame[ TFMP_FRAME_SIZE] = (*pStream).read();
            // Shift the last nine bytes one byte left.
            memcpy( frame, frame+1, TFMP_FRAME_SIZE);
        }
        // If HEADER or serial data are not available
        // for more than one second...
        if( millis() >  serialTimeout)
        {
            status = SERIAL;   // then set error...
            return false;      // and return "false".
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2 - Perform a checksum test.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add together all bytes but the last.
    uint16_t Sum = 0;
    for( uint8_t i = 0; i < ( TFMP_FRAME_SIZE - 1); i++) Sum += frame[ i];
    //  If the low order byte does not equal the last byte...
    if( ( uint8_t)Sum != frame[ TFMP_FRAME_SIZE - 1])
    {
      status = CHECKSUM;  // then set error...
      return false;       // and return "false."
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 3 - If all okay then interpret the frame data...
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    dist = frame[ 2] + ( frame[ 3] << 8);
    flux = frame[ 4] + ( frame[ 5] << 8);
    temp = frame[ 6] + ( frame[ 7] << 8);
    status = READY;    // set status to READY...
    return true;       // and return "true".
}

// Create the proper command byte array, put the command,
// get a repsonse, and return the status
bool TFMPlus::sendCommand( uint32_t cmnd, uint32_t param)
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1 - Build the command data to send to the device
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    static uint8_t cmndLen;             // Length of command
    static uint8_t replyLen;            // Length of command reply data
    static uint8_t cmndData[ 8];        // 8 byte send command array
    static uint16_t checkSum;           // Used to calculate the checksum byte

    memset( cmndData, 0, 8);            // Clear the send command array.
    memcpy( &cmndData[0], &cmnd, 4);    // Copy 4 bytes of data: reply length,
                                        // command length, command number and
                                        // a one byte parameter, all encoded as
                                        // a 32 bit unsigned integer.

    replyLen = cmndData[ 0];            // Save the first byte as reply length.
    cmndLen = cmndData[1];              // Save the second byte as command length.
    cmndData[ 0] = 0x5A;                // Set the first byte to the header character.

    if( cmnd == SET_FRAME_RATE)           // If the command is to Set Frame Rate...
    {
      memcpy( &cmndData[ 3], &param, 2);  // add the 2 byte Frame Rate parameter.
    }
    else if( cmnd == SET_BAUD_RATE)       // If the command is to Set Baud Rate...
    {
      memcpy( &cmndData[ 3], &param, 4);  // add the 3 byte Baud Rate parameter.
    }

    // Create a checksum.
    // Add together all bytes but the last...
    checkSum = 0;
    for( uint8_t i = 0; i < ( replyLen - 1); i++) checkSum += reply[ i];
    // and save it as the last byte of command data.
    cmndData[ cmndLen - 1] = (uint8_t)checkSum;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2 - Send the command data to the device
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    while( (*pStream).available()) (*pStream).read();  // flush input buffer
    (*pStream).flush();                                // flush output buffer
    for( uint8_t i = 0; i < cmndLen; i++) (*pStream).write( cmndData[ i]);

    // If the command is this one, then we're done.
    // Escape and call getData() as a response instead.
    if( cmnd == TRIGGER_DETECTION) return true;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 3 - Get data back fromn the device.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Set a one second timer to timeout if HEADER never appears
    // or serial data never becomes available
    uint32_t serialTimeout = millis() + 1000;
    //memset( reply, 0, sizeof( reply));   // Set all bytes of reply to zero
    reply[0] = 0;
    // Continuously read one byte into the end of the frame buffer
    // and then left shift the whole buffer until the two HEADER
    // bytes appear as thte first two bytes of the frame.
    while( reply[ 0] != 0x5A)
    {
        if( (*pStream).available())
        {
            // Read one byte into the framebuffer's
            // last plus one position.
            reply[ replyLen] = (*pStream).read();
            // Shift the last nine bytes one byte left.
            memcpy( reply, reply+1, TFMP_REPLY_SIZE);
        }
        // If HEADER or serial data are not available
        // for more than one second...
        if( millis() >  serialTimeout)
        {
            status = SERIAL;   // then set error...
            return false;      // and return "false".
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 4 - Perform a checksum test.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add together all bytes but the last.
    uint16_t Sum = 0;
    for( uint8_t i = 0; i < ( replyLen - 1); i++) Sum += reply[ i];
    //  If the low order byte does not equal the last byte...
    if( ( uint8_t)Sum != reply[ replyLen - 1])
    {
      status = CHECKSUM;  // then set error...
      return false;       // and return "false."
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 5 - Interpret different command responses.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if( cmnd == OBTAIN_FIRMWARE_VERSION)
    {
        version[ 0] = reply[5];  // set firmware version.
        version[ 1] = reply[4];
        version[ 2] = reply[3];
    }
    else
    {
        if( cmnd == SYSTEM_RESET ||
            cmnd == RESTORE_FACTORY_SETTINGS ||
            cmnd == SAVE_SETTINGS )
        {
            if( reply[ 3] == 1)      // If PASS/FAIL byte not zero ...
            {
                status = FAIL;       // set status 'FAIL'...
                return false;        // and return 'false'.
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 6 - Set READY status and go home
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    status = READY;
    return true;
}

// - - - - -    For testing purposes    - - - - - -
// Print status 'READY' or 'Error:'
void TFMPlus::printStatus( bool isFrameData)
{
    if( isFrameData) printFrame();
      else printReply();
    Serial.print(" Status: ");
    if( status == READY)   Serial.print( "READY");
    else if( status == SERIAL)   Serial.print( "SERIAL");
    else if( status == HEADER)   Serial.print( "HEADER");
    else if( status == CHECKSUM) Serial.print( "CHECKSUM");
    else if( status == TIMEOUT)  Serial.print( "TIMEOUT");
    else if( status == PASS)  Serial.print( "PASS");
    else if( status == FAIL)  Serial.print( "FAIL");
    else Serial.print( "OTHER");
    Serial.println();
}

// Print the Hex value of each bytye of data
void TFMPlus::printFrame()
{
  printf(" Data:");
  for( uint8_t i = 0; i < TFMP_FRAME_SIZE; i++)
  {
    printf(" %02x", frame[ i]);
  }
}

void TFMPlus::printReply()
{
  printf(" Data:");
  for( uint8_t i = 0; i < reply[1]; i++)
  {
    printf(" %02x", reply[ i]);
  }
}
