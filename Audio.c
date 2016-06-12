#include "fxlib.h"

#define DATAPACK 8080

unsigned char UserStack[DATAPACK];

void Open()
{
	char open_mode[6] = {0};

	open_mode[ 0 ] = 0; // always 0
	open_mode[ 1 ] = 9; // 0=300, 1=600, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200, 7=38400, 8=57600, 9=115200 baud
	open_mode[ 2 ] = 0; // parity: 0=no; 1=odd; 2= even
	open_mode[ 3 ] = 0; // datalength: 0=8 bit; 1=7 bit
	open_mode[ 4 ] = 0; // stop bits: 0=one; 1=two
	open_mode[ 5 ] = 0; // always 0
	   
	Serial_Open( &open_mode );
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	static const unsigned char LowTable[8]={0x00,0x08,0x88,0x94,0x55,0xD5,0xDD,0xDF};
	static unsigned short int giBufIndex = 0;

	FONTCHARACTER path[] ={'\\','\\','f','l','s','0','\\','T','E','T','R', 'I','S','.','w','a','v',0};

	unsigned int i, size;
	unsigned int code1, code2;
	int hFile = Bfile_OpenFile(path, _OPENMODE_READ);
	
	Open();
	
	//skip wav header
	if(Bfile_ReadFile(hFile,UserStack,44,-1) != 44)
	{
		return 0;
	}

	size = Bfile_GetFileSize(hFile);

	Bdisp_AllClr_VRAM();
	locate( 1, 1 );
	Print(&"Playing");
	Bdisp_PutDisp_DD();

	//read it
	for(i = 0; i < size; i+= DATAPACK)
	{
		Bfile_ReadFile(hFile,UserStack ,DATAPACK,-1);
		for(giBufIndex = 0; giBufIndex < DATAPACK; giBufIndex++)
		{
			unsigned char iVal = ~LowTable[UserStack[giBufIndex] >> 5];
			while(Serial_WriteByte(iVal));
		}

		if(Bkey_GetKeyWait(&code1,&code2,KEYWAIT_HALTOFF_TIMEROFF,0,1,0) == KEYREP_KEYEVENT)
		{
			if(code1 == 4 && code2 == 8)
			break;
		}
	}
	Bfile_CloseFile(hFile);

	Serial_Close(1);
}

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{return INIT_ADDIN_APPLICATION(isAppli, OptionNum);}
#pragma section

