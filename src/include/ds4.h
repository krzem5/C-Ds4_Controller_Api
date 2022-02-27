#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <signal.h>



#define bool _Bool
#define false 0
#define true 1
#ifdef NULL
#undef NULL
#endif
#define NULL ((void*)0)
#define assert(x) \
	do{ \
		if (!(x)){ \
			printf("%s:%i (%s): %s: Assertion Failed\n",__FILE__,__LINE__,__func__,#x); \
			raise(SIGABRT); \
		} \
	}while (0)



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
	uint8_t fc;
	uint32_t dt;
	uint32_t btn;
	uint8_t l2;
	uint8_t r2;
	int8_t lx;
	int8_t ly;
	int8_t rx;
	int8_t ry;
	uint8_t bt;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t fr;
	uint8_t sr;
	uint8_t fon;
	uint8_t foff;
	HANDLE _fh;
	unsigned char* _ib;
	unsigned char* _ob;
	uint8_t _f;
	uint64_t _uc;
	OVERLAPPED _o;
	LARGE_INTEGER _lt;
	LARGE_INTEGER _tf;
};



struct DS4DeviceList{
	char* p;
	struct DS4DeviceList* n;
};



void DS4_init(void);



void DS4_cleanup(void);



struct DS4DeviceList* DS4_find_all(void);



void DS4_free_list(struct DS4DeviceList* l);



struct DS4Device* DS4_connect(char* p);



void DS4_update(struct DS4Device* d);



void DS4_hsl(struct DS4Device* d,uint8_t h,uint8_t s,uint8_t l);



void DS4_close(struct DS4Device* d);
