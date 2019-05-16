#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "roomlib.h"
#include <linux/input.h>

static void TimeEvent(int signo)
{
	DPPostMessage(TIME_MESSAGE, 0, 0, 0, MSG_TIME_TYPE);
}

static void SignalProc(int signo)
{
	printf("**************************************************************\r\n");
	printf("************************SIGNAL:%d**************************\r\n", signo);
	printf("**************************************************************\r\n");
}

void DPCreateTimeEvent(void)
{
	for(int i = 0; i < 64; i++)
	{
		if(i != SIGUSR1)
		{
			signal(i + 1, SignalProc);
		}
	}

	struct sigevent evp;
	struct itimerspec ts;
	timer_t timer;
	int ret;
	evp.sigev_value.sival_ptr = &timer;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	signal(SIGUSR1, TimeEvent);
	ret = timer_create(CLOCK_REALTIME, &evp, &timer);
	if (ret)
		perror("DPCreateTimeEvent fail\r\n");
	else
	{
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		ret = timer_settime(timer, 0, &ts, NULL);
		if (ret)
			perror("timer_settime");
	}
}

static void* TouchEvent(void* pParam)
{
	int fd;
	static int xydata[2];
	int flag = 0;
	struct input_event buf;
	static BOOL curdown = FALSE;

	fd = open("/dev/input/event1", O_RDONLY);
	if (fd < 0) {
		printf("open rtp event fail!\n");
		return 0;
	}
	int ret = read(fd, &buf, sizeof(struct input_event));
	while (ret)
	{
		if (buf.type == EV_REL)
		{
			if (buf.code == REL_X)
			{
				xydata[0] = buf.value;
			}
			if (buf.code == REL_Y)
			{
				xydata[1] = buf.value;
			}
		}

		if (buf.code == BTN_TOUCH)
		{
			if (buf.value == 0)
			{
				flag = 0;
			}
			else
			{
				flag = 1;
			}
		}

		if (buf.type == EV_SYN)
		{
			if (flag)
			{
				if (curdown)
					DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_VALID, MSG_TOUCH_TYPE);
				else
				{
					DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_DOWN, MSG_TOUCH_TYPE);
					curdown = TRUE;
				}
			}
			else
			{
				DPPostMessage(TOUCH_RAW_MESSAGE, xydata[0], xydata[1], TOUCH_UP, MSG_TOUCH_TYPE);
				curdown = FALSE;
			}
		}
		ret = read(fd, &buf, sizeof(struct input_event));
	}
	close(fd);
	return NULL;
}

void DPCreateTouchEvent()
{
	pthread_t pid0;
	pthread_create(&pid0, NULL, TouchEvent, NULL);
}

