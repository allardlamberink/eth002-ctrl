# ETH002-ctrl

###### General
* Control program for the ETH002 relay board from robot-electronics
* WARNING: use at your own risk, no warranty!!! code might be buggy
* More info about the ETH002 protocol see: robot-electronics.co.uk


###### Dependencies:
Boost library is used for XML parsing (settings file)


###### Command-line syntax:
```
eth002-ctrl [commandname] [relaynumber] [[0] to switch OFF or [1] to switch ON]
```

###### Example usage:
``` 
ETH002-ctrl version 0.
No arguments given, usage:
eth002-ctrl switch 1 1 (switch relay 1 on)
eth002-ctrl switch 1 0 (switch relay 1 off)
```

###### TODO:
- implement status read-back
- add support for multiple devices in 1 settings file
