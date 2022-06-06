#ifndef PWN_MODUL_H
#define PWN_MODUL_H

void pwm_rgb_init(void);
void rgb_on(uint32_t red, uint32_t green, uint32_t blue);
void init_flash_rgb();

typedef struct 
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
}rgb_t;

#endif