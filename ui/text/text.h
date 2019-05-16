
#ifndef _TEXT_H_
#define _TEXT_H_

#if defined (__cplusplus)
extern "C" {
#endif


#define LINE_GAP_PIXEL              4
#define TEXT_LOAD_INDEX_ADVANCE

typedef struct font_object_tag {
    char *library;
    char *face;
} font_object, *font_handle;

extern font_handle text_font;
extern font_handle text_font_load(const char *file);
extern int text_font_free(font_handle h_font);
extern unsigned int text_fill_canvas(font_handle h_font, int utf8, unsigned int color, int font_w, int font_h, int area_w, int area_h, int x, int y, const char *txt);
extern unsigned int text_show(font_handle h_font, int utf8, unsigned int color, int font_w, int font_h, int area_w, int area_h, int x, int y, const char *txt);
extern unsigned int text_grays2buf(font_handle h_font, unsigned int utf8, unsigned char *buf, int font_w, int font_h, int area_w, int area_h, int x, int y, const char *txt);

#if defined (__cplusplus)
}
#endif

#endif
