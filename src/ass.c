/*
* Automatic screen shutdown (c) by Skywalker-11
* Version 1.0
*
* This program listens on the /dev/input files for mouse and keyboard and turns off the display
* after a period of time without input. If then an input is done it turns the display on again.
*
The MIT License (MIT)
Copyright © 2015 Skywalker-11 (https://github.com/Skywalker-11)

Copyright (c) 2013 glframebuffer glframebuffer@gmail.com (initial author)
Initial script (screensaber - https://github.com/glframebuffer/raspberrypi)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define MOUSEFILE "/dev/input/mice"
#define KEYBOARDFILE "/dev/input/event0"
#define FGCONSOLE "/bin/fgconsole"
#define TVSERVICEON "/opt/vc/bin/tvservice -p"
#define TVSERVICEOFF "/opt/vc/bin/tvservice -o"
#define CHVT "/bin/chvt %s"

/*
* writes the currenty tty console to currConsole
*/
bool getCurrConsole (FILE* fp, char* currConsole){	
	fp = popen(FGCONSOLE, "r");
	if(fp == NULL) {
		printf("WARNING: failed to get current display. You have manually switch the tty to get the display back.");
		return false;
	}
	fgets(currConsole, sizeof(currConsole)-1,fp);
	pclose(fp);
	return true;
}

int main( int argc, char** argv )
{
	bool verbose = false;
	int timeout = 30;

	FILE *fp_fgconsole;
	char currConsole[4];

	char to_temp_tty[(sizeof(CHVT)/sizeof(CHVT[0])) +2];  //stores the command to switch to a temporary tty
	sprintf(to_temp_tty, CHVT, "12");

	char command_buf[(sizeof(CHVT)/sizeof(CHVT[0])) +4];  //stores the command to switch back to original tty

        int fd_mouse;
	int fd_keyboard;
        struct input_event ie_mouse;
        struct input_event ie_keyboard;
	int read_out_mouse;
	int read_out_keyboard;
	int idle_time = 0;
	int HDMI_ON = 1;

	//parse arguments
	int c = 0;
	while((c = getopt (argc, argv, "ht:v")) != -1) {
		switch (c)
		{
			case 'h': 
				printf("Automatic screen shutdown (ass) (c) by Skywalker-11 \n\n");
				printf("This program turns off the display after a timout without keyboard/mouse activity.\n\n");
				printf("Usage: ass [option]\n");
				printf("The valid options are:\n");
				printf("-h               show this help\n");
				printf("-t <timeout>     set the timeout in seconds (default: 30 sec.).\n");
				printf("-v               enable verbose output");
				printf("NOTICE: In some cases you have to enter multiple inputs");
				printf("(eg. hit multiple keys) to bring the display back");
				exit(0);
			case 't': timeout=atoi(optarg); break;
			case 'v': verbose=true;  break;
			default: printf("ERROR: unknown parameter %c\n", c);exit(1);
		}
	}
	printf("Starting ahss with a timeout of %d \n", timeout);
//open the mouse input file to notice the mouse inputs
        if((fd_mouse = open(MOUSEFILE, O_RDONLY)) == -1) {
       	        perror(" ERROR: opening mouse device failed\n");
               	exit(EXIT_FAILURE);
        }
	int flags_mouse = fcntl(fd_mouse, F_GETFL, 0);
	fcntl(fd_mouse, F_SETFL, flags_mouse | O_NONBLOCK);
//open the keyboard input file to notice the keyboard inputs
        if((fd_keyboard = open(KEYBOARDFILE, O_RDONLY)) == -1) {
       	        perror(" ERROR: opening keyboard device failed\n");
               	exit(EXIT_FAILURE);
        }
	int flags_keyboard = fcntl(fd_keyboard, F_GETFL, 0);
	fcntl(fd_keyboard, F_SETFL, flags_keyboard | O_NONBLOCK);


        while(true) {
//read from the input files
		read_out_mouse = read(fd_mouse, &ie_mouse, sizeof(struct input_event));
		read_out_keyboard = read(fd_keyboard, &ie_keyboard, sizeof(struct input_event));

		if(read_out_mouse ==3 || read_out_keyboard ==16) 
		{ //an input was detected
	                printf("\nM: time %ld.%06ld\ttype %d\tcode %d\tvalue %d\n",
                           ie_mouse.time.tv_sec, ie_mouse.time.tv_usec, ie_mouse.type,
			   ie_mouse.code, ie_mouse.value
			);
	                printf("\nK: time %ld.%06ld\ttype %d\tcode %d\tvalue %d\n",
                           ie_keyboard.time.tv_sec, ie_keyboard.time.tv_usec, ie_keyboard.type,
			   ie_keyboard.code, ie_keyboard.value
			);
			//set idle timer to 0
			idle_time = 0;

			if(HDMI_ON == 0)
			{ 
				//get the current console and save it to currConsole
				bool success = getCurrConsole(fp_fgconsole, currConsole);

				//turn the screen on
				system (TVSERVICEON);
				system ("sudo xrefresh");
				if(success) { 
					//if the current tty could be received change the tty
					// and return to the old one
					// this is a workaround to update the display correctly
					sprintf(command_buf, CHVT, currConsole);
//					printf("/bin/chvt %s\n",currConsole);
					system(to_temp_tty);
					system(command_buf);
				}
			}
			HDMI_ON = 1;

		}
		else
		{// no input detected in past second
			if(idle_time> timeout )
			{//if timeout reached and display is on: turn it off
				if(HDMI_ON == 1) {
					printf("power down HDMI \n");
					system (TVSERVICEOFF);
					HDMI_ON = 0;
				}
			} else {
				idle_time++;
			}
		}

		//reopen the input files: this is neccessary because for each keyboard event 
		//multiple input_events are written to the input file
		//TODO: maybe find a better solution
	        if((fd_mouse = open(MOUSEFILE, O_RDONLY)) == -1) {
        	        perror(" ERROR: opening mouse device failed\n");
                	exit(EXIT_FAILURE);
	        }
		int flags_mouse = fcntl(fd_mouse, F_GETFL, 0);
		fcntl(fd_mouse, F_SETFL, flags_mouse | O_NONBLOCK);

	        if((fd_keyboard = open(KEYBOARDFILE, O_RDONLY)) == -1) {
        	        perror(" ERROR: opening keyboard device failed\n");
                	exit(EXIT_FAILURE);
	        }
		int flags_keyboard = fcntl(fd_keyboard, F_GETFL, 0);
		fcntl(fd_keyboard, F_SETFL, flags_keyboard | O_NONBLOCK);

		//sleep 1 second and then check everything again
		sleep(1);
        }
        return 0;
}

