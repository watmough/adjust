# adjust - dynamic adjustment for system parameters
```
GNU/Linux CLI utility that permits easy dynamic adjustment for system
parameters such as brightness, gamma etc.

adjust

a tiny utility for adjusting attributes in the terminal.
usage: adjust <attribute>
       left/right cursor keys to decrement/increment
       q to quit

Function: display minimal text ui outputting saved/adjusted attribute 
value to a preconfigured command. q to end.

How it works: A configuration provides a value range, a step value, and
an initial value. This provides a value which can be manipulated by the
left / right cursor keys. 
After each change of value, the provided command is run with '%' being
replaced by the value.

Bugs/Issues: There is very little error checking, as this is really just
a personal tool
No configuration except by editing additional attributes into the
~/.adjustments configuration file.
If you want to run it as a normal user, you'll likely need to change the 
owner / permissions to SUID.

Examples of items that can be adjusted by this tool

1.  GNU/Linux XWindows Gamma Value
    Command to use: xgamma -gamma value
    Reasonable value range: 0.1 to 0.90 by 0.1 default 0.4

2.  NVidia-specific Brightness Command
    Command to use: echo value > /sys/class/backlight/nvidia_0/brightness
    Reasonable value range: 15 to 95 by 5 default 20

Example ~/.adjustments Configuration File - Place the following four lines
in ~/.adjustments

gamma 0.1 0.9 0.1 0.5
xgamma -gamma %
brightness 15 95 5 20
echo % > /sys/class/backlight/nvidia_0/brightness

Build Instructions
g++ adjust.cpp -lcurses -o adjust

To provide a regular user with superuser privileges for writing to /dev
items. [1]

sudo chown root adjust
sudo chmod +s adjust

[1] Thanks to Dave Munroe at St Andrew Comp.Sci for showing us this, when he
likely got tired of restarting SunOS NFS lock daemons for us...
```
