#ifndef _FRAME_H_
#define _FRAME_H_

#define DEF_FONT_SIZE 15
#define COL_RED 0xff0000
#define COL_GRE 0xff00
#define COL_BLU 0xff
#define COL_WIT 0xffffff

#if 0
#define FRAME_DEBUG(x) frame_draw_text(x, 1, 0xff00, DEF_FONT_SIZE, DEF_FONT_SIZE, 0, DET_HEIGH);
#else
#define FRAME_DEBUG(x)
#endif

void frame_start();
void frame_stop();
void frame_draw_sqar(int col, int row, int width, int height);
void frame_draw_block(int row, int col, int w, int h, unsigned int c);
void frame_draw_text(const char *txt, int utf8, unsigned int color, int font_w, int font_h, int x, int y);


#endif