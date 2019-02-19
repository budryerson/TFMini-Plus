# TFMini-Plus
#### Arduino Library for the Benewake TFMini-Plus Lidar sensor

The Benewake TFMini-Plus is a unique product and not merely an enhanced version of the TFMini. It has its own command and data structure. Therefore, various Arduino TFMini libraries are not compatible with the TFMini Plus.

The Plus features a UART serial communication interface. The serial baud rate is programmable.
<br />Only these rates are supported: 9600/14400/19200/56000/115200/460800/921600.
<br />The default baud rate is 115200.

Data output rates are programmable up to 10KHz, but the internal measuring frame rate is fixed at 4KHz.
<br />If the output rate is set to zero (0), single measurements can be triggered by command.
<br />The default data output rate is 100Hz.

The default format for each data frame consists of three measuremnets:
* Distance to target in centimeters. Range: 0 - 1200
* Strength of return signal in arbitrary units. Range: 0 - 65535
* Temperature of the device

#### Commands
This library now only supports two commands:
1. Read data frame: distance, signal strength, temperature
2. Return the device firmware version number
<br />More commands will follow in time.

An example Arduino sketch is included here: "TFMP_example.ino".
