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
		if (d->btn&BUTTON_PS){
			break;
		}
		d->fr=(d->ry>20?d->ry*2:0);
		d->sr=(d->ly>20?d->ly*2:0);
		DS4_hsl(d,(uint8_t)((double)(t/1000)/20),255,255);
	}
	d->fr=0;
	d->sr=0;
	d->r=0;
	d->g=0;
	d->b=0;
	DS4_update(d);
	DS4_close(d);
	DS4_cleanup();
	return 0;
}
