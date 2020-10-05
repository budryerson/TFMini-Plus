# TFMini-Plus
### PLEASE NOTE:
With version v1.4.0, data variables are changed from unsigned to signed 16bit integers in order to support error codes returned in the `dist` (distance) and `flux` (signal strength) data.  The only wokring error code at the moment is `-1` returned as `flux` data when the return signal is saturated.

In the example code, `printStatus()` or `printErrorStatus()` has been replaced with `printFrame()` in response to a failed `getData()` or `printReply()` if responding to `sendCommand()`.
<hr />

### Arduino Library for the Benewake TFMini-Plus Lidar sensor

The **TFMini-S** is largely compatible with the **TFMini-Plus** and therefore able to use this library.  One difference is that the **TFMini-Plus** switches immediately upon command to change communication mode (`SET_I2C_MODE`, `SET_SERIAL_MODE`).  The **TFMini-S** requires a following `SAVE_SETTING` command.  This library is *not compatible* with the **TFMini**, which is an entirely different product with its own command and data structure.

With hardware v1.3.5 and firmware v1.9.0 and above, the communication interface of the TFMini-Plus can be configured to use either the default **UART** (serial) or the **I2C** (two-wire) protocol.  Additionaly, the device can be configured to output a binary (high/low) voltage level to signal that a detected object is within or beyond a user-defined range.  Please see the manufacturer's Product Manual in 'documents' for more information about the **I/O** output mode.

The UART serial baud-rate is user-programmable, but only the following rates are supported:
</br>9600, 14400, 19200, 56000, 115200, 460800, and 921600.<br>

Device data-frame output rates are programmable up to 10KHz, but the internal measuring frame-rate is fixed at 4KHz.
<br />"Standard" output rates are: 1, 2, 5, 10, 20, 25, 50, 100, 125, 200, 250, 500, and 1000Hz.
<br />If the output rate is set to 0 (zero), single data frames can be triggered by using the `TRIGGER_DETECTION` command.

Measurement data values are passed-back in three, 16-bit, signed integer variables:
<br />&nbsp;&nbsp;&#9679;&nbsp;  Distance to target in centimeters/millimeters. Range: 0 - 1200/12000
<br />&nbsp;&nbsp;&#9679;&nbsp;  Strength (voltage) or quality of returned signal in arbitrary units. Range: -1, 0 - 32767
<br />&nbsp;&nbsp;&#9679;&nbsp;  Temperature of the device in code. Range: -25°C to 125°C

The default TFMini-Plus communication interface is UART (serial); the default baud-rate is 115200 and the default data frame-rate is 100Hz.  Upon power-up in serial mode, the device will immediately start sending frames of measurement data.

This library supports the default, UART (serial) communication interface.  For communication in I2C mode, please use the TFMini-Plus-I2C version of the library.  Read more below about using the I2C mode of the device.
<hr />

### Arduino Library Commands
`begin()` passes a serial stream to the library and returns a boolean value indicating whether serial data is available. The function also sets a public one-byte `status` or error code.  Status codes are defined in the library's header file.

The `getData( dist, flux, temp)` function passes back three, signed, 16-bit measuremnent data values. It sets the `status` error code byte and returns a boolean value indicating 'pass/fail'.  If no serial data is received or no header sequence (`0x5959`) is detected within one (1) second, the function sets an appropriate `status` error code and 'fails'.  Given the asynchronous nature of the device, the serial buffer is flushed before reading and the `frame` and `reply` data arrays are zeroed out to delete any residual data.  This helps with valid data recognition and error discrimination.

A `getData( dist)` function is available that passes back only the distance value.

`sendCommand( cmnd, param)` sends a 32bit command and a 32bit parameter to the device.  It sets the `status` error code byte and returns a boolean 'pass/fail' value.  A proper command (`cmnd`) must be selected from the library's list of twenty defined commands.  A parameter (`param`) may be entered directly as an unsigned number, but it is better to choose from the Library's defined parameters because **an erroneous parameter can block communication and there is no external means of resetting the device to factory defaults.**

Any change of device settings (i.e. frame-rate or baud-rate) must be followed by a `SAVE_SETTINGS` command or else the modified values may be lost when power is removed.  `SYSTEM_RESET` and `RESTORE_FACTORY_SETTINGS` do not require a `SAVE_SETTINGS` command.

Benewake is not forthcoming about the internals of the device, however they did share this:
>Some commands that modify internal parameters are processed within 1ms.  Some commands require the MCU to communicate with other chips may take several ms.  And some commands, such as saving configuration and restoring the factory need to erase the FLASH of the MCU, which may take several hundred ms.

Also included:
<br />&nbsp;&nbsp;&#9679;&nbsp; An Arduino sketch "TFMP_example.ino" is in the Example folder.
<br />&nbsp;&nbsp;&#9679;&nbsp; Recent copies of the manufacturer's Datasheet and Product Manual are in Documents.
<br />&nbsp;&nbsp;&#9679;&nbsp; Valuable information regarding Time of Flight distance sensing in general and the Texas   Instruments OPT3101 module in particular are in a Documents sub-folder.

All of the code for this library is richly commented to assist with understanding and in problem solving.

### Using the I2C version of the device
According to Benewake:
>1- the measuring frequency of the module should be 2.5 times larger than the IIC reading frquency.
<br />2- the iic reading frequency should not exceed 100hz<br />

Because the Data Frame Rate is limited to 1000Hz, this condition implys a 400Hz data sampling limit in I2C mode.  Benewake says sampling should not exceed 100Hz.  They don't say why; but you might keep that limitation in mind when you consider using the I2C interface.

To configure the device for I2C communication, a command must be sent using the UART inteface.  Therefore, this reconfiguation should be made prior to the device's service installation, either by using this library's `SET_I2C_MODE` command or the serial GUI test application and code supplied by the manufacturer.

The `SET_I2C_MODE` command does not require a subsequent `SAVE_SETTINGS` command.  The device will remain in I2C mode after power has been removed and restored.  The only way to return to serial mode is with the `SET_SERIAL_MODE` command.  Even a `RESTORE_FACTORY_SETTINGS` command will NOT restore the device to its default, UART communication interface mode.

The device functions as an I2C slave device and the default address is `16` (`0x10` Hex) but is user-programable by sending the `SET_I2C_ADDRESS` command and a parameter in the range of `1` to `127`.  The new setting will take effect immediately and permanently without a `SAVE_SETTINGS` command, however the `RESTORE_FACTORY_SETTINGS` command will restore the default address.  The I2C address can be set while still in serial communication mode or, if in I2C mode, the example sketch included in the TFMini-Plus-I2C library can be used to test and change the address.

### Using the I/O modes of the device
The so-called I/O modes are not supported in this library.  Please do not attempt to use any I/O commands that you may find to be defined in this library's header file.
