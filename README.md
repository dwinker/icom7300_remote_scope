# icom7300_remote_scope

It is functional as an Alpha demo. It does update the FFT and Waterfall
displays on the PC or Laptop screen at the full speed of data coming from the
radio, so it does accurately reflect the quality that can be expected.

The video ICOM-7300_Remote_Scope_Video.mkv shows what can be expected for
performance. It was recorded on a seven year old PC with slow video. A faster
PC wouldn't have the slight flickers in the FFT display area.

Took code from fldigi for a scope and can read scope data from the radio. Work
in progress. I don't have build instructions, but if you get fldigi/flrig to
compile I'm pretty sure this will too.

Before using this turn off "Menu", "Set", "Connetors", "USB SEND" on the radio.
Apparently with Linux it is not possible to open a serial port without
momentarily activating DTR and RTS.
https://stackoverflow.com/questions/5090451/how-to-open-serial-port-in-linux-without-changing-any-pin/21753723
If "USB SEND" is anything other than off a short transmit control will be sent
from the PC anytime this program is used.

You need to set the USB/Serial port to 115200 baud for remote scope commands to
work. To do that you have to use the radio menus to disconnect USB CI-V from
Remote CI-V.

This program doesn't turn on the scope or change any settings on the radio
(other than to tell the radio to start or stop sending scope data to the PC).
Set up the scope from the radio menu, then start this program and click Off/On.
You can change bands or edge frequencies while this program and the scope are
running.

Right now I intend to implement the '27 xx xx' CIV commands here, and not much
else.  This is kind of the test rig for the scope. I want to incorporate this
remote scope into flrig

ToDo:
* Need to wait for response from radio before sending next message (when incorporated into flrig).
* Persistence on FFT display.
* Add markers.
* Tune radio from markers.
* Add gain and offset option.
* Resizeable.
