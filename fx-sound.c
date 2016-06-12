// original by MPoupe, modified by Casimo
#include "fxlib.h"
#include "7305.h"

#define DATAPACK 8080

unsigned char UserStack[DATAPACK];

//Conversion tables
#define X 0x10
#define _ 0x00

static const unsigned char LowTable[16]=
{
   _,_,_,_,//0
   X,_,_,_,//1
   X,_,X,_,//2
   _,X,X,X,//3
};

                               //0    1    2    3    4    5    6    7
static const unsigned char HighTable[8]={0x00,0x01,0x44,0x45,0x65,0x67,0xE7,0xEF};

#define gpBuffer UserStack

//static unsigned char *gpBuffer;
static unsigned int giBufIndex = 0;
static unsigned char giTableIndex = 0;
static unsigned char giSubSample = 0;
static unsigned char giFromHighTab = 0;

static int ComOpen()
{
   //unsigned char settings[5]={0,9,0,1,0};//115200,2xstop bits
   unsigned char settings[5]={0,9,0,0,0};//115200,1xstop bits
   if(Serial_Open(&settings))
   {
      return 1;
   }
      //SCIF2.SCSMR.BIT.SRC = 1;//1/5 (for 9860)

   //it has no SRC register (comparing to fx9860G), so actual speed is only ~ 460800
   //SCIF2.SCSMR.BIT.SRC = 0;//1/16 (for 9860)
   SCIF2.SCSMR.BIT.CKS = 0;//1/1
      //SCIF2.SCBRR = 2;//!override speed to 1959390 bps (for 9860)
   SCIF2.SCBRR = 0;//!override speed to 1836000 bps (for 9860)
   return  0;
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
   FONTCHARACTER path[] ={'\\','\\','f','l','s','0','\\','T','E','S','T','.','w','a','v',0};
   unsigned int i, size;
   unsigned int code1, code2;
   int hFile = Bfile_OpenFile(path, _OPENMODE_READ);
   //skip wav header
   if(Bfile_ReadFile(hFile,UserStack,44,-1) != 44)
   {
      return 0;
   }

   size = Bfile_GetFileSize(hFile);
   ComOpen();
   Bdisp_AllClr_VRAM();
   locate( 1, 1 );
   Print(&"Playing");
   Bdisp_PutDisp_DD();

   //read it
   for(i = 0; i < size; i+= DATAPACK)
   {
      Bfile_ReadFile(hFile,UserStack ,DATAPACK,-1);
      for(giBufIndex = 0; giBufIndex < DATAPACK;)
      {
         //it has only 16 B fifo :-(
         while ((SCIF2.SCFDR.WORD >> 8) < 16)
         {
            if(giTableIndex >= 4)
            {
               giSubSample = gpBuffer[giBufIndex++] >> 3;
               giFromHighTab = HighTable[giSubSample>>2];
               giSubSample = (giSubSample & 3) << 2;
               giTableIndex = 0;
            }
            SCIF2.SCFTDR = (unsigned char)LowTable[giSubSample|giTableIndex]|giFromHighTab;
            giTableIndex++;
         }
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
