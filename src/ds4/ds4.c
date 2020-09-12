#include <ds4.h>
#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib,"Setupapi.lib")



struct _HIDD_ATTRIBUTES{
	ULONG Size;
	uint16_t VendorID;
	uint16_t ProductID;
	uint16_t VersionNumber;
};
struct _HIDP_CAPS{
	uint16_t Usage;
	uint16_t UsagePage;
	uint16_t InputReportByteLength;
	uint16_t OutputReportByteLength;
	uint16_t FeatureReportByteLength;
	uint16_t Reserved[17];
	uint16_t fields_not_used_by_hidapi[10];
};
typedef int (__stdcall *HidD_GetAttributes_)(HANDLE d,struct _HIDD_ATTRIBUTES* a);
typedef int (__stdcall *HidD_GetSerialNumberString_)(HANDLE d,void* bf,unsigned long bfl);
typedef int (__stdcall *HidD_GetManufacturerString_)(HANDLE h,void* bf,unsigned long bfl);
typedef int (__stdcall *HidD_GetProductString_)(HANDLE h,void* bf,unsigned long bfl);
typedef int (__stdcall *HidD_SetFeature_)(HANDLE h,void* dt,unsigned long l);
typedef int (__stdcall *HidD_GetFeature_)(HANDLE h,void* dt,unsigned long l);
typedef int (__stdcall *HidD_GetIndexedString_)(HANDLE h,unsigned long si,void* bf,unsigned long bfl);
typedef int (__stdcall *HidD_GetPreparsedData_)(HANDLE h,void* dt);
typedef int (__stdcall *HidD_FreePreparsedData_)(void* dt);
typedef NTSTATUS (__stdcall *HidP_GetCaps_)(void* dt,struct _HIDP_CAPS *c);
typedef int (__stdcall *HidD_SetNumInputBuffers_)(HANDLE h,unsigned long n_bf);
static HidD_GetAttributes_ HidD_GetAttributes;
static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
static HidD_GetManufacturerString_ HidD_GetManufacturerString;
static HidD_GetProductString_ HidD_GetProductString;
static HidD_SetFeature_ HidD_SetFeature;
static HidD_GetFeature_ HidD_GetFeature;
static HidD_GetIndexedString_ HidD_GetIndexedString;
static HidD_GetPreparsedData_ HidD_GetPreparsedData;
static HidD_FreePreparsedData_ HidD_FreePreparsedData;
static HidP_GetCaps_ HidP_GetCaps;
static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;
bool _ds4_i=false;
HMODULE _hlib_h=NULL;



void DS4_init(void){
	if (_ds4_i==false){
		_hlib_h=LoadLibraryA("hid.dll");
		assert(_hlib_h!=NULL);
#define RESOLVE(x) x=(x##_)GetProcAddress(_hlib_h,#x);assert(x!=NULL);
		RESOLVE(HidD_GetAttributes);
		RESOLVE(HidD_GetSerialNumberString);
		RESOLVE(HidD_GetManufacturerString);
		RESOLVE(HidD_GetProductString);
		RESOLVE(HidD_SetFeature);
		RESOLVE(HidD_GetFeature);
		RESOLVE(HidD_GetIndexedString);
		RESOLVE(HidD_GetPreparsedData);
		RESOLVE(HidD_FreePreparsedData);
		RESOLVE(HidP_GetCaps);
		RESOLVE(HidD_SetNumInputBuffers);
#undef RESOLVE
		_ds4_i=true;
	}
}



void DS4_cleanup(void){
	if (_hlib_h!=NULL){
		FreeLibrary(_hlib_h);
	}
	_hlib_h=NULL;
	_ds4_i=false;
}



struct DS4DeviceList* DS4_find_all(void){
	struct DS4DeviceList* o=NULL;
	struct DS4DeviceList* c=NULL;
	GUID i_guid={
		0x4d1e55b2,
		0xf16f,
		0x11cf,
		{
			0x88,
			0xcb,
			0x00,
			0x11,
			0x11,
			0x00,
			0x00,
			0x30
		}
	};
	DS4_init();
	SP_DEVINFO_DATA d_dt={
		sizeof(SP_DEVINFO_DATA),
		{
			0,
			0,
			0,
			0
		},
		0,
		0
	};
	SP_DEVICE_INTERFACE_DATA d_idt={
		sizeof(SP_DEVICE_INTERFACE_DATA),
		{
			0,
			0,
			0,
			0
		},
		0,
		0
	};
	HDEVINFO d_il=SetupDiGetClassDevsA(&i_guid,NULL,NULL,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
	SP_DEVICE_INTERFACE_DETAIL_DATA_A* d_ddt=NULL;
	int r;
	uint32_t i=0;
	while (true){
		HANDLE fh=INVALID_HANDLE_VALUE;
		uint32_t sz=0;
		struct _HIDD_ATTRIBUTES d_a;
		r=SetupDiEnumDeviceInterfaces(d_il,NULL,&i_guid,i,&d_idt);
		if (r==0){
			break;
		}
		r=SetupDiGetDeviceInterfaceDetailA(d_il,&d_idt,NULL,0,&sz,NULL);
		d_ddt=malloc(sz);
		d_ddt->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
		r=SetupDiGetDeviceInterfaceDetailA(d_il,&d_idt,d_ddt,sz,NULL,NULL);
		if (r!=0){
			uint32_t j=0;
			while (true){
				char d_nm[256];
				r=SetupDiEnumDeviceInfo(d_il,j,&d_dt);
				if (r==0){
					j=UINT32_MAX;
					break;
				}
				r=SetupDiGetDeviceRegistryPropertyA(d_il,&d_dt,SPDRP_CLASS,NULL,(unsigned char*)d_nm,sizeof(d_nm),NULL);
				if (r==0){
					j=UINT32_MAX;
					break;
				}
				if (strcmp(d_nm,"HIDClass")==0){
					r=SetupDiGetDeviceRegistryPropertyA(d_il,&d_dt,SPDRP_DRIVER,NULL,(unsigned char*)d_nm,sizeof(d_nm),NULL);
					if (r!=0){
						break;
					}
				}
				j++;
			}
			if (j!=UINT32_MAX){
				fh=CreateFileA(d_ddt->DevicePath,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
				if (fh!=INVALID_HANDLE_VALUE){
					d_a.Size=sizeof(struct _HIDD_ATTRIBUTES);
					HidD_GetAttributes(fh,&d_a);
					if (d_a.VendorID==0x054c&&(d_a.ProductID==0x05c4||d_a.ProductID==0x09cc)){
						struct DS4DeviceList* tmp=malloc(sizeof(struct DS4DeviceList));
						tmp->p=malloc(1);
						size_t ln=0;
						for (size_t k=0;true;k++){
							ln++;
							tmp->p=realloc(tmp->p,ln+1);
							*(tmp->p+ln-1)=*(d_ddt->DevicePath+k);
							if (*(d_ddt->DevicePath+k)==0){
								break;
							}
						}
						if (o==NULL){
							o=tmp;
							c=tmp;
							tmp->n=NULL;
						}
						else{
							c->n=tmp;
							c=tmp;
						}
					}
					CloseHandle(fh);
				}
			}
		}
		free(d_ddt);
		i++;
	}
	SetupDiDestroyDeviceInfoList(d_il);
	return o;
}



void DS4_free_list(struct DS4DeviceList* l){
	free(l->p);
	if (l->n!=NULL){
		DS4_free_list(l->n);
	}
	free(l);
}



struct DS4Device* DS4_connect(char* p){
	DS4_init();
	struct DS4Device* o=malloc(sizeof(struct DS4Device));
	o->fc=0;
	o->dt=0;
	o->btn=0;
	o->l2=0;
	o->r2=0;
	o->lx=0;
	o->ly=0;
	o->rx=0;
	o->ry=0;
	o->bt=0;
	o->r=0;
	o->g=0;
	o->b=0;
	o->fr=0;
	o->sr=0;
	o->fon=0;
	o->foff=0;
	o->_fh=CreateFileA(p,(GENERIC_WRITE|GENERIC_READ),FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
	o->_ib=NULL;
	o->_ob=NULL;
	o->_f=0;
	o->_uc=UINT64_MAX;
	o->_o.hEvent=CreateEvent(NULL,false,false,NULL);
	o->_lt.QuadPart=0;
	o->_tf.QuadPart=0;
	// o->_cb0=0;
	// o->_cn0=0;
	// o->_cd0=0;
	// o->_cb1=0;
	// o->_cn1=0;
	// o->_cd1=0;
	// o->_cb2=0;
	// o->_cn2=0;
	// o->_cd2=0;
	// o->_cb3=0;
	// o->_cn3=0;
	// o->_cd3=0;
	// o->_cb4=0;
	// o->_cn4=0;
	// o->_cd4=0;
	// o->_cb5=0;
	// o->_cn5=0;
	// o->_cd5=0;
	if (o->_fh==INVALID_HANDLE_VALUE){
		CloseHandle(o->_o.hEvent);
		free(o);
		return NULL;
	}
	int r=HidD_SetNumInputBuffers(o->_fh,64);
	if (r==0){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	void* d_pdt=NULL;
	r=HidD_GetPreparsedData(o->_fh,&d_pdt);
	if (r==0){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	struct _HIDP_CAPS d_c;
	uint32_t nt_r=HidP_GetCaps(d_pdt,&d_c);
	HidD_FreePreparsedData(d_pdt);
	if (nt_r!=0x110000){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	assert(d_c.InputReportByteLength==64);
	assert(d_c.OutputReportByteLength==32);
	o->_ib=malloc(64);
	for (uint16_t i=0;i<64;i++){
		*(o->_ib+i)=0;
	}
	o->_ob=malloc(32);
	for (uint16_t i=0;i<32;i++){
		*(o->_ib+i)=0;
	}
	QueryPerformanceFrequency(&o->_tf);
	DS4_update(o);
	return o;
}



void DS4_update(struct DS4Device* d){
	uint64_t uc=((uint64_t)d->r<<56)|((uint64_t)d->g<<48)|((uint64_t)d->b<<40)|((uint64_t)d->fr<<32)|((uint64_t)d->sr<<24)|((uint64_t)d->r<<16)|((uint64_t)d->r<<8);
	if (d->_uc!=uc){
		d->_uc=uc;
		*d->_ob=0x05;
		*(d->_ob+1)=0x7f;
		*(d->_ob+2)=0x04;
		*(d->_ob+4)=d->fr;
		*(d->_ob+5)=d->sr;
		*(d->_ob+6)=d->r;
		*(d->_ob+7)=d->g;
		*(d->_ob+8)=d->b;
		*(d->_ob+9)=d->fon;
		*(d->_ob+10)=d->foff;
		OVERLAPPED oe;
		memset(&oe,0,sizeof(OVERLAPPED));
		int r=WriteFile(d->_fh,d->_ob,32,NULL,&oe);
		if (r==0){
			if (GetLastError()!=ERROR_IO_PENDING){
				printf("ERROR: %i\n",GetLastError());
				assert(0);
				return;
			}
		}
		uint32_t br=0;
		r=GetOverlappedResult(d->_fh,&oe,&br,true);
		if (r==0){
			assert(0);
			return;
		}
	}
	uint32_t br=0;
	if ((d->_f&1)==0){
		d->_f|=1;
		memset(d->_ib,0,64);
		ResetEvent(d->_o.hEvent);
		memset(&d->_o,0,sizeof(OVERLAPPED));
		int r=ReadFile(d->_fh,d->_ib,64,NULL,&d->_o);
		if (r==0){
			if (GetLastError()!=ERROR_IO_PENDING){
				CancelIo(d->_fh);
				d->_f&=~1;
				assert(0);
				return;
			}
		}
	}
	int r=GetOverlappedResult(d->_fh,&d->_o,&br,true);
	d->_f&=~1;
	if (r!=0&&br>0){
		d->fc=(uint8_t)(*(d->_ib+7))>>2;
		LARGE_INTEGER c;
		QueryPerformanceCounter(&c);
		if (d->_lt.QuadPart==0){
			d->dt=0;
		}
		else{
			d->dt=(uint32_t)((c.QuadPart-d->_lt.QuadPart)*1000000/d->_tf.QuadPart);
		}
		d->_lt=c;
		uint8_t dp=(*(d->_ib+5)&0x0f);
		d->btn=((dp==0||dp==1||dp==7)?d->btn|BUTTON_UP:d->btn&(~BUTTON_UP));
		d->btn=((dp>=3&&dp<=5)?d->btn|BUTTON_DOWN:d->btn&(~BUTTON_DOWN));
		d->btn=((dp>=5&&dp<=7)?d->btn|BUTTON_LEFT:d->btn&(~BUTTON_LEFT));
		d->btn=((dp>=1&&dp<=3)?d->btn|BUTTON_RIGHT:d->btn&(~BUTTON_RIGHT));
		d->btn=(((*(d->_ib+6))&1)!=0?d->btn|BUTTON_L1:d->btn&(~BUTTON_L1));
		d->btn=(((*(d->_ib+6))&2)!=0?d->btn|BUTTON_R1:d->btn&(~BUTTON_R1));
		d->btn=(((*(d->_ib+6))&4)!=0?d->btn|BUTTON_L2:d->btn&(~BUTTON_L2));
		d->btn=(((*(d->_ib+6))&8)!=0?d->btn|BUTTON_R2:d->btn&(~BUTTON_R2));
		d->btn=(((*(d->_ib+6))&64)!=0?d->btn|BUTTON_L3:d->btn&(~BUTTON_L3));
		d->btn=(((*(d->_ib+6))&128)!=0?d->btn|BUTTON_R3:d->btn&(~BUTTON_R3));
		d->btn=(((*(d->_ib+5))&32)!=0?d->btn|BUTTON_CROSS:d->btn&(~BUTTON_CROSS));
		d->btn=(((*(d->_ib+5))&64)!=0?d->btn|BUTTON_CIRCLE:d->btn&(~BUTTON_CIRCLE));
		d->btn=(((*(d->_ib+5))&16)!=0?d->btn|BUTTON_SQURARE:d->btn&(~BUTTON_SQURARE));
		d->btn=(((*(d->_ib+5))&128)!=0?d->btn|BUTTON_TRIANGLE:d->btn&(~BUTTON_TRIANGLE));
		d->btn=(((*(d->_ib+6))&32)!=0?d->btn|BUTTON_OPTIONS:d->btn&(~BUTTON_OPTIONS));
		d->btn=(((*(d->_ib+6))&16)!=0?d->btn|BUTTON_SHARE:d->btn&(~BUTTON_SHARE));
		d->btn=(((*(d->_ib+7))&1)!=0?d->btn|BUTTON_PS:d->btn&(~BUTTON_PS));
		d->btn=(((*(d->_ib+7))&2)!=0?d->btn|BUTTON_TOUCHPAD:d->btn&(~BUTTON_TOUCHPAD));
		d->l2=*(d->_ib+8);
		d->r2=*(d->_ib+9);
		d->lx=-128+*(d->_ib+1);
		d->ly=127-*(d->_ib+2);
		d->rx=-128+*(d->_ib+3);
		d->ry=127-*(d->_ib+4);
		d->bt=(uint8_t)(((*(d->_ib+30))&16)*15.9375);
	}
}



void DS4_hsl(struct DS4Device* d,uint8_t h,uint8_t s,uint8_t l){
	if (s==0){
		d->r=l;
		d->g=l;
		d->b=l;
	}
	else{
		uint8_t r=h/43;
		uint8_t rm=(h-(r*43))*6;
		uint8_t p=(l*(255-s))>>8;
		uint8_t q=(l*(255-((s*rm)>>8)))>>8;
		uint8_t t=(l*(255-((s*(255-rm))>>8)))>>8;
		switch (r){
			case 0:
				d->r=l;
				d->g=t;
				d->b=p;
				break;
			case 1:
				d->r=q;
				d->g=l;
				d->b=p;
				break;
			case 2:
				d->r=p;
				d->g=l;
				d->b=t;
				break;
			case 3:
				d->r=p;
				d->g=q;
				d->b=l;
				break;
			case 4:
				d->r=t;
				d->g=p;
				d->b=l;
				break;
			default:
				d->r=l;
				d->g=p;
				d->b=q;
				break;
		}
	}
}



void DS4_close(struct DS4Device* d){
	CancelIo(d->_fh);
	if (d->_ib!=NULL){
		free(d->_ib);
	}
	if (d->_ob!=NULL){
		free(d->_ob);
	}
	free(d);
}
