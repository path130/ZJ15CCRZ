/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: my_types.h
* Author				: Ritchie
* Version				: V1.0.0
* Date					: 2012Äê3ÔÂ07ÈÕ
* Description			: 
* Modify by				: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __MY_TYPES_H
#define __MY_TYPES_H

#include <stdint.h>
#include <types.h>

#if 0 //del by wrm 20150321 for redefinition in compile tool header files (types.h)
#ifndef uint
    typedef unsigned int uint;
#endif

#ifndef vuint
    typedef volatile unsigned int vuint;
#endif
#endif


#if  !(defined FALSE) && !(defined TRUE)
typedef enum{FALSE = 0,TRUE =! FALSE}TRUE_FALSE_DEF;//****opt
#endif

typedef enum{DISABLE = 0,ENABLE =! DISABLE}DIS_EN_DEF;

//#if !(defined FAILED) && !(defined SUCCESS)
#ifdef SUCCESS
#undef SUCCESS
#endif
typedef enum{FAILED = 0,SUCCESS =! FAILED}FS_DEF;
//#endif

typedef enum{OFF, ON=!OFF}ON_OFF_DEF;
typedef enum{LOW, HIGH=!LOW}HIGH_LOW_DEF;
typedef enum{Pin0=0x01,Pin1=0x02,Pin2=0x04,Pin3=0x08,Pin4=0x10,Pin5=0x20,Pin6=0x40,Pin7=0x80}Pins_DEF;
typedef enum{Bit0=0x01,Bit1=0x02,Bit2=0x04,Bit3=0x08,Bit4=0x10,Bit5=0x20,Bit6=0x40,Bit7=0x80}Bits_DEF;


#endif

