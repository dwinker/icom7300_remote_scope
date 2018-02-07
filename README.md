# icom7300_remote_scope

Rudimentary functionality. Took code from fldigi for a scope and can read
scope data from the radio. Not integrated yet. Work in progress.

Right now I intend to implement the '27 xx xx' CIV commands here, and not much
else.  I am not taking the command code from flrig because it's big and if I'm
going to make this work as a client to flrig somehow it would be a waste of
time to have ported all that code to here - and flrig doesn't seem to have any
of the '27 xx xx' commands anyhow. I could try to follow flrig's pattern for
implementing CIV commands, but the fastest way for me to get "proof-on-concept"
is to implement commands the way that is easiest for me. Eventually that may be
a prototype for integration into flrig.
