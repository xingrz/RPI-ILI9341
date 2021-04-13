#include <stdio.h>
#include <stdint.h>

#include "ili9341.h"
#include "swap.h"

int
main(int argc, char const *argv[])
{
	ili9341_init();
	ili9341_clear(0x0000);
	ili9341_bl(true);

	size_t size = 0;
	uint8_t rgb24[ILI9341_WIDTH * ILI9341_HEIGHT * 3];
	uint16_t *rgb565 = (uint16_t *)rgb24;

	while (true) {
		size = fread(rgb24, 1, sizeof(rgb24), stdin);
		if (size != sizeof(rgb24)) break;

		uint16_t p;
		for (int i = 0; i < ILI9341_WIDTH * ILI9341_HEIGHT; i++) {
			p = 0;
			p |= (rgb24[i * 3 + 0] >> 3) << 11;
			p |= (rgb24[i * 3 + 1] >> 2) << 5;
			p |= (rgb24[i * 3 + 2] >> 3) << 0;
			rgb565[i] = SWAP(p);
		}

		ili9341_draw(rgb565, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
	}

	ili9341_deinit();
	return 0;
}
