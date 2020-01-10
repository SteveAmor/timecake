
// generic saveram code

#include "debug.h"
#include "saveram.h"
#include "string.h"



int saveram_setup(void)
{
	if( ( saveram->magick  != SAVERAM_MAGICK   ) ||
	    ( saveram->version != SAVERAM_VERSION  ) ||
	    ( saveram->length  != sizeof(saveram)  ) ) // SIMPLE VALIDATION CHECK
	{
		saveram_format();
	}
		saveram_format();
	return 1;
}

void saveram_format(void)
{
	memset(saveram,0xf0,sizeof(saveram));
	for(int i=0;i<sizeof(saveram);i++)
	{
		if( ((char*)saveram)[i]!=0xf0 )
		{
			PRINTF("Saveram write FAIL!");
			while(1);
		}
	}
	memset(saveram,0x00,sizeof(saveram));
	
	saveram->magick  = SAVERAM_MAGICK;
	saveram->version = SAVERAM_VERSION;
	saveram->length  = sizeof(saveram);
}

