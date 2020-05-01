# IoT First Boot Application Software

The purpose of this software is to solve the problem of allowing a user to boot a small
embedded IoT device for the first time and get it connected to a home WiFi network without
the use of any physical interface on the device itself. This is a first step in developing software
for any IoT device. By providing an open source solution, it is hoped that other developers
will be able to make use of this software to create projects quickly and easily.

## How to use the Software
In order for a developer to use this software with their own application, they will need to:

* Remove the files example\_secondary\_app, thingspeak, and http, as these are used as an example of how this software can be used.
* Include the main file of their application in the file iot\_fb\_main.
* Modify the function pointer pt2secondaryAPP from log\_to\_thingspeak to their application main function.