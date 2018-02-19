# icom7300_remote_scope

It is functional as an Alpha demo. It does update the FFT and Waterfall
displays on the PC or Laptop screen at the full speed of data coming from the
radio, so it does accurately reflect the quality that can be expected.

Took code from fldigi for a scope and can read scope data from the radio. Work
in progress. I don't have build instructions, but if you get fldigi/flrig to
compile I'm pretty sure this will too.

Right now I intend to implement the '27 xx xx' CIV commands here, and not much
else.  I am not taking the command code from flrig because it's big and if I'm
going to make this work as a client to flrig somehow it would be a waste of
time to have ported all that code to here - and flrig doesn't seem to have any
of the '27 xx xx' commands anyhow. I could try to follow flrig's pattern for
implementing CIV commands, but the fastest way for me to get "proof-on-concept"
is to implement commands the way that is easiest for me. Eventually that may be
a prototype for integration into flrig.

ToDo:
* Need to wait for response from radio before sending next message.
* Turn off scope data from radio when exiting.
* Persistence on FFT display.
* Add scale (the green bar is place reserved for scale).
* Add markers.
* Tune radio from markers.
* Add gain and offset option.
* Resizeable.
