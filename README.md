# TFMini-Plus
### Arduino Library for the Benewake TFMini-Plus Lidar sensor

The Benewake TFMini-Plus is a unique product and not an enhanced version of the TFMini. It has its own command and data structure.  This Arduino TFMini-Plus library is not compatible with the TFMini product.

With hardware v1.3.5 and firmware v1.9.0 and above, the communication interface of the TFMini-Plus can be configureed to use either the default UART (serial) or the I2C (two wire) protocol.  Additionaly, the device can be configured to output a binary voltage level (high or low) to signal that a detected object is within or beyond a user-defined range.  Please see the manufacturer's Product Manual in 'documents' for more information about this output mode.

The UART serial baud-rate is user-programmable, but only the following rates are supported:
</br>9600, 14400, 19200, 56000, 115200, 460800, and 921600.<br>

Device data-frame output rates are programmable up to 10KHz, but the internal measuring frame-rate is fixed at 4KHz.
<br />"Standard" output rates are: 1, 2, 5, 10, 20, 25, 50, 100, 125, 200, 250, 500, and 1000Hz.
<br />If the output rate is set to 0 (zero), single data frames can be triggered by using the `TRIGGER_DETECTION` command.

The default format for each frame of data consists of three measuremnets:
<br />&nbsp;&nbsp;&#9679;&nbsp;  Distance to target in centimeters. Range: 0 - 1200
<br />&nbsp;&nbsp;&#9679;&nbsp;  Strength (voltage) or quality of returned signal in arbitrary units. Range: 0 - 65535
<br />&nbsp;&nbsp;&#9679;&nbsp;  Temperature value of the device, library-translated to °C or °F

The default TFMini-Plus communication interface is UART (serial); the default baud-rate is 115200 and the default data-frame-rate is 100Hz.  Upon power-up, the device will immediately start sending serial measurement data frames.

At the moment, this library supports only the default, serial (UART) communication interface.  Please read more below about using the I2C version of the device.

### Arduino Library Commands
`begin()` passes a serial stream to the library and returns a boolean value indicating whether serial data is available. The function also sets a public one-byte `status` or error code.  Status codes are defined in the library's header file.

`getData( dist, flux, temp)` passes back three measurement values.  It sets the status/error code byte and returns a boolean 'pass/fail' value.  If no serial data or no header sequence (`0x5959`) is received within one (1) second, the function sets an appropriate error code and fails.  The serial buffer is flushed before reading and the `frame` and `reply` data arrays are zeroed out to delete any residual data.  Given the asynchronous nature of the device, flushing helps with valid data recognition and error discrimination.

`sendCommand( cmnd, param)` sends a 32bit command and a 32bit parameter to the device. It sets the status/error code byte and returns a boolean "pass/fail" value.  A command (`cmnd`) must be selected from the library's list of twenty defined commands. A parameter (`param`) may be entered directly as an unsigned number, but it is better to choose from the Library's defined parameters because **an erroneous parameter can block communication and there is no external means of resetting the device to factory defaults.**

Any change of device settings (i.e. frame-rate or baud-rate) must be followed by a `SAVE_SETTINGS` command or else the modified values will be lost when power is removed.  `SYSTEM_RESET` and `RESTORE_FACTORY_SETTINGS` do not require a `SAVE_SETTINGS` command.

Benewake is not forthcoming about the internals of the device, however they did share this.
>Some commands that modify internal parameters are processed within 1ms. Some commands require the MCU to communicate with other chips may take several ms. And some commands, such as saving configuration and restoring the factory need to erase the FLASH of the MCU, which may take several hundred ms.

An Arduino sketch "TFMP_example.ino" is included in example.
<br>Recent copies of the manufacturer's Datasheet and Product Manual are included in documents.

All of the code for this library is richly commented to assist with understanding and in problem solving.

### Some thoughts about the I2C version of the device
While this device can be configured to use an I2C communication protocol instead of the default, UART (serial) protocol, the modifying command must be sent using the default, UART inteface.  Therefore, the decision to use the I2C interface should be made "offline," prior to the device's service installation, by using either this library or the serial GUI test application supplied by the manufacturer.

The I2C address for the device is user-programable.  The default address is 0x10.  If a different adress is desired, a `SET_I2C_ADDRESS` command should be sent first, before sending the `SET_I2C_MODE command to modify the communication protocol.  It appears that the device will save the communication interface change and remain in I2C mode even after pwoer has been removed and restored.

Be aware that a `RESTORE_FACTORY_SETTINGS` command sent using the I2C protocol may restore the device to its default, UART communication protocol, which will make further I2C communication impossible.

A `SET_IO_MODE_HILO` or `SET_IO_MODE_LOHI` command sent using the I2C interface protocol will disable all further I2C communication.  Communication protocol must be reset to UART before this library will allow any IO_MODE command can be sent to the device.  This library will not allow "I/O Mode" commands to be sent wil the device is in I2C mode.

I2C mode is not yet supported in this version.  We are working on it.  A few I2C commands are defined in this library's header file, please do not attempt to use these codes at this time.
