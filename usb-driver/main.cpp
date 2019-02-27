#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlpc350_common.h"
#include "dlpc350_api.h"
#include "dlpc350_usb.h"

int main(int argc, char ** argv){
	unsigned char powerlevel=127;
	unsigned char r,g,b;	
	bool setpower=false;
	if(argc==2){
		powerlevel=atoi(argv[1])&0xff;
		setpower=true;
	}
	if(setpower)printf("Setting powerlevel to %d\n",powerlevel);
	DLPC350_USB_Init();
	if(DLPC350_USB_Open()==0){
		if(setpower){
			DLPC350_SetLedEnables(false, false, false, true); //enable blue channel only
			DLPC350_SetLedCurrents(0, 0, 255-powerlevel); //set blue channel power level
			DLPC350_SetLongAxisImageFlip(true);
			DLPC350_SetShortAxisImageFlip(false);
		}
		DLPC350_GetLedCurrents(&r, &g, &b);
		printf("Current power level is: %d\n", 255-b);
		//DLPC350_SetMode(false);//set video mode
		DLPC350_USB_Exit();
		return 0;
		
	}else{
		printf("Failed to connect to projector\n");
		DLPC350_USB_Exit();
		return 1;
	}
	
	
}
