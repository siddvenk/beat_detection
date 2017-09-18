// File name: Interface_to_Linux.c
//
// Contains Linux specific code:
// 		the Linux read/write COM port support
//		sleep


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h> 
#include <signal.h>
#include <string.h>
#include <time.h>

char comPort[100] = "/dev/ttyUSB0";

struct termios tio;
struct termios stdio;
struct termios old_stdio;

int tty_fd;

static char exit_string[129];

unsigned exit_flag = 0;


void reset_terminal()
{
	close(tty_fd);
    tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);
}


void ExitProgram(char * text)
{
	//if (exit_flag++ != 0) exit(1);
	close(tty_fd);
    tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);
    printf("%s\n", text);
    printf("execution terminated\n");
	exit(1);
}

void initialize_data_source(long int baud_rate, unsigned sw_verbose)
{
	//signal(SIGINT, EndProgram);
		
	// Based on:  https://en.wikibooks.org/wiki/Serial_Programming/Serial_Linux

    tcgetattr(STDOUT_FILENO,&old_stdio);		// get present configuration
/*
	memcpy(&stdio, &old_stdio, sizeof(stdio));	// copy into new 
    //memset(&stdio,0,sizeof(stdio));
    
    //stdio.c_iflag = BRKINT;
    //stdio.c_oflag = OCRNL;
    //stdio.c_cflag = CS8 | CREAD |HUPCL;
    //stdio.c_lflag = ISIG; 
    stdio.c_lflag= stdio.c_lflag & ~(ICANON|ECHO);
    stdio.c_cc[VMIN]=1;
    stdio.c_cc[VTIME]=0;
    tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
    tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio); 
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking
*/
	// -----set up the com port connected to the client
	
    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=old_stdio.c_oflag;
    tio.c_cflag=CS8|CREAD|CLOCAL|CRTSCTS|HUPCL;    // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;

    tty_fd=open(comPort, O_RDWR | O_NONBLOCK);    
    if (tty_fd < 0) {
    	sprintf(exit_string, "can't open %s\n", comPort);
		ExitProgram(exit_string);
	}
	printf("%s opened\n", comPort);
	
    int controlbits;

	ioctl(tty_fd, TIOCMGET, &controlbits);
	controlbits |=  TIOCM_RTS;
	ioctl(tty_fd, TIOCMSET, &controlbits); 

	switch(baud_rate) {
    case      50 : baud_rate = B50;
                   break;
    case      75 : baud_rate = B75;
                   break;
    case     110 : baud_rate = B110;
                   break;
    case     134 : baud_rate = B134;
                   break;
    case     150 : baud_rate = B150;
                   break;
    case     200 : baud_rate = B200;
                   break;
    case     300 : baud_rate = B300;
                   break;
    case     600 : baud_rate = B600;
                   break;
    case    1200 : baud_rate = B1200;
                   break;
    case    1800 : baud_rate = B1800;
                   break;
    case    2400 : baud_rate = B2400;
                   break;
    case    4800 : baud_rate = B4800;
                   break;
    case    9600 : baud_rate = B9600;
                   break;
    case   19200 : baud_rate = B19200;
                   break;
    case   38400 : baud_rate = B38400;
                   break;
    case   57600 : baud_rate = B57600;
                   break;
    case  115200 : baud_rate = B115200;
                   break;
    case  230400 : baud_rate = B230400;
                   break;
    case  460800 : baud_rate = B460800;
                   break;
    case  500000 : baud_rate = B500000;
                   break;
    case  576000 : baud_rate = B576000;
                   break;
    case  921600 : baud_rate = B921600;
                   break;
    case 1000000 : baud_rate = B1000000;
                   break;
    case 1152000 : baud_rate = B1152000;
                   break;
    case 1500000 : baud_rate = B1500000;
                   break;
    case 2000000 : baud_rate = B2000000;
                   break;
    case 2500000 : baud_rate = B2500000;
                   break;
    case 3000000 : baud_rate = B3000000;
                   break;
    case 3500000 : baud_rate = B3500000;
                   break;
    case 4000000 : baud_rate = B4000000;
				   break;
    default: 	   sprintf(exit_string, "unsupported baud rate (%ld)\n", baud_rate);
				   ExitProgram(exit_string);
                   break;
  }
    cfsetospeed(&tio,baud_rate); 
    cfsetispeed(&tio,baud_rate);
    tcsetattr(tty_fd,TCSANOW,&tio);
}

#define COMBUFFERSIZE 256

// read bytes from client via the com port

unsigned char comRXbuffer[COMBUFFERSIZE+1] = {0};
uint8_t *read_ptr;
ssize_t BytesRead = 0;

uint16_t get_byte(uint8_t * value)
{
	uint16_t ret_value = 0;
	
	//printf("get_byte\n");
	if (BytesRead <= 0) {
		BytesRead = read(tty_fd, &comRXbuffer[0], COMBUFFERSIZE);
		read_ptr = &comRXbuffer[0];
	}
	if (BytesRead > 0) {
		BytesRead--;
		*value = *read_ptr++;
		ret_value = 1;			// indicates success in reading value
		//printf("get_byte: %c\n", *value);
	}
		return ret_value;		// otherwise return 0
}


// write one or more characters to the client via the com port

void put_byte(uint8_t * buffer, uint16_t size)
{
	unsigned ctr;
	
	//printf("%u buffer size\r\n", (unsigned)size); // testing
	for (ctr = 0; ctr < size; ctr++) {
		 write(tty_fd, buffer+ctr,1); 
	}
}

void stdin_flush()
{
    tcflush(0, TCIFLUSH);
    return;
}
