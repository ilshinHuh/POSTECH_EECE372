#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define BAUDRATE B1000000

int main()
{
	int fd;
	struct termios newtio;
	char fbuf[1024];
	char buf[256];

	fd = open("/dev/serial0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {

		// Insert your code
		// check if picture is taken
		static int picture_taken = 1;
		
		// read transmitted characters
		int cnt = read(fd, buf, sizeof(buf));
		buf[cnt] = '\0';

		// if picture is not taken yet and the first character is 'c' or 'C'
		if(picture_taken && (buf[0] == 'c' || buf[0] == 'C')){
			printf("cheeze\r\n");
			system("libcamera-still --width 640 --height 480 -o image.jpg");

			// open image file and read
			FILE* fp = fopen("image.jpg", "rb");

			// get file size
			fseek(fp, 0, SEEK_END);
			long file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			// read and write file data into dynamically allocated memory
			unsigned char* data = (unsigned char*)malloc(file_size);
			fread(data, 1, file_size, fp);
			write(fd, data, file_size);

			// close file and free memory
			fclose(fp);
			free(data);

			// set the static flag to indicate that the picture is taken
			picture_taken = 0;
		}
		
		if(picture_taken == 0)
			break;
	}
	return 0;
}
