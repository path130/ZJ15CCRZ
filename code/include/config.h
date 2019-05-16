#pragma once

#define SCREEN_CALIBRATE

#ifdef DPCE
#define SDCARD			"/CardA"
#endif

#ifdef DPLINUX
#define SDCARD			"/sdcard"
#endif

#define FRAME_WIDTH				800
#define FRAME_HEIGHT			480

#define V_NORMAL_LEFT			134
#define V_NORMAL_TOP			80
#define V_NORMAL_WIDTH			532
#define V_NORMAL_HEIGHT			320

#define SELECT_LEFT				55
#define SELECT_LEFT2			165
#define SELECT_TOP				95
#define SELECT_HEIGHT			45

#define USERDIR					"/UserDev"
#define FLASHDIR				"/FlashDev"
#define WINDOWSDIR				"/Windows"
#define LAYOUT_DIR				"/FlashDev/Layout"

#define DOWNLOAD_FILE_PATH		"/Windows/temp.dat"
#define	DOWNLOAD_IMAGE_PATH		"/Windows/Image.dd"
#define	IPTABLE_NAME			"NetCfg.dat"
#define	IPTABLE_BAK				"NetCfg.bak"
#define	TEMP_MSG_PATH 			"/Windows/msg.tmp"
#define LIUYAN_FILE_PATH		"/Windows/liuyan.wav"

#define IMAGE_OBJECT_SIZE		(6 * 1024 * 1024)	// 6M
#define NORMAL_OBJECT_SIZE		(2 * 1024 * 1024)	// 6M

#define WARNINGINFO				"============================>>>>>>>>>>>>>>>>>>>>>"


/************************************************************************/
/*                                                                      */
/************************************************************************/
#define	SAFE_MAX_NUMBER			8

#define SAREA_KITCHEN			0	// ����
#define SAREA_BEDROOM			1	// ����
#define SAREA_HALL				2	// ����	
#define SAREA_WINDOW			3	// ����
#define SAREA_DOOR				4	// ����
#define SAREA_BALCONY			5	// ��̨
#define SAREA_GUEST				6	// �ͷ�	
#define SAREA_MAX				7	
#define SAREA_LOCALHOST			89	// ���ڻ�

#define STYPE_URGENT			0	// ����
#define STYPE_FIRE				1	// �̸�
#define STYPE_GAS				2	// ú��	
#define STYPE_MAGNETIC			3	// �Ŵ�
#define STYPE_IR				4	// ����
#define STYPE_WINDOW			5	// ����
#define STYPE_CLASS				6	// ����	
#define STYPE_MAX				7	

#define STYPE_HOSTAGE	        19	// Ю��
#define STYPE_TAMPER			20	// ����
#define STYPE_VOLTAGE			21	// ��ѹ

#define SSTATUS_CANCEL_DEFENSE		0	//δ����
#define SSTATUS_SET_DEFENSE			1	//�Ѳ���
#define SSTATUS_ALARM_DELAY			2	//������ʱ��	
#define SSTATUS_ALARMING			3	//������

#define AREAMODE_HOME		0
#define AREAMODE_LEAVE		1
#define AREAMODE_MAX		2

#define	SMODE_TEST		0		// ����ģʽ
#define	SMODE_UNSET		1		// ����ģʽ
#define	SMODE_INHOME	2		// �ڼ�ģʽ
#define	SMODE_NIGHT		3		// ҹ��ģʽ
#define	SMODE_LEAVE		4		// ���ģʽ
#define	SMODE_MAX		5		//

#define	SLEVEL_LOW		0		// �͵�ƽ
#define	SLEVEL_HIGH		1		// �ߵ�ƽ

#define	TANTOU_IDLE		0		// ����
#define	TANTOU_DELAY	1		// ������ʱ
#define	TANTOU_ALARM	2		// ������
#define	TANTOU_DISALARM	3		// ������������

	