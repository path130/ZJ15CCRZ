#ifndef _VCODEC_LIB_H_
#define _VCODEC_LIB_H_

#define	DECODE_MP4		0x1
#define	DECODE_H263		0x2
#define	DECODE_H264		0x3
#define	DECODE_JPEG		0x4
#define	DECODE_PNG		0x5

#define	DECODE_OUTDIR	0x01
#define	DECODE_OUTRGB	0x02
#define	DECODE_OUTYUV	0x03
#define	DECODE_OUTMEM	0x04

#define	ENCODE_MP4		0x01
#define	ENCODE_JPEG		0x02
#define	ENCODE_H264		0x03

#define	ENCODE_BLANK_ON		0x01
#define	ENCODE_BLANK_OFF	0x02
#define	ENCODE_OSD_ON		0x04
#define	ENCODE_OSD_OFF		0x08
#define	ENCODE_QUALITY		0x10
#define	ENCODE_SKIP			0x20
#define	ENCODE_REALTIME		0x40
#define	ENCODE_MAXKEY		0x80
#define	ENCODE_USESBUF		0x100
#define	ENCODE_SET_SCALER	0x2000
#define ENCODE_CHAR_MAP		0x4000

#define	ENCODE_IFRAME		0x01

#define	PREV_USESBUF		0x100
#define	ENCODE_VFUREQUEST   0x1000


#define DECODE_SHOW_LAYER      0x01
#define	DECODE_HIDE_LAYER		0x02

enum
{
	ENC_ERR_MODE	= 0x10000,		// Not work in ENC mode
	ENC_ERR_TMOUT	= 0x10001,		// Wait new frame timeout
	ENC_ERR_BUFSHORT= 0x10002,		// Enc buffer length short
	ENC_ERR_BUFNULL	= 0x10003,		// Enc buffer is NULL
} ;

typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;

typedef struct
{
	DWORD charnum;
	DWORD height;
	BYTE* width;
	BYTE* pixmap;
} CharMap;



typedef struct
{
	DWORD m_outtype;	// 
	DWORD m_winl;	// display window left
	DWORD m_wint;	// display window top
	DWORD m_winw;	// display window width
	DWORD m_winh;	// display window hight
	DWORD property;
} Preview_Info;

typedef struct
{
	DWORD m_dectype;
	DWORD m_outtype;	// 
	DWORD m_winl;		// display window left
	DWORD m_wint;		// display window top
	DWORD m_winw;		// display window width
	DWORD m_winh;		// display window hight
	DWORD m_decw;		//
	DWORD m_dech;		//
	DWORD m_property;	
} Decode_Info;

typedef struct
{
	DWORD m_enctype;
	DWORD m_encw;	// camera resolution width
	DWORD m_ench;	// camera resolution hight
	DWORD property;
	WORD  osd_x;		// osd left
	WORD  osd_y;		// osd top
	DWORD m_quality;	// 1~16
	DWORD m_bcolor;		// when blanc, filled color
	DWORD m_skiptime;
	DWORD m_maxkey;
	DWORD m_scalerinfo;
	DWORD value[100];
} Encode_Info;
typedef struct
{
	DWORD m_enctype;
	DWORD m_encw;	// camera resolution width
	DWORD m_ench;	// camera resolution hight
	DWORD property;
	WORD  osd_x;		// osd left
	WORD  osd_y;		// osd top
	DWORD m_quality;	// 1~16
	DWORD m_bcolor;		// when blanc, filled color
	DWORD m_skiptime;
	DWORD m_maxkey;
	DWORD m_scalerinfo;
	DWORD sensor_w;
	DWORD sensor_h;
	BYTE* m_framey;
	BYTE* m_frameuv;
	BYTE* buf;
	DWORD dwsize;
} Encode_Frame;

typedef struct
{
	BYTE* buf;
	DWORD dwsize;
	DWORD timeout;
	DWORD dec_pts;
} WRITE_INFO;

enum
{
	I_VOP = 0,
	P_VOP = 1
};

typedef struct
{
	BYTE* buf;
	DWORD dwsize;
	DWORD timeout;
	DWORD property;
} READ_INFO;

typedef struct
{
	WRITE_INFO w;
	READ_INFO r;
} WR_INFO;

typedef enum
{
	SIF_BRIGHTNESS	= 0x01,		// Value from 0 ~ 0xff
	SIF_HUE			= 0x02,		// Value from 0 ~ 0xff
	SIF_CONTRAST	= 0x04,		// Value from 0 ~ 0xff
	SIF_SATURATION	= 0x08,		// Value from 0 ~ 0xff
	SIF_CHANGEIN	= 0x10,		// Value from 1 ~ 3
} Sif_Support;

#define	YUV422		1	// jpeg encode
#define	YUV420		2	// mp4 encode

typedef enum
{
	SIF_GETBRIGHTNESS	= 0x01,
	SIF_SETBRIGHTNESS	= 0x02,
	SIF_GETHUE			= 0x03,
	SIF_SETHUE			= 0x04,
	SIF_GETCONTRAST		= 0x05,
	SIF_SETCONTRAST		= 0x06,
	SIF_GETSATURATION	= 0x07,
	SIF_SETSATURATION	= 0x08,
	SIF_GETOUTSIZE 		= 0x09,
	SIF_SETOUTSIZE 		= 0x0A,
	SIF_GETOUTFMT		= 0x0B,
	SIF_SETOUTFMT		= 0x0C,
	SIF_GETWINSIZE		= 0x0D,
	SIF_SETWINSIZE		= 0x0E,
	SIF_GETCHANGEIN 	= 0x0F,
	SIF_SETCHANGEIN 	= 0x10,
	SIF_SETSINGLEBUF	= 0x11,
	SIF_GETREGVAL		= 0x12,
	SIF_SETREGVAL		= 0x13
} Sif_Cmd;

typedef struct 
{
	WORD	Width;
	WORD	Height;
} SIF_SIZE;

typedef struct 
{
	DWORD		Supported;
	SIF_SIZE 	OutSize[6];
} Sensor_Prop;

typedef struct 
{
	DWORD		val;
	DWORD		SifProperty;
}Sensor_Set;

typedef struct
{
	Decode_Info		decode;
	Encode_Info		encode;
}ReEncode_Info;

typedef struct
{
	DWORD len;
	char*buf;
}OSD_String;

typedef union
{
	Preview_Info	preview;
	Decode_Info		decode;
	Encode_Info		encode;
	ReEncode_Info   reencode;
	Encode_Frame	eframe;
	READ_INFO		rbuf;
	WRITE_INFO		wbuf;
	WR_INFO			wrbuf;
	Sensor_Prop		spop;
	Sensor_Set		sset;
} Vdec_Info;



#define CTL_CODE(DeviceType, Function, Method, Access) (     \
  ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)    \
)

#define IOCTL_CAMERA_BASE 			0x200
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0

#define	IOCTL_PREVIEW_START			CTL_CODE(IOCTL_CAMERA_BASE, 0x220, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Preview_Info
#define	IOCTL_PREVIEW_STOP			CTL_CODE(IOCTL_CAMERA_BASE, 0x221, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_ENCODE_START			CTL_CODE(IOCTL_CAMERA_BASE, 0x222, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_ENCODE_STOP			CTL_CODE(IOCTL_CAMERA_BASE, 0x223, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_DECODE_START			CTL_CODE(IOCTL_CAMERA_BASE, 0x224, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Decode_Info
#define	IOCTL_DECODE_STOP			CTL_CODE(IOCTL_CAMERA_BASE, 0x225, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_READ_BUF				CTL_CODE(IOCTL_CAMERA_BASE, 0x226, METHOD_BUFFERED, FILE_ANY_ACCESS)	// READ_INFO
#define	IOCTL_WRITE_BUF				CTL_CODE(IOCTL_CAMERA_BASE, 0x227, METHOD_BUFFERED, FILE_ANY_ACCESS)	// WRITE_INFO
#define	IOCTL_SIF_GETCAP			CTL_CODE(IOCTL_CAMERA_BASE, 0x228, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Sensor_Prop
#define	IOCTL_SIF_CONTROL			CTL_CODE(IOCTL_CAMERA_BASE, 0x229, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Sensor_Set
#define	IOCTL_PREVIEW_SET			CTL_CODE(IOCTL_CAMERA_BASE, 0x22a, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Preview_Info
#define	IOCTL_ENCODE_SET			CTL_CODE(IOCTL_CAMERA_BASE, 0x22b, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_DECODE_SET			CTL_CODE(IOCTL_CAMERA_BASE, 0x22c, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Decode_Info
#define	IOCTL_ENCODE_OSD			CTL_CODE(IOCTL_CAMERA_BASE, 0x22d, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_ENCODE_FRAME			CTL_CODE(IOCTL_CAMERA_BASE, 0x22e, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define IOCTL_GET_CUSTOM_INFO		CTL_CODE(IOCTL_CAMERA_BASE, 0x22f, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_REENCODE_STOP			CTL_CODE(IOCTL_CAMERA_BASE, 0x230, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_REENCODE_RW			CTL_CODE(IOCTL_CAMERA_BASE, 0x231, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_REENCODE_START		CTL_CODE(IOCTL_CAMERA_BASE, 0x232, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_REENCODE_READ			CTL_CODE(IOCTL_CAMERA_BASE, 0x233, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_ENCODE_FRAME_START	CTL_CODE(IOCTL_CAMERA_BASE, 0x234, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_ENCODE_FRAME_STOP		CTL_CODE(IOCTL_CAMERA_BASE, 0x235, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info
#define	IOCTL_ENCODE_WRITE_AND_READ_FRAME	CTL_CODE(IOCTL_CAMERA_BASE, 0x236, METHOD_BUFFERED, FILE_ANY_ACCESS)	// Encode_Info

#define	IOCTL_GRAPHIC_SNR			0x201
#define	IOCTL_SENSOR_START			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x220, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_STOP		CTL_CODE(IOCTL_GRAPHIC_SNR, 0x221, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_CTRL			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x222, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_READ			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x223, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

