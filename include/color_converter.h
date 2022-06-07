#ifndef COLOR_CONVERTER_H
#define COLOR_CONVERTER_H
#include <stdint.h>

typedef struct 
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
}rgb_t;

typedef struct 
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
}hsv_t;

extern rgb_t rgb_color;
extern hsv_t hsv_color;

void rgb2hsv(const rgb_t *rgb, hsv_t *hsv);
void hsv2rgb(const hsv_t *hsv, rgb_t *rgb);
#endif