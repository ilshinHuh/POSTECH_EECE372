#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

// import libraries
#include <wiringPi.h>

// set led variables
#define A 2
#define B 3
#define C 4
#define D 5
#define E 6
#define F 7
#define G 22

// set 7-segment number count variable function
int set_count(char buf);

#define BAUDRATE B1000000

int fd;
char buf[256];

void callback_function(int status)
{
	int cnt = read(fd, buf, 256);
	buf[cnt] = '\0';
	write(fd, "echo: ", 6);
	write(fd, buf, cnt);
	write(fd, "\r\n", 2);
	printf("Received: %s\r\n", buf);
	
	switch(set_count(buf[0])){
		case 0:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,0);
		break;
		case 1:
		digitalWrite(A,0);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,0);
		digitalWrite(E,0);
		digitalWrite(F,0);
		digitalWrite(G,0);
		break;
		case 2:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,0);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,0);
		digitalWrite(G,1);
		break;
		case 3:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,0);
		digitalWrite(F,0);
		digitalWrite(G,1);
		break;
		case 4:
		digitalWrite(A,0);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,0);
		digitalWrite(E,0);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 5:
		digitalWrite(A,1);
		digitalWrite(B,0);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,0);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 6:
		digitalWrite(A,1);
		digitalWrite(B,0);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 7:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,0);
		digitalWrite(E,0);
		digitalWrite(F,1);
		digitalWrite(G,0);
		break;
		case 8:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 9:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,0);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 10:
		digitalWrite(A,1);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,0);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 11:
		digitalWrite(A,0);
		digitalWrite(B,0);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 12:
		digitalWrite(A,0);
		digitalWrite(B,0);
		digitalWrite(C,0);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,0);
		digitalWrite(G,1);
		break;
		case 13:
		digitalWrite(A,0);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,0);
		digitalWrite(G,1);
		break;
		case 14:
		digitalWrite(A,1);
		digitalWrite(B,0);
		digitalWrite(C,0);
		digitalWrite(D,1);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 15:
		digitalWrite(A,1);
		digitalWrite(B,0);
		digitalWrite(C,0);
		digitalWrite(D,0);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
		break;
		case 99:
		digitalWrite(A,0);
		digitalWrite(B,1);
		digitalWrite(C,1);
		digitalWrite(D,0);
		digitalWrite(E,1);
		digitalWrite(F,1);
		digitalWrite(G,1);
	}
}

void task()
{
	int i;
	for(i=0; i<400000000; i++);
}

int main()
{	
	// set wiringPi
	if(wiringPiSetup()==-1)
		return 0;

	// set pin
	pinMode(A,OUTPUT);
	pinMode(B,OUTPUT);
	pinMode(C,OUTPUT);
	pinMode(D,OUTPUT);
	pinMode(E,OUTPUT);
	pinMode(F,OUTPUT);
	pinMode(G,OUTPUT);

	struct termios newtio;
	struct sigaction saio;

	fd = open("/dev/serial0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&saio, 0, sizeof(saio));	
	saio.sa_handler = callback_function;
	saio.sa_restorer = NULL;
	sigaction(SIGIO, &saio, NULL);

	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	speed_t baudRate = B1000000;      //Use when there is a problem with Baudrate
	cfsetispeed(&newtio, baudRate);
	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	write(fd, "interrupt method\r\n", 18);
	while(1) {

		task();
	}
	return 0;
}

int set_count(char buf){
	if(buf == '0') return 0;
	else if(buf == '1') return 1;
	else if(buf == '2') return 2;
	else if(buf == '3') return 3;
	else if(buf == '4') return 4;
	else if(buf == '5') return 5;
	else if(buf == '6') return 6;
	else if(buf == '7') return 7;
	else if(buf == '8') return 8;
	else if(buf == '9') return 9;
	else if(buf == 'A') return 10;
	else if(buf == 'B') return 11;
	else if(buf == 'C') return 12;
	else if(buf == 'D') return 13;
	else if(buf == 'E') return 14;
	else if(buf == 'F') return 15;
	else return 99;
}
