#include <ds4.h>
#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>
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
						struct DS4DeviceList* tmp=calloc(1,sizeof(struct DS4DeviceList));
						tmp->p=malloc(1);
						size_t ln=0;
						for (size_t k=0;*(d_ddt->DevicePath+k)!=0;k++){
							ln++;
							tmp->p=realloc(tmp->p,ln+1);
							*(tmp->p+ln-1)=*(d_ddt->DevicePath+k);
						}
						*(tmp->p+ln-1)=0;
						printf("PATH: %s\n",tmp->p);
						if (o==NULL){
							o=tmp;
							c=tmp;
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



void DS4_free_list(struct DS4DeviceList*);



struct DS4Device* DS4_connect(char* p);



bool DS4_close(struct DS4Device* d);
