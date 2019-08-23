# TFMini-Plus
### Arduino Library for the Benewake TFMini-Plus Lidar sensor

The Benewake TFMini-Plus is a unique product and not an enhanced version of the TFMini. It has its own command and data structure.  This Arduino TFMini-Plus library is not compatible with the TFMini product.

With hardware v1.3.5 and firmware v1.9.0 and above, the communication interface of the TFMini-Plus can be configured to use either the default **UART** (serial) or the **I2C** (two-wire) protocol.  Additionaly, the device can be configured to output a binary (high/low) voltage level to signal that a detected object is within or beyond a user-defined range.  Please see the manufacturer's Product Manual in 'documents' for more information about the **I/O** output mode.

The UART serial baud-rate is user-programmable, but only the following rates are supported:
</br>9600, 14400, 19200, 56000, 115200, 460800, and 921600.<br>

Device data-frame output rates are programmable up to 10KHz, but the internal measuring frame-rate is fixed at 4KHz.
<br />"Standard" output rates are: 1, 2, 5, 10, 20, 25, 50, 100, 125, 200, 250, 500, and 1000Hz.
<br />If the output rate is set to 0 (zero), single data frames can be triggered by using the `TRIGGER_DETECTION` command.

The default format for each frame of data consists of three 16-bit measurement values:
<br />&nbsp;&nbsp;&#9679;&nbsp;  Distance to target in centimeters. Range: 0 - 1200
<br />&nbsp;&nbsp;&#9679;&nbsp;  Strength (voltage) or quality of returned signal in arbitrary units. Range: 0 - 65535
<br />&nbsp;&nbsp;&#9679;&nbsp;  Temperature of the device in code. Range: -25°C to 125°C

The default TFMini-Plus communication interface is UART (serial); the default baud-rate is 115200 and the default data frame-rate is 100Hz.  Upon power-up in serial mode, the device will immediately start sending frames of measurement data.

At the moment, this library supports only the default, UART (serial) communication interface.  Please read more below about using the I2C version of the device.

### Arduino Library Commands
`begin()` passes a serial stream to the library and returns a boolean value indicating whether serial data is available. The function also sets a public one-byte `status` or error code.  Status codes are defined in the library's header file.

`getData( dist, flux, temp)` passes back three measurement values.  It sets the `status` error code byte and returns a boolean value indicating 'pass/fail'.  If no serial data is received or no header sequence (`0x5959`) is detected within one (1) second, the function sets an appropriate `status` error code and 'fails'.  Given the asynchronous nature of the device, the serial buffer is flushed before reading and the `frame` and `reply` data arrays are zeroed out to delete any residual data.  I find that this helps with valid data recognition and error discrimination.

`sendCommand( cmnd, param)` sends a 32bit command and a 32bit parameter to the device. It sets the `status` error code byte and returns a boolean 'pass/fail' value.  A proper command (`cmnd`) must be selected from the library's list of twenty defined commands.  A parameter (`param`) may be entered directly as an unsigned number, but it is better to choose from the Library's defined parameters because **an erroneous parameter can block communication and there is no external means of resetting the device to factory defaults.**

Any change of device settings (i.e. frame-rate or baud-rate) must be followed by a `SAVE_SETTINGS` command or else the modified values may be lost when power is removed.  `SYSTEM_RESET` and `RESTORE_FACTORY_SETTINGS` do not require a `SAVE_SETTINGS` command.

Benewake is not forthcoming about the internals of the device, however they did share this.
>Some commands that modify internal parameters are processed within 1ms. Some commands require the MCU to communicate with other chips may take several ms. And some commands, such as saving configuration and restoring the factory need to erase the FLASH of the MCU, which may take several hundred ms.

An Arduino sketch "TFMP_example.ino" is included in example.
<br>Recent copies of the manufacturer's Datasheet and Product Manual are included in documents.

All of the code for this library is richly commented to assist with understanding and in problem solving.

### Some thoughts about the I2C version of the device
The command to re-configure this device to use an I2C communication protocol instead of the default, UART (serial) protocol must be sent using the default, UART inteface.  Therefore, the decision to use the I2C interface should be made 'offline,' prior to the device's service installation, either by using this library or the serial GUI test application supplied by the manufacturer.

Ths device functions as an I2C slave device.  Its address is user-programable.  The default is `0x10`.  If a different address is desired, a `SET_I2C_ADDRESS` command, followed by a one byte address parameter in the range of `0x01` to `0x7F`, can be sent.

It appears that the device will retain a change to it's communication interface without a `SAVE_SETTINGS` command being sent.  Once set, it will remain in I2C mode even after power has been removed and restored.  Apparently, even a `RESTORE_FACTORY_SETTINGS` command sent while in I2C mode will not restore the device to its default, UART communication interface mode.  It seems that the only way to get back to serial mode is with the `SET_SERIAL_MODE` command.

A `SET_IO_MODE_HILO` or `SET_IO_MODE_LOHI` command sent while in I2C mode may probably disable all further I2C communication. I strongly  recommend that **communication should be reset to UART before any I/O mode command is sent** to the device.

I2C and I/O modes are not yet supported in this version of the library.  They are coming very soon.  Please do not attempt to use the I2C and I/O commands that are defined in this library's header file at this time.
