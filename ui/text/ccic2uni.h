#ifndef _CCIC2UNI_H_
#define _CCIC2UNI_H_

#if defined (__cplusplus)
extern "C" {
#endif

extern unsigned int   is_cn_char(const char *txt);
extern unsigned short ccic_trans_unicode(const char *txt);

#if defined (__cplusplus)
}
#endif

#endif
