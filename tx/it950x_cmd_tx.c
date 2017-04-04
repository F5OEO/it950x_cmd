/**
 * Copyright (c) 2013 ITE Corporation. All rights reserved. 
 * 
 * Date:
 *    2013/10/15
 *
 * Abstract:
 *    ITEAPIEXTest_TX.cpp : Defines the entry point for the console application.
 *    This is a sample testkit for IT9500 device (modulator).
 */

#include "api.h"
#include "crc.h"
#include "options.h"

#include <signal.h>
#include <sys/stat.h>


#define ERROR(x...)												\
	do {														\
		fprintf(stderr, "ERROR: ");								\
		fprintf(stderr, x);										\
		fprintf (stderr, "\n");									\
	} while (0)

#define PERROR(x...)											\
	do {														\
		fprintf(stderr, "ERROR: ");								\
		fprintf(stderr, x);										\
		fprintf (stderr, " (%s)\n", strerror(errno));			\
	} while (0)


typedef struct {
	char *name;
	int value;
} Param;

typedef struct {
	Dword Frequency;
	Word Bandwidth;
} ModulatorParam;

typedef struct {
	uint32_t count;
	uint32_t ulErrCount;
	uint32_t ulLostCount;
	uint16_t pid;
	uint8_t  sequence1;
	uint8_t  sequence2;
	uint8_t  dup_flag;
} PIDINFO, *PPIDINFO;

static int gDoExit = 0;

/*
static const Param inversion_list [] = {
	{ "INVERSION_OFF", INVERSION_OFF },
	{ "INVERSION_ON", INVERSION_ON },
	{ "INVERSION_AUTO", INVERSION_AUTO }
};

static const Param bw_list [] = {
	{ "BANDWIDTH_6_MHZ", BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", BANDWIDTH_8_MHZ }
};

static const Param fec_list [] = {
	{ "FEC_1_2", FEC_1_2 },
	{ "FEC_2_3", FEC_2_3 },
	{ "FEC_3_4", FEC_3_4 },
	{ "FEC_4_5", FEC_4_5 },
	{ "FEC_5_6", FEC_5_6 },
	{ "FEC_6_7", FEC_6_7 },
	{ "FEC_7_8", FEC_7_8 },
	{ "FEC_8_9", FEC_8_9 },
	{ "FEC_AUTO", FEC_AUTO },
	{ "FEC_NONE", FEC_NONE }
};

static const Param guard_list [] = {
	{"GUARD_INTERVAL_1_16", GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4", GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8", GUARD_INTERVAL_1_8}
};

static const Param hierarchy_list [] = {
	{ "HIERARCHY_1", HIERARCHY_1 },
	{ "HIERARCHY_2", HIERARCHY_2 },
	{ "HIERARCHY_4", HIERARCHY_4 },
	{ "HIERARCHY_NONE", HIERARCHY_NONE }
};

static const Param constellation_list [] = {
	{ "QPSK", QPSK },
	{ "QAM_128", QAM_128 },
	{ "QAM_16", QAM_16 },
	{ "QAM_256", QAM_256 },
	{ "QAM_32", QAM_32 },
	{ "QAM_64", QAM_64 }
};

static const Param transmissionmode_list [] = {
	{ "TRANSMISSION_MODE_2K", TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K", TRANSMISSION_MODE_8K },
};
*/

#define LIST_SIZE(x) sizeof(x)/sizeof(Param)
#define TX_DATA_LEN 65424

int gTransferRate = 0;
uint32_t gTransferInterval = 0;
int gRateControl = 0; 
Dword g_ChannelCapacity = 0;
Byte NullPacket[188]={0x47,0x1f,0xff,0x1c,0x00,0x00};
MODULATION_PARAM g_ChannelModulation_Setting;

//***************** IQTable Constant *****************//
#define IQTABLE_NROW 65536
#define Default_IQtable_Path "./bin/IQ_table.bin"

//Test Periodical Custom Packets Insertion, (for SI/PSI table insertion)

//Sample SDT
Byte CustomPacket_1[188]={
	0x47,0x40,0x11,0x10,0x00,0x42,0xF0,0x36,0x00,0x99,0xC1,0x00,0x00,0xFF,0x1A,0xFF,
	0x03,0xE8,0xFC,0x80,0x25,0x48,0x23,0x01,0x10,0x05,0x49,0x54,0x45,0x20,0X20,0X20,
	0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0x10,0x05,0x49,0x54,0x45,0x20,0x43,
	0x68,0x61,0x6E,0x6E,0x65,0x6C,0x20,0x31,0x20,0x20,0xFF,0xFF,0xFF,0xFF //LAST 4 BYTE=CRC32
};

//Sample NIT
Byte CustomPacket_2[188]={
	0x47,0x40,0x10,0x10,0x00,0x40,0xf0,0x38,0xff,0xaf,0xc1,0x00,0x00,0xf0,0x0d,0x40,/*0x00000000*/
	0x0b,0x05,0x49,0x54,0x45,0x20,0x4f,0x66,0x66,0x69,0x63,0x65,0xf0,0x1e,0x00,0x99,/*0x00000010*/
	0xff,0x1a,0xf0,0x18,0x83,0x04,0x03,0xe8,0xfc,0x5f,0x5a,0x0b,0x02,0xeb,0xae,0x40,/*0x00000020*/
	0x1f,0x42,0x52,0xff,0xff,0xff,0xff,0x41,0x03,0x03,0xe8,0x01,0x1a,0xe6,0x2c,0x3f,/*0x00000030*/
};

Byte CustomPacket_3[188]={0x47,0x10,0x03,0x1c,0x00,0x00};
Byte CustomPacket_4[188]={0x47,0x10,0x04,0x1c,0x00,0x00};
Byte CustomPacket_5[188]={0x47,0x10,0x05,0x1c,0x00,0x00};


//==================================================================================================
static void sighandler(int signum)
{
    fprintf(stderr, "Signal %d caught, exiting!\n", signum);
    gDoExit = 1;
}

//==================================================================================================
int kbhit(void)  
{  
	struct termios oldt, newt;  
	int ch;  
	int oldf;  
	tcgetattr(STDIN_FILENO, &oldt);  
	newt = oldt;  
	newt.c_lflag &= ~(ICANON | ECHO);  
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);  
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);  
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);  
	ch = getchar();  
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  
	fcntl(STDIN_FILENO, F_SETFL, oldf);  
	if(ch != EOF) {  
		ungetc(ch, stdin);  
		return 1;  
	}  
	return 0;  
}  

//==================================================================================================
intmax_t GetFileSize(const char* filePath)
{
	struct stat statbuf;
	
	if(stat(filePath, &statbuf) == -1)
    {
        fprintf(stderr, "Error %s\n", strerror(errno));
		return -1;
    }
    
	return (intmax_t) statbuf.st_size;
}

//==================================================================================================
Dword GetChannelCapacity(options_t *options)
{
    Dword ChannelCapacity = 0;
    
    ChannelCapacity = options->bandwidth*1000;
    ChannelCapacity *= options->constellation_index*2+2;

    switch (options->interval_index) 
    {
        case 0: //1/32
            ChannelCapacity = ChannelCapacity*32/33;
            break;
        case 1: //1/16
            ChannelCapacity = ChannelCapacity*16/17;
            break;
        case 2: //1/8
            ChannelCapacity = ChannelCapacity*8/9;
            break;
        case 3: //1/4
            ChannelCapacity = ChannelCapacity*4/5;
            break;
    }
    
    switch (options->coderate_index) 
    {
        case 0: //1/2
            ChannelCapacity = ChannelCapacity*1/2;
            break;
        case 1: //2/3
            ChannelCapacity = ChannelCapacity*2/3;
            break;
        case 2: //3/4
            ChannelCapacity = ChannelCapacity*3/4;
            break;
        case 3: //5/6
            ChannelCapacity = ChannelCapacity*5/6;
            break;
        case 4: //7/8
            ChannelCapacity = ChannelCapacity*7/8;
            break;
    }

    ChannelCapacity = ChannelCapacity/544*423;
    
    return ChannelCapacity;
}

//==================================================================================================
void printDeviceType(options_t *options, Byte DevNo)
{
    int device_type = 0;

    g_ITEAPI_TxGetDeviceType((Byte*)&device_type, options->dev_handle);
    fprintf(stderr, "\nCurrent Device Type: %d\n", device_type);    
}

//==================================================================================================
void setGain(ModulatorParam *param, options_t *options)
{
    int outgainvalue;
    Dword frequency = 0;
    Dword bandwidth = 0;
    int MaxGain = 0;
    int MinGain = 0;
    int Gain = 0;
    
    frequency = param->Frequency;
    bandwidth = param->Bandwidth;
    
    if(g_ITEAPI_TxGetGainRange(frequency, (Word)bandwidth, &MaxGain, &MinGain, options->dev_handle) != ERR_NO_ERROR)
    {
        fprintf(stderr, "g_ITEAPI_TxGetGainRange error\n");
        return;
    }
    
    fprintf(stderr, "MinGain: %d, MaxGain: %d\n", MinGain, MaxGain);

    if(g_ITEAPI_TxGetOutputGain(&Gain, options->dev_handle) != 0) 
    {
        fprintf(stderr, "g_ITE_TxGetOutputGain error\n");
        return;
    }
    
    if(g_ITEAPI_TxAdjustOutputGain(options->adjustoutputgain, &outgainvalue, options->dev_handle) == ERR_NO_ERROR)
    {
        fprintf(stderr, "g_ITEAPI_TxAdjustOutputGain ok: %d dB\n", outgainvalue);
    }
    else
    {
        fprintf(stderr, "g_ITEAPI_TxAdjustOutputGain error\n");
    }
}

//==================================================================================================
void setTPSCellId(options_t *options)
{
    TPS tps;

    memset(&tps, 0, sizeof(tps));

    // Get current TPS setting 
    if(g_ITEAPI_TxGetTPS(&tps, options->dev_handle) == 0) 
    {
        fprintf(stderr, "=====================\n");
        fprintf(stderr, "Current TPS:\n");
        fprintf(stderr, "cellid = %d\n", tps.cellid);
        fprintf(stderr, "constellation = %d\n", tps.constellation);
        fprintf(stderr, "highCodeRate = %d\n", tps.highCodeRate);
        fprintf(stderr, "interval = %d\n", tps.interval);
        fprintf(stderr, "lowCodeRate = %d\n", tps.lowCodeRate);
        fprintf(stderr, "transmissionMode = %d\n", tps.transmissionMode);
        fprintf(stderr, "=====================\n");
    } 
    else
    {
        fprintf(stderr, "g_ITEAPI_TxGetTPS error\n");
    }

    tps.cellid = options->tps_cellid;
    
    if(g_ITEAPI_TxSetTPS(tps, options->dev_handle) == 0) 
    {
        fprintf(stderr, "g_ITEAPI_TxSetTPS ok\n");
    } 
    else
    {
        fprintf(stderr, "g_ITEAPI_TxSetTPS error\n");
    }
}

//==================================================================================================
Dword SetIQCalibrationTable(options_t *options) 
{
    FILE *pfile = NULL;
    Word error = ERR_NO_ERROR;
    Byte* ptrIQtable;
    Word IQtableSize;

    pfile = fopen(options->iq_table_filename, "rb");
    
    if(pfile == NULL) 
    {
        fprintf(stderr, "Could not open %s\n", options->iq_table_filename);
    }
    else 
    {
        fprintf(stderr, "Open %s ok!\n", options->iq_table_filename);
        IQtableSize = GetFileSize(options->iq_table_filename);
    }
    
    ptrIQtable = (Byte*) malloc(sizeof(Byte)*IQtableSize);
    error = fread(ptrIQtable,1,IQtableSize,pfile);
    
    if(error != IQtableSize) 
    {
        fprintf(stderr, "IQ table read error!\n");
        return error;
    }

    error = g_ITEAPI_TxSetIQTable(ptrIQtable, IQtableSize, options->dev_handle);
    
    if(error)
    {
        fprintf(stderr, "Set IQ Calibration Table fail: 0x%08x \n", error);
    }
    else
    {
        fprintf(stderr, "Set IQ Calibration Table ok!\n");
    }

    if(pfile)
    {
        fclose(pfile);
    }
    
    if(ptrIQtable)
    {
        free(ptrIQtable);
    }

    return error;
}

//==================================================================================================
static int GetDriverInfo(HandleType HandleNum,Byte DevNo)
{
	Word ChipType = 0;		
	Byte Tx_NumOfDev = 0;//, device_type = 7;
	DEVICE_INFO DeviceInfo;
	Dword dwError = ERR_NO_ERROR;

	if((dwError = g_ITEAPI_TxDeviceInit(HandleNum,DevNo)) == ERR_NO_ERROR)
		fprintf(stderr, "g_ITEAPI_TxDeviceInit ok\n");
	else 
		fprintf(stderr, "g_ITEAPI_TxDeviceInit fail\n");

	if((dwError = g_ITEAPI_TxGetNumOfDevice(&Tx_NumOfDev,DevNo)) == ERR_NO_ERROR)
		fprintf(stderr, "%d Devices\n", Tx_NumOfDev);
	else 
		fprintf(stderr, "g_ITEAPI_TxGetNumOfDevice error\n");	


	if ((dwError = g_ITEAPI_TxGetDrvInfo(&DeviceInfo,DevNo))) {
		fprintf(stderr, "Get Driver Info failed 0x%lu!\n", dwError);
	} else {
		fprintf(stderr, "g_ITEAPI_GetDrvInfo ok\n");		
		fprintf(stderr, "DriverInfo.DriverVerion  = %s\n", DeviceInfo.DriverVerion);
		fprintf(stderr, "DriverInfo.APIVerion     = %s\n", DeviceInfo.APIVerion);
		fprintf(stderr, "DriverInfo.FWVerionLink  = %s\n", DeviceInfo.FWVerionLink);
		fprintf(stderr, "DriverInfo.FWVerionOFDM  = %s\n", DeviceInfo.FWVerionOFDM);
		fprintf(stderr, "DriverInfo.Company       = %s\n", DeviceInfo.Company);
		fprintf(stderr, "DriverInfo.SupportHWInfo = %s\n", DeviceInfo.SupportHWInfo);
		fprintf(stderr, "DriverInfo.ChipType      = IT%x", DeviceInfo.ProductID);
	}

	return dwError;
}

//==================================================================================================
static long TxSetChannelTransmissionParameters(ModulatorParam *param, options_t *options)
{
    Dword dwStatus = 0;
    Dword ChannelCapacity = 0;	
    MODULATION_PARAM ChannelModulation_Setting;	

    dwStatus = g_ITEAPI_TxSetChannel(options->frequency, options->bandwidth, options->dev_handle);
    
    if (dwStatus) {
        fprintf(stderr, "g_ITEAPI_TxSetChannel error\n");	
        return dwStatus;
    }
    else
    {
        fprintf(stderr, "g_ITEAPI_TxSetChannel ok\n");
    }

    param->Frequency = options->frequency;
    param->Bandwidth = options->bandwidth;
        
    ChannelModulation_Setting.constellation = options->constellation_index;
    ChannelModulation_Setting.highCodeRate = options->coderate_index;
    ChannelModulation_Setting.interval = options->interval_index;
    ChannelModulation_Setting.transmissionMode = options->fftsize_index;
        
    dwStatus = g_ITEAPI_TxSetChannelModulation(ChannelModulation_Setting, options->dev_handle);

    if (dwStatus)
    {
        fprintf(stderr, "\n****** g_ITEAPI_TxSetChannelModulation error!!, %lu ******\n", dwStatus);
    }
    else
    {
        fprintf(stderr, "\n****** g_ITEAPI_TxSetChannelModulation ok  ******\n\n");
    }
    
    g_ChannelModulation_Setting = ChannelModulation_Setting;

    ChannelCapacity = options->bandwidth*1000;
    ChannelCapacity *= ChannelModulation_Setting.constellation*2+2;

    switch (ChannelModulation_Setting.interval) 
    {
        case 0: //1/32
            ChannelCapacity=ChannelCapacity*32/33;
            break;
        case 1: //1/16
            ChannelCapacity=ChannelCapacity*16/17;
            break;
        case 2: //1/8
            ChannelCapacity=ChannelCapacity*8/9;
            break;
        case 3: //1/4
            ChannelCapacity=ChannelCapacity*4/5;
            break;
    }
    
    switch (ChannelModulation_Setting.highCodeRate) 
    {
        case 0: //1/2
            ChannelCapacity=ChannelCapacity*1/2;
            break;
        case 1: //2/3
            ChannelCapacity=ChannelCapacity*2/3;
            break;
        case 2: //3/4
            ChannelCapacity=ChannelCapacity*3/4;
            break;
        case 3: //5/6
            ChannelCapacity=ChannelCapacity*5/6;
            break;
        case 4: //7/8
            ChannelCapacity=ChannelCapacity*7/8;
            break;
    }

    ChannelCapacity = ChannelCapacity/544*423;
    fprintf(stderr, "The Maximum Channel Capacity is %lu bps(%lu Kbps)\n", ChannelCapacity, ChannelCapacity/1000);
    
    return ChannelCapacity;
}

//==================================================================================================
uint32_t TxTransferTimeDelay(struct timeval start, struct timeval end)
{
	uint32_t diff, delay;

	diff = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
	delay = gTransferInterval - diff;
	//printf("%d  ", delay);
	
	if (delay > 0)
		return delay;
	else
		return 0;
}


//==================================================================================================
long int Get_DataBitRate_S(options_t *options)
{

    int mPID = 0;
	const int BUFSIZE = 512*188; 
	Byte pbTSBuffData[BUFSIZE]; 
	Dword len, FileSize, i;

	double min_Packet_Time=0xfffffff;   // in xxx  Hz per Packet, 90KHz PCR clock
	double max_Packet_Time=0x0;		 	// in xxx  Hz per Packet, 90KHz PCR clock
	double Packet_Time=0x0;

	ULONGLONG pcr_diff;
	Dword     pkt_diff;
	FILE *TsFile = NULL;
	intmax_t fileLength;

	Dword dwReadSize = 0;
	Dword FirstSyncBytePos=0;
	Dword FileOffset=0;

	ULONGLONG pcr1=0,pcr2=0,prev_pcr=0;
	ULONGLONG pcr1_offset=0,pcr2_offset=0,prev_pcr_offset=0;
	long int lStreamBitRate = 0;

	//Dword pcrb=0,pcre=0;
	ULONGLONG pcr=0;
	int adaptation_field_control=0 ;
	unsigned PID=0	;
	long lStreamBitRate_Min;
	long lStreamBitRate_Max;	

	if(!(TsFile = fopen(options->ts_filename, "r+"))) 
    {
		fprintf(stderr, "Get_DataBitRate_S: Open TS File Error!\n");
		return -1;
	}
	fileLength = GetFileSize(options->ts_filename);

	len=FileSize=fileLength;

	if(len > BUFSIZE) len = BUFSIZE;


	fseek(TsFile, 0, SEEK_SET);	
	dwReadSize = fread(pbTSBuffData, 1, len, TsFile);

	//Find the first 0x47 sync byte
	for (FirstSyncBytePos=0;(FirstSyncBytePos<dwReadSize-188);FirstSyncBytePos++) 
	{
        if ((pbTSBuffData[FirstSyncBytePos] == 0x47)&&(pbTSBuffData[FirstSyncBytePos+188] == 0x47))
        {
             //Sync byte found
             fprintf(stderr, "TS Sync byte found in offset:%lu\n",FirstSyncBytePos);
             break;
        }
	}

	if (FirstSyncBytePos>=dwReadSize-188) {
	   fprintf(stderr, "No sync byte found, it's not a valid 188-byte TS file!!!\n");

	   return 0;
	}

	fseek(TsFile, FirstSyncBytePos, SEEK_SET);
	dwReadSize = fread(pbTSBuffData, 1, len, TsFile);
//	SetFilePointer(TsFile, FirstSyncBytePos, NULL, FILE_BEGIN);	
//	ReadFile(TsFile,pbTSBuffData,len,&dwReadSize,NULL);


	FileOffset=FirstSyncBytePos;
	while (dwReadSize>188) 
		{          
		for (i=0;(i<dwReadSize);i+=188) 
			{
			 adaptation_field_control=0 ;
			 PID=0;
				PID = ((pbTSBuffData[i+1] & 0x1f) << 8) |pbTSBuffData[i+2];	
				adaptation_field_control = ((pbTSBuffData[i+3] >> 4) & 0x3);
				if ( PID!=0x1fff &&(adaptation_field_control == 3 || adaptation_field_control == 2))
				{
					//CHECK adaptation_field_length !=0 && PCR flag on
					if (pbTSBuffData[i+4] != 0 && (pbTSBuffData[i+5] & 0x10) != 0)
					{
						//pcrb=0;
						//pcre=0;
						pcr=0;

							
							
							pcr=((ULONGLONG)pbTSBuffData[i+6]<<25) |
								 (pbTSBuffData[i+7]<<17) |
								 (pbTSBuffData[i+8]<<9) |
								 (pbTSBuffData[i+9]<<1) |
								 (pbTSBuffData[i+10]>>7) ;

							//pcre = ((pbTSBuffData[i+10] & 0x01) << 8) | (pbTSBuffData[i+11]);

							//printf("Offset:%d,PID:%d(0x%x),PCR:(0x%lx-%lx)\n",FileOffset+i,PID,PID,pcr);							

							if (!pcr1)
							{
								pcr1=pcr;	
								pcr1_offset=FileOffset+i;
								mPID=PID;
								fprintf(stderr, "1'st PCR Offset:%d,PID:(0x%x),PCR:(0x%lx-%lx)\n",(int)(FileOffset+i),PID, (long unsigned int)PID, (long unsigned int)pcr);								
								prev_pcr=pcr1;
								prev_pcr_offset=pcr1_offset;
							}
							else {
								if (mPID==PID) 
								{


									if (prev_pcr) {
										pkt_diff=(Dword)((((ULONGLONG)FileOffset+i)-prev_pcr_offset)/188);
										pcr_diff=pcr-prev_pcr;
										Packet_Time=(double)pcr_diff/pkt_diff;

										if (min_Packet_Time>Packet_Time) {
											min_Packet_Time=Packet_Time;
											//printf ("pcr diffb=%x-%x,pkt#=%d,pkt time=%f, min=%f, max=%f\n", pcr_diff,pkt_diff,Packet_Time,min_Packet_Time,max_Packet_Time);
										}
										if (max_Packet_Time<Packet_Time) {
											max_Packet_Time=Packet_Time;
											//printf ("pcr diffb=%x-%x,pkt#=%d,pkt time=%f, min=%f, max=%f\n", pcr_diff,pkt_diff,Packet_Time,min_Packet_Time,max_Packet_Time);
										}
									}
									

									pcr2=pcr;	
									pcr2_offset=(ULONGLONG)FileOffset+i;

									prev_pcr=pcr2;
									prev_pcr_offset=pcr2_offset;
									//if (FileOffset>10000000) printf("2'nd PCR Offset:%u,PCRB:%u(0x%x),PCRE:%u(0x%x),PCR:%l(0x%lx)\n",FileOffset+i,pcrb,pcrb,pcre,pcre,pcr,pcr);
								}
							}


					
					}
				}

			}
			FileOffset+=dwReadSize;




			//Both PCR1 & PCR2 are found,
			//If it's a large file (>20MB) , then we skip the middle part of the file,
			// and try to locate PCR2 in the last 10 MB in the file end
			// As PCR2 gets farther away from PCR1, the bit rate calculated gets more precise
//			if (0)
		    if (FileOffset>10000000 && FileOffset<FileSize-10000000 && pcr2) 
			{
	 				
				//Move to the last 10 MB position of file 
				//SetFilePointer(TsFile, FileSize-10000000, NULL, FILE_BEGIN);	

				//SetFilePointer(TsFile, -10000000, NULL, FILE_END);	
				
				//ReadFile(TsFile,pbTSBuffData,len,&dwReadSize,NULL);
				//Move to the last 10 MB position of file 
				fseek(TsFile, -10000000, SEEK_END);				
				dwReadSize = fread(pbTSBuffData, 1, len, TsFile);				

				//Find the first 0x47 sync byte
				for (FirstSyncBytePos=0;(FirstSyncBytePos<dwReadSize-188);FirstSyncBytePos++) 
				{
					if ((pbTSBuffData[FirstSyncBytePos] == 0x47)&&(pbTSBuffData[FirstSyncBytePos+188] == 0x47))
					{
						//Sync byte found
						//printf("TS Sync byte found in offset:%d\n",FirstSyncBytePos+FileSize-10000000);
						break;
					}
				}

				if (FirstSyncBytePos>=dwReadSize-188)
				{
					fprintf(stderr, "No sync byte found in the end 10 MB of file!!!\n");
					break;
				}

				//SetFilePointer(TsFile, FileSize-10000000+FirstSyncBytePos, NULL, FILE_BEGIN);	

				//SetFilePointer(TsFile, -10000000+FirstSyncBytePos, NULL, FILE_END);	
				 fseek(TsFile, -10000000+FirstSyncBytePos, SEEK_SET);					

				FileOffset=FileSize-10000000+FirstSyncBytePos;

				prev_pcr=0;
				prev_pcr_offset=0;
			}
			
			
			//if (FileOffset+len>FileSize) break; //EOF			
			dwReadSize = fread(pbTSBuffData, 1, len, TsFile);
			//ReadFile(TsFile,pbTSBuffData,len,&dwReadSize,NULL);
	}



	if (pcr2) 
	{
		double fTmp;
		if (pcr2>pcr1)
			fTmp =(double) ((pcr2_offset-pcr1_offset )* 8)/(double) ((pcr2 - pcr1));  //Bits per Hz
		else
			fTmp =(double) ((pcr2_offset-pcr1_offset )* 8)/(double) ((pcr2 +0x1FFFFFFFFLL- pcr1));  //Bits per Hz		

		lStreamBitRate = (long)(fTmp*90000);	 ////Bits per Seconds, 90KHz/Sec)
		
		//fTmp =(double) ((pcr2_offset-pcr1_offset ))/(double) ((pcr2 - pcr1))*8;  //Bits per Hz
		//float fTmp =(float) (pcr2_offset-pcr1_offset * 8) /(float) (pcr2 - pcr1);
		//lStreamBitRate = (long)(fTmp*90000);	 ////Bits per Seconds, 90KHz/Sec)
		
		lStreamBitRate_Min=(long)(((double) 90000)/max_Packet_Time*188*8);
		lStreamBitRate_Max=(long)(((double) 90000)/min_Packet_Time*188*8);


		fprintf (stderr, "min packet time=%f, max packet time=%f\n",min_Packet_Time,max_Packet_Time);
		fprintf (stderr, "Min Stream Data Rate=%d bps (%d Kbps)\n", (int)lStreamBitRate_Min, (int)lStreamBitRate_Min/1000);
		fprintf (stderr, "Max Stream Data Rate=%d bps (%d Kbps)\n", (int)lStreamBitRate_Max, (int)lStreamBitRate_Max/1000);

		fprintf (stderr, "Average Stream Data Rate=%d bps (%d Kbps)\n", (int)lStreamBitRate, (int)lStreamBitRate/1000);


	}
    
    if (options->ts_use_max_rate)
    {
        fprintf (stderr, "TS Stream Data Rate forcibly set to the maximum of %d bps (%d Kbps)\n", (int)lStreamBitRate_Max, (int)lStreamBitRate_Max/1000);
        lStreamBitRate = lStreamBitRate_Max;
    }
    
	return lStreamBitRate;
	
}

//==================================================================================================
//Analyze PAT TSID and SID
long int Get_PAT_TSID_SID(FILE * TsFile,Byte *TSid,Byte *Sid,intmax_t FileLength)
{
    //int mPID = 0;
    unsigned i;
	unsigned int j;
	const int BUFSIZE = 512*188; 
	unsigned PID, psi_offset, sec_len,numofserv;	
	Byte pbTSBuffData[BUFSIZE]; 
	Dword   len,FileSize;

	len=FileSize=FileLength; 
	if(len > BUFSIZE) 		len = BUFSIZE;

	Dword dwReadSize = 0;
	Dword FirstSyncBytePos=0;
	Bool bPAT=False, bSDT=False; //,bNIT=False;

	fseek(TsFile, 0, SEEK_SET);	
	dwReadSize = fread(pbTSBuffData, 1, len, TsFile);

	//Find the first 0x47 sync byte
	for (FirstSyncBytePos=0;(FirstSyncBytePos<dwReadSize-188);FirstSyncBytePos++) 
	{
			if ((pbTSBuffData[FirstSyncBytePos] == 0x47)&&(pbTSBuffData[FirstSyncBytePos+188] == 0x47))
			{
				//Sync byte found
				//printf("TS Sync byte found in offset:%d\n",FirstSyncBytePos);
				break;
			}
	}

	if (FirstSyncBytePos>=dwReadSize-188) {
		fprintf(stderr, "No sync byte found, it's not a valid 188-byte TS file!!!\n");

		return 0;
	}

	fseek(TsFile, FirstSyncBytePos, SEEK_SET);
	dwReadSize = fread(pbTSBuffData, 1, len, TsFile);

	while (dwReadSize>188&&!bPAT) 
	{		
			for (i=0;(i<dwReadSize);i+=188) 
			{
				PID=0; psi_offset=0; sec_len=0; numofserv=0;

				PID = ((pbTSBuffData[i+1] & 0x1f) << 8) |pbTSBuffData[i+2];	
				if (PID==0x11&&!bSDT)
				{
					bSDT=True;
					fprintf(stderr, "Warning: SDT table already exists in this stream!\r\n");
				}

				if (PID==0&&!bPAT)
				{
					psi_offset=pbTSBuffData[i+4];

					if (pbTSBuffData[i+psi_offset+5]!=0) 
					{
						//it's not PAT Table ID
						continue;
					}

					sec_len=pbTSBuffData[i+psi_offset+7];
					memcpy(TSid, pbTSBuffData+i+psi_offset+8,2); //note it's big-endian
					numofserv=(sec_len-9)/4;
					for (j=0;j<numofserv;j++) 
					{
						memcpy(Sid, pbTSBuffData+i+psi_offset+0xd+j*4,2); //note it's big-endian
						
						
						if (Sid[0] || Sid[1]) 
						{
							//We only want the first TV service 
							fprintf(stderr, "PAT TS ID:0x%02x%02x and Service ID:0x%02x%02x found\r\n",TSid[0],TSid[1],Sid[0],Sid[1]);
							bPAT=True;
							break;
						}
						else 
						{
							//zero service id is NIT, instead of a service
							//bNIT=True;
						}
					}
			 
				}
			}
			if (bPAT&&bSDT) break;
			dwReadSize = fread(pbTSBuffData, 1, len, TsFile);			
	}
	if (!bPAT) fprintf(stderr, "No PAT or Service ID found!\r\n");
	return 0;

}

//==================================================================================================
void SetPeriodicCustomPacket(Handle TsFile, intmax_t FileLength, ModulatorParam *param, options_t *options)
{
    int timer_interval, i; //, ret;
    Byte TSid[2]={0x00, 0x00}; //Note: it's big-endian
    Byte ONid[2]={0x01, 0x01};
    Byte NETid[2]={0x02, 0x02};
    Byte Sid[2]={0x00, 0x01};
    Byte ProviderName[16]={0x05,'i','T','E',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
                        //In sample SDT table, the name lenther is fixed as 16 bytes
                        //The first byte is Selection of character table, 0x5 is ISO/IEC 8859-9 [33] Latin alphabet No. 5
    Byte ServiceName[16]={0x05,'i','T','E',' ','C','H','1',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
                        //In sample SDT table, the name lenther is fixed as 16 bytes
                        //The first byte is Selection of character table, 0x5 is ISO/IEC 8859-9 [33] Latin alphabet No. 5
    Byte NetworkName[11]={0x05,'i','T','E',' ','T','a','i','p','e','i'};
                        //In sample NIT table, the name lenther is fixed as 11 bytes
                        //The first byte is Selection of character table, 0x5 is ISO/IEC 8859-9 [33] Latin alphabet No. 5
    Byte LCN=0x50;   //Logical Channel Number

    Byte DeliveryParameters[3]={0x00, 0x00,0x00};
    unsigned  CRC_32 = 0;


    //GET TSID & SID from stream file
    Get_PAT_TSID_SID(TsFile,TSid,Sid,FileLength);

    //***************************************************************************************************
    //  Custom SDT Table Insertion
    //***************************************************************************************************
    //Set Sample SDT IN CustomPacket_1
    memcpy(CustomPacket_1+8 ,TSid,2);
    memcpy(CustomPacket_1+13,ONid,2);
    memcpy(CustomPacket_1+0x10,Sid,2);
    memcpy(CustomPacket_1+0x19,ProviderName,16);
    memcpy(CustomPacket_1+0x2A,ServiceName,16);

    CRC_32 = GetCRC32(CustomPacket_1+5, 0x35);
    CustomPacket_1[0x3a] = ((CRC_32 >> 24) & 0xFF);
    CustomPacket_1[0x3b] = ((CRC_32 >> 16) & 0xFF);
    CustomPacket_1[0x3c] = ((CRC_32 >> 8) & 0xFF);
    CustomPacket_1[0x3d] = ((CRC_32) & 0xFF);


    // Set & Copy SDT packets to internal buffer 1

    if (g_ITEAPI_TxSetPeridicCustomPacket(188, CustomPacket_1, 1, options->dev_handle))
    {
        fprintf(stderr, "g_ITEAPI_TxAccessFwPSITable 1 fail\n");
    }

    //printf("Enter the timer interval for custom table packet buffer 1, SDT (in ms, 0 means disabled): ");
    //scanf("%d", &timer_interval);

    timer_interval=20; //Set SDT repetition rate to 20 ms, 

    if (g_ITEAPI_TxSetPeridicCustomPacketTimer(1, timer_interval, options->dev_handle))
    {
        fprintf(stderr, "g_ITEAPI_TxSetFwPSITableTimer  %d fail\n",1);
    }

    return ;

    //The following codes are for programmer's reference to customize other SI tables in table packet buffer 2~5

    //**************************************************************************************************/
    // Custom NIT Table Insertion (set LCN and Delivery descriptor)
    //**************************************************************************************************/

    //Set Sample NIT IN CustomPacket_2
    memcpy(CustomPacket_2+8   ,NETid,2);
    memcpy(CustomPacket_2+0x11,NetworkName,11);
    memcpy(CustomPacket_2+0x1E,TSid,2);
    memcpy(CustomPacket_2+0x20,ONid,2);


    //Set LCN Descriptor:0x83
    memcpy(CustomPacket_2+0x26,Sid,2);
    memcpy(CustomPacket_2+0x29,&LCN,1);


    //Set Delievery Descriptor:0x5a, NOTE:it's in 10Hz, instead of KHz or Hz
    CustomPacket_2[0x2c] = (Byte)((param->Frequency*100 >> 24) & 0xFF);
    CustomPacket_2[0x2d] = (Byte)((param->Frequency*100 >> 16) & 0xFF);
    CustomPacket_2[0x2e] = (Byte)((param->Frequency*100 >> 8) & 0xFF);
    CustomPacket_2[0x2f] = (Byte)((param->Frequency*100) & 0xFF);

    //Bandwidth 000:8M, 001:7,010:6,011:5
    DeliveryParameters[0]|=(abs(param->Bandwidth/1000-8)<<5);
    //Priority,timeslice, MPEFEC, reserved bits
    DeliveryParameters[0]|=0x1f;

    //Constellation:0: QPSK, 1: 16QAM, 2: 64QAM
    DeliveryParameters[1]|=(g_ChannelModulation_Setting.constellation<<6);
    //CR:0: 1/2, 1: 2/3, 2: 3/4, 3:5/6, 4: 7/8
    DeliveryParameters[1]|=(g_ChannelModulation_Setting.highCodeRate);

    //GI:0: 1/32, 1: 1/16, 2: 1/8, 3: 1/4
    DeliveryParameters[2]|=(g_ChannelModulation_Setting.interval<<3);

    //Transmission mode: 0: 2K, 1: 8K  2:4K
    DeliveryParameters[2]|=(g_ChannelModulation_Setting.transmissionMode<<1);

    memcpy(CustomPacket_2+0x30,DeliveryParameters,3);

    //Set Service list descriptror:0x41
    memcpy(CustomPacket_2+0x39,Sid,2);

    CRC_32 = GetCRC32(CustomPacket_1+5, 0x37);
    CustomPacket_2[0x3c] = ((CRC_32 >> 24) & 0xFF);
    CustomPacket_2[0x3d] = ((CRC_32 >> 16) & 0xFF);
    CustomPacket_2[0x3e] = ((CRC_32 >> 8) & 0xFF);
    CustomPacket_2[0x3f] = ((CRC_32) & 0xFF);

    if (g_ITEAPI_TxSetPeridicCustomPacket(188, CustomPacket_2, 2, options->dev_handle))
    {
        fprintf(stderr, "g_ITEAPI_TxAccessFwPSITable 2 fail\n");
    }

    printf("Enter the timer interval for custom packet table 2, NIT (in ms, 0 means disabled): ");
    scanf("%d", &timer_interval);

    if (g_ITEAPI_TxSetPeridicCustomPacketTimer(2, timer_interval, options->dev_handle) )
    {
        fprintf(stderr, "g_ITEAPI_TxSetFwPSITableTimer  %d fail\n",2);
    }

    //***************************************************************************************************
    //  Other Custom Table Insertion
    //***************************************************************************************************

    if (g_ITEAPI_TxSetPeridicCustomPacket(188, CustomPacket_3, 3, options->dev_handle) == 0)
    {
        printf("g_ITEAPI_TxAccessFwPSITable 3 ok\n");
    }
    else
    {
        printf("g_ITEAPI_TxAccessFwPSITable 3 fail\n");
    }
    
    if (g_ITEAPI_TxSetPeridicCustomPacket(188, CustomPacket_4, 4, options->dev_handle) == 0)
    {
        printf("g_ITEAPI_TxAccessFwPSITable 4 ok\n");
    }
    else
    {
        printf("g_ITEAPI_TxAccessFwPSITable 4 fail\n");
    }
    
    if (g_ITEAPI_TxSetPeridicCustomPacket(188, CustomPacket_5, 5, options->dev_handle) == 0)
    {
        printf("g_ITEAPI_TxAccessFwPSITable 5 ok\n");
    }
    else
    {
        printf("g_ITEAPI_TxAccessFwPSITable 5 fail\n");
    }

    for (i=3;i<=5;i++) 
    {
        printf("Enter the timer interval for custom packet %d (in ms, 0 means disabled): ", i);
        scanf("%d", &timer_interval);
        
        if (g_ITEAPI_TxSetPeridicCustomPacketTimer(i, timer_interval, options->dev_handle))
        {
            printf("g_ITEAPI_TxSetFwPSITableTimer  %d fail\n",i);
        }
    }
}

//==================================================================================================
unsigned long GetTickCount()
{
    //struct tms tm;
    //return (times(&tm)*1000)/sysconf(_SC_CLK_TCK); // Return ticks. One tick = 1/1000 seconds.

    //struct timespec ts;
    //gettimeofday(&ts, NULL);
    //printf("%d\n", ts.tv_sec*1000 + ts.tv_nsec/1000);
    //return (ts.tv_sec*1000 + ts.tv_nsec/1000);


    struct timeval tv;
    unsigned long utime;
    gettimeofday(&tv,NULL);
    utime = tv.tv_sec * 1000 + tv.tv_usec/1000;
    //printf("utime: [%u][%d]\n", tv.tv_sec, tv.tv_usec);
    return utime;

    //clock_t tv;
    //printf("%d\n", clock());
    //return clock(); 
}

//==================================================================================================
void ClkDelay(Dword delay)
{
	clock_t goal = delay * (CLOCKS_PER_SEC /1000) + clock();
	while(goal > clock());
}

//==================================================================================================
Dword TxDataOutputTest(ModulatorParam *param, options_t *options)
{
    int input = 0;
    Handle TsFile = NULL;
    intmax_t dwFileSize = 0;
    Byte *fileBuf = NULL;
    Dword Tx_datalength = TRANSMITTER_BLOCK_SIZE;
    int i = 0, k = 0;
    int stdin_used = 0;

    Dword LoopStartTime = 0;
    ULONGLONG FirstSyncBytePos=0;
    ULONGLONG bytesRead,Total_bytesSent;
    Bool Play_InfiniteLoop = False;
    ULONGLONG TSFileDataRate;
    int LoopCnt=0;

    //Dword Txbytes_PerMs = 0;
    //Dword DelayTime_PerTx = 0;

    Dword TxStartTime = 0;
    //Dword PCRAdaptationOffset = 0;
    char key;
    Dword ret;
    ULONGLONG Output_Data_Rate;

    struct timespec req = {0};
    req.tv_sec = 0;     
    req.tv_nsec = 1;    

    if (strcmp("-", options->ts_filename) == 0)
    {
        fprintf(stderr, "Using stdin TS\n");
        TsFile = stdin;
        stdin_used = 1;
    }
    else if(!(TsFile = fopen(options->ts_filename, "r+"))) 
    {
        fprintf(stderr, "TxDataOutputTest: Open TS File Error!\n");
        return ERR_OPEN_FILE_FAIL;
    }

    if (TsFile == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Open Ts File invalid handle value!\n");
        return INVALID_HANDLE_VALUE;
    }

    if (!stdin_used)
    {
        dwFileSize = GetFileSize(options->ts_filename);
        fprintf(stderr, "%s size = %9jd\n", options->ts_filename, dwFileSize);
    }

    if (options->insert_sisdt_custom_packets&&!stdin_used)
    {
        SetPeriodicCustomPacket(TsFile, dwFileSize, param, options);
    }
    else
    {
        // Disable Custom table packet insertion 
        for (i=1;i<=5;i++) 
        {
            if (g_ITEAPI_TxSetPeridicCustomPacketTimer(i, 0, options->dev_handle))
            {
                fprintf(stderr, "g_ITEAPI_TxSetFwPSITableTimer  %d fail\n",i);
            }
        }
    }
    
    // Check input file data bit rate here, slow but safe 
    if (!stdin_used)
    {
        TSFileDataRate = Get_DataBitRate_S(options);
        fprintf(stderr, "The recommended input file data rate for %s is = %llu bps (%llu kbps)\n", options->ts_filename, TSFileDataRate, TSFileDataRate/1000);
    }
    
    if (options->ts_data_rate>0)
    {
        fprintf(stderr, "Input file data rate set forcibly to %llu bps (%llu kbps)\n", options->ts_data_rate, options->ts_data_rate/1000);
        TSFileDataRate = options->ts_data_rate;
    }

    if (TSFileDataRate > g_ChannelCapacity)
    {
        fprintf(stderr, "Warning the input file data rate (%llu bps) is larger than channel data rate(%lu bps)\n",TSFileDataRate,g_ChannelCapacity);
    }

    fileBuf = (Byte *)malloc(Tx_datalength);
    
    // Find the first 0x47 sync byte 
    fseek(TsFile, 0, SEEK_SET);
    bytesRead = fread(fileBuf, 1, Tx_datalength, TsFile);

    for (FirstSyncBytePos=0;(FirstSyncBytePos<bytesRead-188);FirstSyncBytePos++) 
    {
        if ((fileBuf[FirstSyncBytePos] == 0x47)&&(fileBuf[FirstSyncBytePos+188] == 0x47))
        {
            //Sync byte found
            fprintf(stderr, "TS Sync byte found in offset:%llu\n",FirstSyncBytePos);
            break;
        }
    }

    if (FirstSyncBytePos >= bytesRead-188) 
    {
        fprintf(stderr, "No sync byte found, it's not a valid 188-byte TS file!!!\n");
        goto TX_DataOutputTest_exit;
    }
    
    Total_bytesSent=0;
    LoopCnt=0;

    fseek(TsFile, FirstSyncBytePos, SEEK_SET);
    LoopStartTime =TxStartTime= GetTickCount();
    fprintf(stderr, "LoopStartTime: %lu\n", LoopStartTime);
    fprintf(stderr, "Loop Repeat: %d\n", LoopCnt);
    g_ITEAPI_StartTransfer(options->dev_handle);

    memset(fileBuf, 0, Tx_datalength);
    bytesRead = fread(fileBuf, 1, Tx_datalength, TsFile);
    ret = g_ITEAPI_TxSendTSData((Byte*)fileBuf, bytesRead, options->dev_handle);
    
    TxStartTime = GetTickCount();

    while(!gDoExit)
    {
        //memset(fileBuf, 0, Tx_datalength);
        bytesRead = fread(fileBuf, 1, Tx_datalength, TsFile);
    
rewrite_case:
        ret = g_ITEAPI_TxSendTSData((Byte*)fileBuf, bytesRead, options->dev_handle);
    
        if (ret != ERR_NO_ERROR) // RB is full. Try wait and rewrite until success.
        {     
            usleep(10000);
            fprintf(stderr, "Overflow\n");
            goto rewrite_case;
        } 
        
        
        Total_bytesSent+=bytesRead;

        // The end of file condition 
        if (bytesRead < Tx_datalength)
        {
            fprintf(stderr, "End of file reached..Total loop time: %lu ms\n", GetTickCount()-LoopStartTime);
            
            if(options->ts_loop)
            {
                //Repeat Loop 
                g_ITEAPI_StopTransfer(options->dev_handle);
                usleep(3000000);
                g_ITEAPI_StartTransfer(options->dev_handle);
                fseek(TsFile, FirstSyncBytePos, SEEK_SET);
                LoopStartTime= GetTickCount();
                //PCRAdaptationOffset=(LoopStartTime-TxStartTime) * 90 / 2; //Change system time offset(in ms) to PCR's tick count offset (in 45KHz)
                Total_bytesSent=0;
                LoopCnt++;
                printf("Loop Repeat: %d\n", LoopCnt);
            }
            else
            {
                break;
            }
        }

        // Data Rate Control, IF the TS data rate is less than channel modulation data rate 
        // and excluding the first time check (divide by zero check)                        
        
        while ((TSFileDataRate < g_ChannelCapacity-1024) && Total_bytesSent)
        {
            Output_Data_Rate=0;
            
            if (GetTickCount() == LoopStartTime) 
            {
                continue;
            }
        
            Output_Data_Rate= Total_bytesSent*8/(GetTickCount()-LoopStartTime)*1000;
            
            if (Output_Data_Rate <= TSFileDataRate)
            {
                break;
            }
            else
            {
                nanosleep(&req, &req);
            }
        }
    }

TX_DataOutputTest_exit:
    g_ITEAPI_StopTransfer(options->dev_handle);
    if(fileBuf) free (fileBuf);
    if(TsFile) fclose(TsFile);
    return ret;
}

//==================================================================================================
int TxOutOnOff(Byte DevNo)
{
	int cho; //, ret;
	Dword dwStatus = 0;
	
	printf("\n=> Please Input Tx mode ON or OFF (0:Off  1:On): ");
	scanf("%d", &cho);
	
	if (cho == 1)
		dwStatus = g_ITEAPI_TxSetModeEnable(1, DevNo);
	else if (cho == 0)
		dwStatus = g_ITEAPI_TxSetModeEnable(0, DevNo);
	else {
		dwStatus = 1;
		printf("\nInvaild value !!, %d", cho);
	}
	
	if (dwStatus)
		printf("\nSetTxMode Error!!, %lu", dwStatus);
	else
		printf("\n******  SetTxMode %s DONE  *******\n\n", cho?"On":"Off");

	return 0;
}

//==================================================================================================
int TxAutoChangeModule(Byte DevNo)
{
	int i, k = 0;
	int ct, it, cr;
	Dword dwStatus = 0;
	long fileLength = 0;
	char *fileBuf = NULL;
	FILE *fp = NULL;
	int writeLen = TX_DATA_LEN;
	struct timeval start, end;
	uint32_t diff;
	int dwRead = 0;
	int repeatCount;
	int ret = 0;
	int txStart = 0;
	MODULATION_PARAM ChannelModulation_Setting;	
	
	//for (tm = 1; tm >= 0; tm--) {
		for(ct = 0; ct < 3; ct++) {
			for (it = 0; it < 4; it++) {
				for (cr = 0; cr < 5; cr++) {
						//printf("\ntransmissionMode: %d, constellation: %d, interval: %d, highCodeRate: %d\n", 1, ct, it, cr);
						ChannelModulation_Setting.transmissionMode = 1;
						ChannelModulation_Setting.constellation = ct;
						ChannelModulation_Setting.interval = it;
						ChannelModulation_Setting.highCodeRate = cr;
						dwStatus = g_ITEAPI_TxSetChannelModulation(ChannelModulation_Setting, DevNo);
						if (dwStatus)
							printf("\nSetModule Error!!, %lu", dwStatus);
						else
							printf("\n******  SetModule DONE  ******\n\n");
						
						if (txStart == 0) {
							fileLength = GetFileSize("/home/encode_Produce.ts");	
							fileBuf = (char *)malloc(writeLen); 
							if (!(fp = fopen("/home/encode_Produce.ts", "r+"))) {
								printf("File open fail!");
								return -1;
							}
							printf("File Length is %ld, Tx Length is %d\n", fileLength, writeLen);
							g_ITEAPI_StartTransfer(DevNo);
							txStart = 1;
						}
						
						repeatCount = 3;
						while (repeatCount) {
							for (i = 0; i < (fileLength / writeLen); i++) {
								gettimeofday(&start, NULL);
								dwRead = fread(fileBuf, 1, writeLen, fp);
rewrite:
								ret = g_ITEAPI_TxSendTSData((Byte*)fileBuf, writeLen, DevNo);
								if (ret == 0 && i != ((fileLength / writeLen) -1)) {
									usleep(50);
									//printf("rewrite\n");			
									goto rewrite;		
								} else if (ret < 0 || ret == ERR_NOT_IMPLEMENTED) {
									usleep(100000);
									printf("write fail\n");			
									k++;
									if (k>50) {
										break; // 5sec
									}
									goto rewrite;	
								}
								
								gettimeofday(&end, NULL);	
								diff = TxTransferTimeDelay(start, end);
								if (diff > 0 && diff <= gTransferInterval && gRateControl)
									usleep(diff * 1000);
								
							}
							
							fseek(fp, 0, SEEK_SET);
							repeatCount--;
							printf("Remaining Repeat times = %d\n",repeatCount);
						}	
				}	
			}
		}
		
		fclose(fp);
		free(fileBuf);

		g_ITEAPI_StopTransfer(DevNo);
		txStart = 0;
	//}
	return 0;
}

//==================================================================================================
int main(int argc, char **argv)
{
    struct sigaction sigact;
    int chose, adjustoutputgain, outgainvalue;
    int closeFlag = 0;
    Dword ret;
    Dword tempValue = 0;
    Dword frequency = 0;
    Dword bandwidth = 0;
    Dword ChannelCapacity = 0;
    int MaxGain = 0;
    int MinGain = 0;
    int Gain = 0;
    Byte DevNo;
    ModulatorParam param;
    int dc_i, dc_q;
    HandleType handleType;
    handleType=EAGLEI; // What is other type ? 
    memset(&param, 0, sizeof(param));
    options_t options;
    
    get_options(argc, argv, &options);
    print_options(&options);
    
    ChannelCapacity = GetChannelCapacity(&options);
    fprintf(stderr, "The Maximum Channel Capacity is %lu bps (%lu Kbps)\n", ChannelCapacity, ChannelCapacity/1000);
    fprintf(stderr, "\n");

    if (GetDriverInfo(handleType,options.dev_handle) != ERR_NO_ERROR)
    {
        return 0;
    }

    sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);
    
    g_ChannelCapacity = TxSetChannelTransmissionParameters(&param, &options);
    setGain(&param, &options);
    
    if (options.print_device_type)
    {
        printDeviceType(&options,DevNo);
    }

    if (options.tps_cellid_specified)
    {
        setTPSCellId(&options);
    }
    
    if (options.iq_table_filename)
    {
        SetIQCalibrationTable(&options);
    }
    
    if (options.dc_iq_specified)
    {
        if(g_ITEAPI_TxSetDCCalibrationValue(options.dc_i, options.dc_q, options.dev_handle) == 0)
        {
            fprintf(stderr, "g_ITEAPI_TxSetDCCalibrationValue ok\n");
        }
        else
        {
            fprintf(stderr, "g_ITEAPI_TxSetDCCalibrationValue fail\n");
        }
    }

    print_options(&options);
    
    if (options.ts_filename)
    {
        int ret;
        
        gRateControl = 1;
        g_ITEAPI_TxSetModeEnable(True, options.dev_handle);
        ret = TxDataOutputTest(&param, &options);
        g_ITEAPI_TxSetModeEnable(False, options.dev_handle);
        
        if(ret < 0)
        {
            fprintf(stderr, "TX_DataOutputTest error(0x%X)\n", (Byte) ret);
        }
    }

    g_ITEAPI_TxDeviceExit(options.dev_handle);
    
    delete_options(&options);

    return 0;
}
