#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "dpplatform.h"
#include "dpgpio.h"

static int fd;
Vdec_Info pinfo;

void preview_start()
{
	int ret = 0;

	if (fd > 0) return;

	fd = open("/dev/encdec", O_RDWR, 0);
	if(fd < 0)
	{
		printf("open /dev/encdec fail\n");
		return ;
	}
	printf("open/dev/encdec ok\n");

	pinfo.preview.m_outtype = DECODE_OUTDIR;//DECODE_OUTYUV;//DECODE_OUTDIR;
	pinfo.preview.m_winw = 320;
	pinfo.preview.m_winh = 240;
	pinfo.preview.property = 0;
	pinfo.preview.m_winl = 0;
	pinfo.preview.m_wint = 0;
	
	ret = ioctl(fd, IOCTL_PREVIEW_START, &pinfo);
	if(ret < 0)
	{
		printf("IOCTL_PREVIEW_START fail\r\n");
		close(fd);
		return ;
	}

	printf("IOCTL_PREVIEW_START OK\r\n");
}

void preview_stop()
{
	if(fd <= 0) return;

	if (0 > ioctl(fd, IOCTL_PREVIEW_STOP, &pinfo)) {
		printf("IOCTL_PREVIEW_START fail\r\n");
		close(fd);
		return;
	}
	close(fd);
	fd = 0;

}


