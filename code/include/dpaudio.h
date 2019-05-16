#pragma once
typedef BOOL (*Audiofun) (DWORD userData, char * pdata, int dlen);

HANDLE AudioPlayCreate(void);
void AudioPlayDestroy(HANDLE);
void AudioPlayStart(HANDLE hAudio, int simplerate, int nchannel);
void AudioPlayStoped(HANDLE hAudio);
void AudioPlaySetVolume(HANDLE hAudio, DWORD Volume);
void AudioPlayAddPlay(HANDLE hAudio, char * lpdata, int dlen);

HANDLE AudioRecCreate(void);
void AudioRecStart(HANDLE hAudio, int simplerate, int nchannel, Audiofun dwInCallback, DWORD userData);
void AudioRecStop(HANDLE hAudio);
void AudioRecDestroy(HANDLE);

void SetSystemVolume(DWORD vol);
void GetSystemVolume(DWORD* vol);

