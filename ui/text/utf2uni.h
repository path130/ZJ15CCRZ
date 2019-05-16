#ifndef _UTF2UNI_H_
#define _UTF2UNI_H_

#if defined (__cplusplus)
extern "C" {
#endif

extern int get_utf8_bytes(const unsigned char *txt);
extern unsigned short utf8_trans_unicode(const char *txt, int bytes);

#if defined (__cplusplus)
}
#endif

#endif
