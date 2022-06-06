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

#endif