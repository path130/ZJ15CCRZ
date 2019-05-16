
#if defined (__cplusplus)
extern "C" {
#endif

void face_load(char *model);
int face_detect(int height, int width, unsigned char *ybuf, int pos[], int len);

#if defined (__cplusplus)
}
#endif
