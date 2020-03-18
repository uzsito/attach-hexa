# ![logo](graphics/logo.jpg)
Highly configurable, low-cost solution to produce fake cooling fan TACH signals for various computer systems.

## Description
Attach-Hexa is a combined circuit board, which consists of three parts:
* Custom pcb with a few cheap components, mounting holes & socket for a replaceable microcontroller
* Arduino Nano or any clone with the same pinout
* Code running on Arduino to handle outputs in a flexible way

With the code uploaded to the Arduino, a complete Attach-Hexa is able to drive six computer fan headers (channels) with TACH signals. Users can easily adjust the required RPM simulation on each channel, using pre-defined commands on serial interface.

![rpm_simulation](graphics/render1.jpg)
![rpm_simulation](graphics/render2.jpg)
![rpm_simulation](graphics/render3.jpg)
![rpm_simulation](graphics/render4.jpg)
![rpm_simulation](graphics/render5.jpg)

![rpm_simulation](graphics/rpm_simulation.jpg)

## Do I need this?
If you have a conventional computer system with ordinary cooling, you don't. However, Attach-Hexa can be useful in the following cases:
* In passively cooled, modded enterprise systems (e.g. home lab), where elimination of "missing fan" alerts is needed to prevent undesired behaviour and ensure proper booting of the system (many servers do not boot at all, if fans are not present)
* In any system with water cooling, where RPM simulation is handy for tuning or testing the setup
* In PWM operation analysis, or other similar hardware development procedure
* In any other case, where generating square-waves is helpful

## Technical Details
TACH signals are used to report the current RPM of cooling fans to the system. Each fan has a dedicated pin for this purpose, usually in the following pinout:

![rpm_simulation](graphics/header.jpg)

On most motherboards headers, TACH pin is pulled high (+10-12V) by default, using internal pullup resistor. As the connected fan turns, it's own built-in electronics (hall-effect sensor, transistor) pulls the TACH line to GND twice in every revolution. This results in a square-wave with a certain period, which is proportional to the fan's rotation speed. So if we would like to reproduce this, we have to pull that pin low at a specified rate. Arduino and similar microcontrollers can do that easily from code.

The custom pcb board is holding the Arduino Nano, and makes it capable of pulling +12V (or more) lines safely through transistors and base resistors. Arduino can be powered from any fan header (using Attach-Hexa GND & VIN pins as input) up to +12V, or from USB. The power source is automatically selected to the highest voltage.

## Features
* Six simultaneously working channels
* Built-in LED can be linked to any channel as a visual tracker
* Desired RPMs, LED linked channel and LED frequency settings can be adjusted at runtime
* Serial interface listener to receive commands
* All settings can be saved to EEPROM 

## BOM

## Serial interface
-terminal programs, settings
-command list
