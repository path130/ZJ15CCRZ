#pragma once

void silkenc_uninit(int idec);
int silkenc_init(void);
int silkenc_process(int idec, BYTE* encbuf, int* enclen, short* pcmbuf, int SampleNumber);
int silkdec_init(void);
void silkdec_uninit(int idec);
int silkdec_process(int idec, char* encbuf, int enclen, short* pcmbuf);

