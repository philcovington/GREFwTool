GREFwTool Version 0.1 Readme   12/17/2016

This tool is a simple application that can update the firmware and set the
time for some GRE based scanners. Currently this includes the PSR-800, Pro-18,
Pro-668, and WS-1080.

# Installation
Installation of the tool is done by double clicking on the install file and
following the menus and prompts of the installer.

# Using The Program
The various functions are enabled and disabled based on the state of the tool
and scanner. The Settings dialog can only be displayed when the tool is
disconnected from the scanner. The Firmware Update function is only enabled
when the scanner is connected in CPU Update Mode, and the Connect function is
used to establish communications.

# Settings
The Settings dialog is used to set the type of scanner and the serial port
used for communications with the scanner. Information about the serial port
selected is displayed.

Scanner Type is set to the scanner hardware being used with the tool.

Firmware defaults to the same selection as the Scanner Type. It is used for a
very experimental transcode feature that allows firmware files to be used
between some scanner types. See the Firmware Transcode section below.

Protocol Debug is used for displaying scanner protocol for debugging purposes.
It displays the information sent and received over the serial port on the
display screen of the tool.

# Set Time and Date
Use the Set Time function to set the scanner to the same time and date as the
computer.

# Firmware Update
Updating the firmware on the scanner is split into two tasks, downloading the
firmware, and updating the scanner. This allows the user to go back to an
earlier firmware version by starting at step 2.

1. Use the Download Firmware function on the tool to download the firmware
file from the website. The tool will display the change information about the
firmware. Currently there is no check if the firmware version is already
downloaded.

2. With the scanner powered off and unplugged from the USB, put the scanner
in CPU Update mode. This is done by pressing the Menu button on the scanner
and plugging the scanner into the computer while holding the Menu button. The
menu button is released after the scanner is plugged into the computer and the
computer detects the scanner.

3. Plug the scanner in the USB Port. Wait until the computer detects
the scanner and sets up the port. Use the Connect function on the tool to
connect to the scanner. The display on the tool should show firmware version
information and the scanner being in CPU Update Mode.

4. Use the Firmware Update function on the tool to update the scanner. Select
the firmware file in the file dialog. The display and a progress dialog
should update with the progress of the update. The tool will disconnect from
the scanner when the update is complete.

Failed updates will put the scanner into CPU Update Mode when powered on.
Retry the Firmware Update from step 3. The scanner can only be powered off
in CPU Update Mode by disconnecting the usb connection and removing one of
the battery cells in the scanner.

# Firmware Transcode
This is a very experimental feature that allows using firmware from another
similar scanner. It does this by transcoding the firmware file so the
bootloader of the scanner will accept it. Otherwise the bootloader will erase
the current App firmware and not save the new App firmware.

The main usage is to add WS-1080 features to different models. This can have
limitations due to missing or different hardware between the two scanner
models. The Pro-668 and WS-1080 are basically the same hardware platform. The
PSR-800 has a different charging circuit than the WS-1080, so in-scanner
charging should be turned off. The Pro-18 has the same in-scanner charging
limitation as the PSR-800, plus it does not have the three color alert led.

Using WS-1080 firmware, or other firmware not meant for the scanner, does not
grant any support from the manufacturer of the firmware used. It also makes
any reporting of bugs and problems found invalid, since the problem could be
due to running the firmware on a different hardware platform.

The PC Application (EZ-Scan, etc...,) for the loaded firmware has to be used
to gain maximum benefit with the loaded firmware. Updating to WS-1080 firmware
will require a DSP update, and the PC application for the WS-1080 is required
to update the DSP. The SD Card may need to be initialized in the new PC
Application to set new parameters.

Do not update the CPU by using CPU Update in the PC Application! This will
result in the CPU firmware being erased and the scanner will power up in CPU
Update Mode. The tool will have to be used to reload the firmware and fix the
problem.

# Compiling The Source
The application was compiled using Qt Version 5.7.0. The installed QT Creator
was used as the IDE for code development. Qt Installer Framwork was used to
create the installer. Make sure the Qt Installer Framework bin directory is
included in the PATH variable of the Qt Build Environment.

Source code is available at: https://github.com/LinuxSheeple-E/GREFwTool

There are two project files, GREFwTool.pro to build the tool, and
GREFwToolInstaller.pro to build the windows installer. The project files assume
that the application and installer are built using a shadow build. The build
directory is named build and is under their respective project directory
(GREFwTool\build and GREFwTool\installer\build).

