#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dpvideo.h"

pthread_t pid_read264;
pthread_t pid_readjpg;
int pfd[2];
struct decodebuf {
	char *d;
	int len;
};

void *thread_read264(void *arg);
void *thread_readjpg(void *arg);

int main()
{
	struct decodebuf dbuf;
 
int c;
	pipe(pfd);
	//H264DecStart(0, 0, 1024, 600);
	JPGDecStart(0, 0, 1024, 600);


	//pthread_create(&pid_read264, NULL, thread_readcode, pfd);
	pthread_create(&pid_readjpg, NULL, thread_readjpg, pfd);

	while (1) {
		read(pfd[0], &dbuf, sizeof(dbuf));
		H264WriteData(dbuf.d, dbuf.len, 0);
printf("%d %d %x\n", c++, dbuf.len, dbuf.d);

	}

	H264DecStop();


	return 0;
}


#define SBUFSIZE 1024
#define BUFS 10*1024
#define BUFN 5
#define FPS(x) do {usleep(1000000/x);} while (0);
#define H264FILE "helloo.264"
#define JPGFILE "0.jpeg"

void *thread_readjpg(void *arg)
{
	int ret;
	int i;
	int *pfd = arg;
	struct decodebuf dbuf[BUFN];
	int fd;

	unsigned char *buf = malloc(BUFS*BUFN);

	char *jfile[] = {
		"0.jpeg",
		"1.jpeg",
		"2.jpeg",
		"3.jpeg",
		"4.jpeg",
		"5.jpeg",
		"6.jpeg",
		"7.jpeg",
		"8.jpeg",
		"9.jpeg",
	};

	while (1) {
		fd = open(jfile[(i++)%10], O_RDWR);
		ret = read(fd, buf, BUFS*BUFN);
		dbuf[0].d = buf;
		dbuf[0].len = ret;
		write(pfd[1], dbuf, sizeof(struct decodebuf));//send buf pointer and size;
		FPS(10)
		close(fd);
	}

}

void *thread_read264(void *arg)
{
        int ret;
        int i, j = 0;
        int start = 0;
        int count = 0;
	struct decodebuf dbuf[BUFN];
	int *pfd = arg;

        int fd = open(H264FILE, O_RDWR);
        unsigned char *_buf = malloc(BUFS*BUFN);
        unsigned char *buf = _buf;
        unsigned char *sbuf = malloc(SBUFSIZE);

        while (1) {
                ret = read(fd, sbuf+4, SBUFSIZE);

                for (i = 0; i < ret; i++) {
                        if (sbuf[i] == 0)
                                if (sbuf[i+1] == 0)
                                        if (sbuf[i+2] == 0)
                                                if (sbuf[i+3] == 1)
                                                        {
                                                                if (start) {

                                                                        //printf("%d bytes:\t", j);
                                                                        //printf("%02x %02x %02x %02x %02x %02x %02x %02x\n", \
                                                                        //        buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
									dbuf[count].d = buf;
									dbuf[count].len = j;
                                                                        write(pfd[1], dbuf+count, sizeof(struct decodebuf));//send buf pointer and size;
                                                                        //start next frame
									count = ((++count)%BUFN);
									buf = _buf+BUFS*(count);
                                                                        j = 0;
                                                                        FPS(10);
                                                                } else {
                                                                        start = 1;
                                                                }
                                                        }

                        buf[j++] = sbuf[i];
                }
                if (ret < SBUFSIZE) {
                        lseek(fd, SEEK_SET, 0);
                        //TODO.dispose last non-complete frame
                        start = 0;
			j = 0;
                        continue;
                }
                memcpy(sbuf, sbuf+SBUFSIZE, 4);
        }

        return 0;
}


