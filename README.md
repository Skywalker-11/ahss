# ass
This Program to turns off the display automatically after a period of inactivity and turn it on when an key/mouse event was triggered.

In the ```bin/``` folder exists a precompiled version for the RaspberryPi with Raspbian.


##Requirements:
* The mouse triggers ```/dev/input/mice```, the keyboard triggers ```/dev/input/event0``` (one of them have to be available)
* ```fgconsole``` and ```chvt``` are installed under ```/bin/``` (for a workaround to refresh the display correctly)
* tvservice (https://github.com/raspberrypi/userland/tree/master/host_applications/linux/apps/tvservice) is installed under ```/opt/vc/bin/tvservice``` (to turn the screen on/off)

On a RaspberryPi this should work out of the box with the precompiled version (tested with RaspberryPi 2 and B+).

##Build
Just do a ```make all``` in the root folder

##Usage
```ass [options]```
The following options are available:

| Options            | Desription 
| ------------------ |:-------------|
| ```-h```           | shows the help 
| ```-t <timeout>``` | sets the timeout in seconds (default is 30) 
| ```-v```           | enables verbose messages 

If you only use the console and no XServer you will receive ```sudo: xrefresh: command not found``` when the screen is turned back on. You could ignore that.

##Examples
This command will turn the screen off after 300 seconds of inactivity (no key pressed and mouse not moved). If you then press a key or move the mouse it will turn on again.
```$ ./ass -t 300```

To start the program in the background use:
```$ ./ass -t 300 &``



##Notice
If you want to trigger something else than a display shutdown after the timeout (eg. logoff the current user) you can do that with the program too. You just have to change the value of ```SCREEN_OFF``` in the source file and build it.You could also use other input files by changing ```MOUSEFILE``` or ```KEYBOARDFILE```.
