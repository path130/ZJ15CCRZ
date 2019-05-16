#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
//#include "JpegCodec.h"

int JpegDecoderRGB(char *filename, int *width, int *height, char **outbuf)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	FILE *infile;
	char *inptr, *outptr;
	int  i;
	char* poutbuf;

	//printf("decjpg %s\r\n", filename);
	infile = fopen (filename, "rb");
	if (infile==NULL)
		return 0;

	// Step 1: allocate and initialize JPEG decompression object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Step 2: specify data source (eg, a file)
	jpeg_stdio_src(&cinfo, infile);

	// Step 3: read file parameters
	i = jpeg_read_header(&cinfo, TRUE);
	if (i != JPEG_HEADER_OK)
	{
		jpeg_destroy_decompress(&cinfo);
		fclose (infile);
		return 0;
	}

	// Step 4: set parameters for decompression
	cinfo.out_color_space = JCS_RGB;

	// Step 5: start decompressor
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	poutbuf = (char*)malloc(cinfo.output_height * row_stride);
	*outbuf = poutbuf;
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// Step 6: process data
	row_stride = (row_stride + 3) & 0xfffffffc;
//	outbuf += (cinfo.output_height - 1) * row_stride;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		inptr  = (char*)(*buffer);
		outptr = poutbuf;
		for (i=0; i<cinfo.output_width; i++)
		{
			outptr[0] = inptr[0];
			outptr[1] = inptr[1];
			outptr[2] = inptr[2];
			inptr  += 3;
			outptr += 3; 
		}
		poutbuf += row_stride;
	}

	// Step 7: finish decompression
	(void) jpeg_finish_decompress(&cinfo);

	// Step 8: release JPEG decompression object
	jpeg_destroy_decompress(&cinfo);

	fclose (infile);
	*width = cinfo.output_width;
	*height = cinfo.output_height;
	return 1;
}


