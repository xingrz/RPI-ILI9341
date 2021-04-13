#include <unistd.h>
#include <bcm2835.h>

#include "ili9341.h"
#include "pinout.h"

#define msleep(ms) usleep(ms * 1000)

#define COUNT(x) (sizeof(x) / sizeof(x[0]))
#define SWAP(n) (((n >> 8) & 0xFF) | ((n & 0xFF) << 8))

#define TRACE 0
#define TRACE_DATA 0

#if TRACE
#include <stdio.h>
#endif

static struct {
	uint8_t cmd;
	uint8_t len;
	uint8_t data[16];
} ili9341_reg_conf[] = {
		{0xCF, 3, {0x00, 0xC1, 0X30}},
		{0xED, 4, {0x64, 0x03, 0X12, 0X81}},
		{0xE8, 3, {0x85, 0x00, 0x79}},
		{0xCB, 5, {0x39, 0x2C, 0x00, 0x34, 0x02}},
		{0xF7, 1, {0x20}},
		{0xEA, 2, {0x00, 0x00}},

		// Power control
		{0xC0, 1, {0x1D}},  // VRH[5:0]
		{0xC1, 1, {0x12}},  // SAP[2:0];BT[3:0]

		// VCM control
		{0xC5, 2, {0x33, 0x3F}},
		{0xC7, 1, {0x92}},

		// Memory Access Control
		{0x3A, 1, {0x55}},
		{0x36, 1, {0x08}},

		{0xB1, 2, {0x00, 0x12}},

		// Display Function Control
		{0xB6, 2, {0x0A, 0xA2}},

		{0x44, 1, {0x02}},

		// 3Gamma Function Disable
		{0xF2, 1, {0x00}},

		// Gamma curve selected
		{0x26, 1, {0x01}},

		// Set Gamma
		{0xE0, 15,
				{0x0F, 0x22, 0x1C, 0x1B, 0x08, 0x0F, 0x48, 0xB8, 0x34, 0x05, 0x0C, 0x09, 0x0F, 0x07,
						0x00}},
		{0XE1, 15,
				{0x00, 0x23, 0x24, 0x07, 0x10, 0x07, 0x38, 0x47, 0x4B, 0x0A, 0x13, 0x06, 0x30, 0x38,
						0x0F}},

		// Display on
		{0x29, 0, {}},
};

static void
ili9341_write_cmd(uint8_t cmd)
{
	bcm2835_gpio_write(PIN_LCD_DC, LOW);
	bcm2835_spi_writenb(&cmd, sizeof(uint8_t));

#if TRACE
	printf("CMD %02x\n", cmd);
#endif
}

static void
ili9341_write_data(void *data, uint32_t len)
{
	bcm2835_gpio_write(PIN_LCD_DC, HIGH);
	bcm2835_spi_writenb((const char *)data, len);

#if TRACE_DATA
	printf("DATA (%d):", len);
	for (int i = 0; i < len; i++) {
		printf(" %02x", ((uint8_t *)data)[i]);
	}
	printf("\n");
#endif
}

static void
ili9341_write_byte(uint8_t data)
{
	ili9341_write_data(&data, sizeof(uint8_t));
}

void
ili9341_init(void)
{
	// Set up pins
	bcm2835_init();

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);

	bcm2835_gpio_fsel(PIN_LCD_DC, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_LCD_RST, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_LCD_BL, BCM2835_GPIO_FSEL_OUTP);

	// Backlight off
	ili9341_bl(false);

	// Reset panel
	bcm2835_gpio_write(PIN_LCD_RST, HIGH);
	msleep(100);
	bcm2835_gpio_write(PIN_LCD_RST, LOW);
	msleep(100);
	bcm2835_gpio_write(PIN_LCD_RST, HIGH);
	msleep(100);

	// Init panel
	ili9341_write_cmd(0x11);  // sleep out
	msleep(120);

	for (int i = 0; i < COUNT(ili9341_reg_conf); i++) {
		ili9341_write_cmd(ili9341_reg_conf[i].cmd);
		if (ili9341_reg_conf[i].len > 0) {
			ili9341_write_data(ili9341_reg_conf[i].data, ili9341_reg_conf[i].len);
		}
	}
}

void
ili9341_deinit(void)
{
	bcm2835_spi_end();
	bcm2835_close();
}

void
ili9341_bl(bool on)
{
	if (on) {
		bcm2835_gpio_write(PIN_LCD_BL, HIGH);
	} else {
		bcm2835_gpio_write(PIN_LCD_BL, LOW);
	}
}

static void
ili9341_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	struct {
		uint16_t b;
		uint16_t e;
	} axis;

	axis.b = SWAP(x1);
	axis.e = SWAP(x2);
	ili9341_write_cmd(0x2A);
	ili9341_write_data(&axis, sizeof(axis));

	axis.b = SWAP(y1);
	axis.e = SWAP(y2);
	ili9341_write_cmd(0x2B);
	ili9341_write_data(&axis, sizeof(axis));
}

void
ili9341_clear(uint16_t color)
{
	ili9341_fill(color, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
}

void
ili9341_fill(uint16_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	ili9341_set_window(x, y, x + w - 1, y + h - 1);

	uint16_t px[w * h];
	for (int i = 0; i < w * h; i++) {
		px[i] = SWAP(color);
	}

	ili9341_write_cmd(0x2C);
	ili9341_write_data(px, sizeof(px));
}
