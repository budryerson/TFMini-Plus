/* File Name: TFMP_example.ino
 * Developer: Bud Ryerson
 * Inception: 29JAN2019
 * Last work: 22JUL2020

 * Description: Arduino sketch to test the Benewake TFMini Plus
 * time-of-flight Lidar ranging sensor using the TFMPlus library.

 * Default settings for the TFMini Plus are a 115200 serial baud rate
 * and a 100Hz measurement frame rate. The device will begin returning
 * measurement data right away:
 *   Distance in centimeters,
 *   Signal strength in arbitrary units,
 *   and an encoded number for Temperature in degrees centigrade.

 * Use the 'sendCommand()' to send commands and return a status code.
 * Commands are selected from the library's list of defined commands.
 * Parameters can be entered directly (115200, 250, etc) but for
 * safety, they should be chosen from the library's defined lists.
 */

#include <TFMPlus.h>  // Include TFMini Plus Library v1.4.1
TFMPlus tfmP;         // Create a TFMini Plus object

// The Software Serial library is an alternative for devices that
// have only one hardware serial port. Delete the comment slashes
// on lines 37 and 38 to invoke the library, and be sure to choose
// the correct RX and TX pins: pins 10 and 11 in this example. Then
// in the 'setup' section, change the name of the hardware 'Serial2'
// port to match the name of your software serial port, such as:
// 'mySerial.begin(115200); etc.

#include <SoftwareSerial.h>       
SoftwareSerial mySerial( 10, 11);   
#define TFMPSERIAL mySerial

// else, you can use a hardwareserial port!
//#define TFMPSERIAL Serial1

void setup()
{
    Serial.begin( 115200);   // Intialize terminal serial port
    delay(20);               // Give port time to initalize
    
    Serial.println("TFMPlus Library Simple Demo");  // say 'hello'

    TFMPSERIAL.begin( 115200);  // Initialize TFMPLus device serial port.
    delay(20);                  // Give port time to initalize
    tfmP.begin(&TFMPSERIAL);    // Initialize device library object and...
                                // pass device serial port to the object.

    // Send some example commands to the TFMini-Plus
    // - - Perform a system reset - - - - - - - - - - -
    Serial.print("System reset: ");
    if (tfmP.sendCommand(SYSTEM_RESET, 0)) {
      Serial.println("passed");
    } else {
      Serial.println("failed");
      tfmP.printReply();
      while (1) delay(1); // wait forever
    }
  
    delay(500);  // added to allow the System Reset enough time to complete

    // - - Display the firmware version - - - - - - - - -
    Serial.print("Firmware version: ");
    if (tfmP.sendCommand(OBTAIN_FIRMWARE_VERSION, 0)) {
      Serial.print(tfmP.version[0]);
      Serial.print(".");
      Serial.print(tfmP.version[1]);
      Serial.print(".");
      Serial.println(tfmP.version[2]);
    } else {
      tfmP.printReply();
    }
    
    // - - Set the data frame-rate to 20Hz - - - - - - - -
    Serial.print("Data-Frame rate: ");
    if (tfmP.sendCommand(SET_FRAME_RATE, FRAME_20)) {
        Serial.print(FRAME_20);
        Serial.println(" Hz");
    } else {
      tfmP.printReply();
    }

    delay(500);            // And wait for half a second.
}

void loop() {
    int16_t tfDist, tfFlux, tfTemp;
  
    // Use the 'getData' function to pass back device data.
    if (tfmP.getData(tfDist, tfFlux, tfTemp)) { // Get data from the device.
      Serial.print("Distance (cm): ");
      Serial.println(tfDist);   // display distance,
    }
}