#include <ds4.h>
#include <stdio.h>



int main(int argc,const char** argv){
	DS4_init();
	struct DS4DeviceList* l=DS4_find_all();
	struct DS4Device* d=DS4_connect(l->p);
	DS4_free_list(l);
	while (true){
		static uint64_t t=0;
		DS4_update(d);
		t+=d->dt;
		printf("BUTTONS: %x\n",d->btn);
		DS4_hsl(d,(uint8_t)((double)(t/1000)/1000*255),255,255);
	}
	DS4_close(d);
	DS4_cleanup();
	return 0;
}
