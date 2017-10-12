#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	const char file_path[20] = "/dev/eudyptula";
	int fd = 0, ret = 0;
	char r_buf[15] = {0};
	char *buf = NULL;

	if (argc < 2) {
		printf("Usage: ./test <string>\n");
		printf("String is the value to be written into device node\n");
		return -1;
	}else {
		buf = strdup(argv[1]);
		strcpy(buf, argv[1]);
		printf("buf = %s\n", buf);
	}
		
	fd = open(file_path, O_RDWR);
	if (fd < 0) {
		printf("Error in fopen...error=%s\n", strerror(errno));
		return -1;
	}
	
	if (read(fd, r_buf, sizeof(r_buf)) < 0)
		printf("Error in read operation due to %s\n", strerror(errno));
	else
		printf("String read from device node = %s\n", r_buf);
	
	ret = write(fd, buf, strlen(buf));
	if (ret < 0) 
		printf("Error in write operation due to %s\n", strerror(errno));
	else
		printf("Write operation success. %d number of bytes written\n", ret);
	
	close(fd);
	return 0;
}
