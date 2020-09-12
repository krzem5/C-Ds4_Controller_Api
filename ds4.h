#include <stdint.h>
#include <limits.h>



enum BUTTON{
	BUTTON_UP=0x00001,
	BUTTON_DOWN=0x00002,
	BUTTON_LEFT=0x00004,
	BUTTON_RIGHT=0x00008,
	BUTTON_L1=0x00010,
	BUTTON_R1=0x00020,
	BUTTON_L2=0x00040,
	BUTTON_R2=0x00080,
	BUTTON_L3=0x00100,
	BUTTON_R3=0x00200,
	BUTTON_CROSS=0x00400,
	BUTTON_CIRCLE=0x00800,
	BUTTON_SQURARE=0x01000,
	BUTTON_TRIANGLE=0x02000,
	BUTTON_OPTIONS=0x04000,
	BUTTON_SHARE=0x08000,
	BUTTON_PS=0x10000,
	BUTTON_TOUCHPAD=0x20000
};



struct DS4Device{
	uint32_t b;
	uint8_t l2;
	uint8_t r2;
	int16_t lx;
	int16_t ly;
	int16_t rx;
	int16_t ry;
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
};
