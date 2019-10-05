# A Simulator for the HD6309 computer

See: https://github.com/trcwm/HD6309-Computer

Although the computer has an 6309 which has more
instructions than the 6809, the USIM library does
support these, so we'll have to stick to 6809
compatible binaries for now.

This will only compile/run on Linux.

# Monitor commands
* M XXXX - show memory at HEX address XXXX.
* R XXXX - jump to HEX address XXXX and run.
