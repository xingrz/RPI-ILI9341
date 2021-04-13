#include <stdio.h>

#include "ili9341.h"
#include <unistd.h>

int
main(int argc, char const *argv[])
{
	ili9341_init();
	ili9341_clear(0x001F);
	ili9341_bl(true);
	sleep(1);
	ili9341_fill(0xF800, 60, 80, 120, 160);
	ili9341_deinit();
	return 0;
}
