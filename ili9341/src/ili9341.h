#ifndef __ILI9341_ILI9341__
#define __ILI9341_ILI9341__

#include <stdbool.h>
#include <stdint.h>

#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

void ili9341_init(void);
void ili9341_deinit(void);

void ili9341_bl(bool on);

void ili9341_clear(uint16_t color);
void ili9341_fill(uint16_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#endif  // __ILI9341_ILI9341__
