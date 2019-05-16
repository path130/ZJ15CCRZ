#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>


#include "NPDScan_Face_Det.h"

static NPDScan_Face_Det inst_face_det;

#if defined (__cplusplus)
extern "C" {
#endif
void face_load(char *model)
{
	//inst_face_det.model_loader("model_frontal.bin");
	inst_face_det.model_loader(model);

}

int face_detect(int height, int width, unsigned char *ybuf, int pos[], int len)
{
vector<square_box> faces;


	int i;
	faces = inst_face_det.DetectFace(height, width, ybuf, 1, 50, 200);
	if(faces.size() >0)
	{
//printf("faces:%d\n", faces.size());
		for (i = 0; i < faces.size() && (i+1)*4 <= len; i++) {
			//printf("\t x:%d,y:%d,w:%d h:%d\n",faces[i].col,faces[i].row,faces[i].width,faces[i].height);	
			pos[i*4] = faces[i].col;
			pos[i*4+1] = faces[i].row;
			pos[i*4+2] = faces[i].width;
			pos[i*4+3] = faces[i].height;

		}
	} else {
		return 0;
	}

	return i;
}


#if defined (__cplusplus)
}
#endif
