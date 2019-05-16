#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_SPR_BASE 			0x201
#define METHOD_BUFFERED			0
#define FILE_ANY_ACCESS			0
#define IOCTL_SET_BKGRD			CTL_CODE(IOCTL_SPR_BASE, 0x268, METHOD_BUFFERED, FILE_ANY_ACCESS)	



int main()
{
	int fd = open("/dev/frame", O_RDWR);
	if (fd < 0) perror("open frame");

	unsigned int bg = 0xC2BCBE;

	ioctl(fd, IOCTL_SET_BKGRD, &bg);



	return 0;
}
