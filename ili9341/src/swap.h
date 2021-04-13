#ifndef __ILI9341_SWAP__
#define __ILI9341_SWAP__

#define SWAP(n) (((n >> 8) & 0xFF) | ((n & 0xFF) << 8))

#endif  // __ILI9341_SWAP__
