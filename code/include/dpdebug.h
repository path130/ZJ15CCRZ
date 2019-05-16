#pragma once

#define	DISP_MOD	0
#define	DB_MOD		1
#define	SDA_MOD		2
#define	APP_MOD		3
#define	SIP_MOD		4
#define	DEV_MOD		5
#define	PORT_MOD	6
#define	SRV_MOD		7
#define	SAFE_MOD	8
#define	SND_MOD		9		// ÉùÒôÄ£¿é

#define	DPINFO		29
#define	DPWARNING	30
#define	DPERROR		31

void InitDebugLeven(DWORD level);
int DBGMSG(DWORD level, const char * format, ...);

