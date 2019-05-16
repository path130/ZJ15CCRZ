/******************************************************************************************
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^(C) COPYRIGHT 2011 AnJuBao^^^^^^^^^^^^^^^^^^^^^^^^^^^
* File Name 			: debug.h
* Author				: Ritchie
* Version				: V1.0.0
* Date					: 2012年7月03日
* Description			: 
* Modify by				: 
* Modify date			: 
* Modify description	: 
******************************************************************************************/
#ifndef __MY_DEBUG_H
#define __MY_DEBUG_H

#include <time.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define	ANSI_COLORS
//#define	HTML_COLORS
/*-------------------------------------------------------*/
#ifdef ANSI_COLORS
#define GRAY "\033[2;37m"
#define GREEN "\033[0;32m"
#define DARKGRAY "\033[0;30m"
//#define BLACK "\033[0;39m"
#define BLACK "\033[2;30m" // not faint black
//#define NOCOLOR "\033[0;39m"
#define NOCOLOR "\033[0m"
#define DBLUE "\033[2;34m"
#define RED "\033[0;31m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define CLS "\014"
#define NEWLINE "\r\n"
#define CR "\r"

#elif (defined HTML_COLORS)

#define GRAY "<font color=gray>"
#define GREEN "<font color=green>"
#define DARKGRAY "<font color=darkgray>"
#define BLACK "\033[2;30m" // not faint black
//#define NOCOLOR "\033[0;39m"
#define NOCOLOR "</font></font></u></b>"
#define DBLUE "<font color=blue>"
#define RED "<font color=red>"
#define BOLD "<b>"
#define UNDERLINE "<u>"
#define CLS "\014"
#define NEWLINE "<br>\r\n"
#define CR "<br>\r\n"
#else
#define GRAY 
#define BLACK
#define NOCOLOR
#define DBLUE
#define RED
#endif

/*-------------------------------------------------------*/

#if 0//ndef DEBUG_NONE
	#define	DEBUG_NONE
#endif
#undef DEBUG_FILE

#ifndef DEBUG_NONE

#ifdef  DEBUG_FILE
#define DEBUG_PATH "./debug.txt"

#define my_debug(fmt, ...)  {\
        time_t now; \
        now = time(0); \
        struct tm *tnow = localtime(&now); \
        FILE *fp_debug = fopen(DEBUG_PATH, "at"); \
        fprintf(fp_debug, "%d/%d/%d %02d:%02d:%02d : ", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min, tnow->tm_sec); \
        fprintf(fp_debug, "[%s] : {%s} : (%d)\n"fmt"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
        fclose(fp_debug); }
    
#else
      #define my_debug(fmt, ...)              fprintf(stderr, fmt, ##__VA_ARGS__)
    
    //#define my_debug(fmt, ...)               fprintf(stderr, "[%s] : {%s} : (%d)\t"fmt"\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#endif

    #define debug_print(format, arg...)     my_debug(format, ##arg)

	#define debug_red(format, arg...)		debug_print(RED BOLD format NOCOLOR, ##arg)
	#define debug_blue(format, arg...)		debug_print(DBLUE format NOCOLOR, ##arg)

	#define dbg_print(format, arg...) 		my_debug(format, ##arg)
	#define dbg_red(format, arg...)			debug_print(RED BOLD format NOCOLOR, ##arg)
	#define dbg_blue(format, arg...)		debug_print(DBLUE format NOCOLOR, ##arg)

	#define dbg_green(format, arg...)		debug_print(""GREEN"info: "format NOCOLOR, ##arg)

#ifndef DEBUG_ERROR
	#define debug_info(format, arg...)      debug_print(""GREEN"info: "format NOCOLOR, ##arg)
	#define debug_warn(format, arg...)		debug_print("\n"DBLUE"warn: [%s_%d] "format NOCOLOR"\n", __FILE__, __LINE__, ##arg)
	#define dbg_inf(format, arg...)			debug_print(""GREEN"info: "format NOCOLOR, ##arg)
	#define dbg_warn(format, arg...)		debug_print("\n"DBLUE BOLD"warn: [%s_%d] "format NOCOLOR"\n", __FILE__, __LINE__, ##arg)
#else
    #define debug_info(format, arg...)
	#define debug_warn(format, arg...)
	#define dbg_inf(format, arg...)
	#define dbg_warn(format, arg...)
#endif

	#define debug_err(format, arg...)       debug_print("\n"RED BOLD"err: [%s_%d] "format NOCOLOR, __FILE__, __LINE__,##arg)
	#define dbg_err(format, arg...)			debug_print("\n"RED BOLD"err: [%s_%d] "format NOCOLOR, __FILE__, __LINE__,##arg)
	#define	dbg_lo(format, arg...)			debug_print(DBLUE BOLD"[%s_%d] "format NOCOLOR"\n", __FILE__, __LINE__,##arg)
    #define dbg_perror(format, arg...) 		debug_print("\n"RED BOLD"err: [%s_%d]:"format "%s\n" NOCOLOR, __FILE__, __LINE__, ##arg, strerror(errno))
#else// no debug
    #define debug_print(fmt, ...)
    #define debug_info(format, arg...)
	#define debug_warn(format, arg...)
    #define debug_err(format, arg...)
	#define	my_debug(format, arg...)		
	#define dbg_print(fmt, ...)
	#define dbg_inf(format, arg...)
	#define dbg_warn(format, arg...)
	#define dbg_err(format, arg...)
	#define	dbg_lo(format, arg...)
#endif

#ifndef MTOSTR
#define	MTOSTR(S)		#S
#endif

#ifndef assert_ptr
	#ifndef DEBUG_NONE
		#define	assert_ptr(x) ((NULL==(x))&&(debug_err("%s is null\n", MTOSTR(x))))
	#else
		//#define	assert_ptr(x) ((NULL==(x))&&(my_debug("%s is null\n", MTOSTR(x))))
		#define	assert_ptr(x) ((NULL==(x))&&(fprintf(stderr, "%s is null\n", MTOSTR(x))))		
	#endif
#endif

#if defined (__cplusplus)
}
#endif

#endif
