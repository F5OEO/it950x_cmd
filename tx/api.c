/**
 * Copyright (c) 2015 ITE Technologies, Inc. All rights reserved.
 * 
 * Date:
 *    2015/01/14
 *
 * Module Name:
 *    api.c
 *
 * Abstract:
 *    Defines the entry point for the DLL application.
 */

#include "api.h"

int g_hDriver[16][2] = {{0}};           // Support number of 16 devices.
int g_hSPIDriverHandle = 0;
int g_hPrimaDriverHandle = 0;

/*********************************************************************************
 * The client calls this function to get USB device handle.
 * @param void
 * Return:  Dword if successful, INVALID_HANDLE_VALUE indicates failure.
 *********************************************************************************/
Dword g_ITEAPI_TxDeviceOpen(
	IN HandleType handleType,
	IN Byte DevNo)
{
	int hDriver = 0, ret = 0;
	char* devName = "";
	
	switch(handleType){
		case EAGLEI:
			ret = asprintf(&devName, "/dev/usb-it950x%d", DevNo);
			g_hDriver[DevNo][1] = EAGLEI;
			break;
		
		case EAGLEII:
			ret = asprintf(&devName, "/dev/usb-it951x%d", DevNo);
			g_hDriver[DevNo][1] = EAGLEII;			
			break;

		default:
			printf("Handle type error[%d]\n", handleType);
			return ERR_USB_INVALID_HANDLE;
	}

    hDriver = open(devName, O_RDWR);
	if (hDriver <= INVALID_HANDLE_VALUE) {
		printf("\nOpen %s fail\n", devName);	
		return ERR_USB_INVALID_HANDLE;
	} else {
		printf("\nOpen %s ok\n", devName);	
	}

	return hDriver;
}

/*********************************************************************************
 * The client calls this function to close device.
 * @param   device handle
 * Return:  TRUE indicates success. FALSE indicates failure. 
 ********************************************************************************/
Dword g_ITEAPI_TxDeviceExit(
    IN  Byte DevNo)
{
    return (close(DevNo));
}

Dword g_ITEAPI_TxDeviceInit(
	IN HandleType handleType, 
	IN Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    DEVICE_INFO DeviceInfo;
	
    g_hDriver[DevNo][0] = g_ITEAPI_TxDeviceOpen(handleType, DevNo);

    if (g_hDriver[DevNo][0] == INVALID_HANDLE_VALUE){
        dwError = ERR_USB_INVALID_HANDLE;
		return dwError; 
	}

    /* Check driver is loaded correctly */
    dwError = g_ITEAPI_TxGetDrvInfo(&DeviceInfo, DevNo);
	if(dwError != ERR_NO_ERROR) {
		dwError = ERR_INVALID_CHIP_REV;		
		return dwError;
	}

	return (dwError);
}


Dword g_ITEAPI_TxGetDrvInfo(
    OUT PDEVICE_INFO pDeviceInfo,
    IN  Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    TxModDriverInfo request;
	Word ChipType = 0;
    int result;

	/* Get DriverVerion and others information */
    if (g_hDriver[DevNo][0] > 0) {
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETDRIVERINFO, (void *)&request);
        dwError = (Dword) request.error;
        if(dwError != ERR_NO_ERROR) {
			dwError = ERR_INVALID_CHIP_REV;		
			return dwError;
		}
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }
	
	/* Get Chip Type */
	dwError = g_ITEAPI_TxGetChipType(&ChipType, DevNo);
	if(dwError != ERR_NO_ERROR) {
        dwError = ERR_INVALID_CHIP_REV;		
        return dwError;
	}

	memcpy(pDeviceInfo->DriverVerion, &request.DriverVerion, sizeof(Byte)*16);
	memcpy(pDeviceInfo->APIVerion, &request.APIVerion, sizeof(Byte)*32);
	memcpy(pDeviceInfo->FWVerionLink, &request.FWVerionLink, sizeof(Byte)*16);
	memcpy(pDeviceInfo->FWVerionOFDM, &request.FWVerionOFDM, sizeof(Byte)*16);
	memcpy(pDeviceInfo->DateTime, &request.DateTime, sizeof(Byte)*24);
	memcpy(pDeviceInfo->Company, &request.Company, sizeof(Byte)*8);
	memcpy(pDeviceInfo->SupportHWInfo, &request.SupportHWInfo, sizeof(Byte)*32);
	memcpy(&pDeviceInfo->ProductID, &ChipType, sizeof(Word));
	
    return dwError;	
}

Dword g_ITEAPI_TxPowerCtl(
    IN  Byte byCtrl,
    IN  Byte DevNo)
{
	
    Dword dwError = ERR_NO_ERROR;
    int result = 0;
    TxControlPowerSavingRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.control = byCtrl;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_CONTROLPOWERSAVING, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }
	
    return dwError;
}

Dword g_ITEAPI_TxSetChannelModulation(	
	IN MODULATION_PARAM ChannelModulation_Setting,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetModuleRequest request;
	Byte transmissionMode = ChannelModulation_Setting.transmissionMode;
	Byte constellation = ChannelModulation_Setting.constellation;
	Byte interval = ChannelModulation_Setting.interval;
	Byte highCodeRate = ChannelModulation_Setting.highCodeRate;

	if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
		request.transmissionMode = transmissionMode;
        request.constellation = constellation;
		request.interval = interval;
		request.highCodeRate = highCodeRate;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETMODULE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxSetChannel(	
	IN Dword bfrequency,
	IN Word bandwidth,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxAcquireChannelRequest request;

	if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
		request.frequency = (__u32) bfrequency;
        request.bandwidth = bandwidth;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ACQUIRECHANNEL, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxSetModeEnable(
	IN Byte OnOff, 
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxModeRequest request;
	
	if (g_hDriver[DevNo][0] > 0) {
        request.OnOff = OnOff;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ENABLETXMODE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxSetDeviceType(
	IN Byte DeviceType, 
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetDeviceTypeRequest request;

	if (g_hDriver[DevNo][0] > 0) {
		request.DeviceType = DeviceType;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETDEVICETYPE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxGetDeviceType(
	OUT Byte *DeviceType,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxGetDeviceTypeRequest request;

	if (g_hDriver[DevNo][0] > 0) {
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETDEVICETYPE, (void *)&request);
        *DeviceType = request.DeviceType;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxAdjustOutputGain(
	IN int Gain_value,
	OUT int *Out_Gain_value,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetGainRequest request;
    
	if (g_hDriver[DevNo][0] > 0) {
		request.GainValue = Gain_value;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ADJUSTOUTPUTGAIN, (void *)&request);
        *Out_Gain_value = request.GainValue;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxControlPidFilter(
	Byte control,
	Byte enable,
	IN Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxControlPidFilterRequest request;

    //RETAILMSG( 1, (TEXT("g_ITEAPI_EnablePIDTbl\n\r") ));

    if (g_hDriver[DevNo][0] > 0) {
        request.control = control;
        request.enable = enable;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_CONTROLPIDFILTER, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return(dwError);
}

Dword g_ITEAPI_TxAddPID(
    IN Byte byIndex,            // 0 ~ 31
	IN Word wProgId,            // pid number
	IN Byte DevNo)
{	
    Dword dwError = ERR_NO_ERROR;
    int result;
    AddPidAtRequest request;
    Pid pid;
	memset(&pid, 0, sizeof(pid));
    pid.value = (Word)wProgId;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.pid = pid;
        request.index = byIndex;

        //RETAILMSG( 1, (TEXT("g_ITEAPI_AddPID - Index = %d, Value = %d\n\r"), request.index, request.pid.value));

        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ADDPIDAT, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return (dwError);
}

Dword g_ITEAPI_TxResetPidFilter(
	IN Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ResetPidRequest request;

    //RETAILMSG( 1, (TEXT("g_ITEAPI_ResetPIDTable\n\r")));

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_RESETPID, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return(dwError);
}

Dword g_ITEAPI_StartTransfer(
	IN Byte DevNo)
{

	Dword dwError = ERR_NO_ERROR;
    int result;

    //here tell driver begin to read data
    result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_STARTTRANSFER);

    return(dwError);
}

Dword g_ITEAPI_StartTransfer_CMD(
	IN Byte DevNo)
{

	Dword dwError = ERR_NO_ERROR;
    int result;

    //here tell driver begin to read data
    result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_STARTTRANSFER_CMD);

    return(dwError);
}

Dword g_ITEAPI_StopTransfer(
	IN Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;

    result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_STOPTRANSFER);

    return(dwError);
}

Dword g_ITEAPI_StopTransfer_CMD(
	IN Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;

    result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_STOPTRANSFER_CMD);

    return(dwError);
}

Dword g_ITEAPI_TxSendTSData(
    OUT Byte* pBuffer,
    IN  OUT Dword pdwBufferLength,
    IN  Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
	int Len = 0;
	
    if (g_hDriver[DevNo][0] > 0) {
		Len =  write(g_hDriver[DevNo][0], pBuffer, pdwBufferLength);
    }
    else {
        dwError = -ERR_NOT_IMPLEMENTED;
    }

    return Len;
}

Dword g_ITEAPI_TxAddPIDEx(
    IN  DTVPid pid,
    IN  Byte DevNo)
{	
    Dword dwError = ERR_NO_ERROR;
    int result;
    AddPidAtRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        memcpy(&request.pid, &pid, sizeof(Pid));
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ADDPIDAT, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return (dwError);
}

Dword g_ITEAPI_TxReadRegOFDM(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData,
    IN  Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxReadRegistersRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.processor = Processor_OFDM;
        request.registerAddress = (__u32) dwRegAddr;
        request.bufferLength = 1;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_READREGISTERS, (void *)&request);
		*pbyData = request.buffer[0];
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxWriteRegOFDM(
    IN  Dword dwRegAddr,
    IN  Byte byData,
    IN  Byte DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxWriteRegistersRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.processor = Processor_OFDM;
        request.registerAddress = (__u32) dwRegAddr;
        request.bufferLength = 1;
        memcpy (request.buffer, &byData, 1);
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_WRITEREGISTERS, (void *)&request);
        dwError = (Dword) request.error;
    } else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxReadRegLINK(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;

    int result;
    TxReadRegistersRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.processor = Processor_LINK;
        request.registerAddress = (__u32) dwRegAddr;
        request.bufferLength = 1;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_READREGISTERS, (void *)&request);
		*pbyData = request.buffer[0];
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxWriteRegLINK(
    IN  Dword dwRegAddr,
    IN  Byte  byData,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxWriteRegistersRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.processor = Processor_LINK;
        request.registerAddress = (__u32) dwRegAddr;
        request.bufferLength = 1;
        memcpy (request.buffer, &byData, 1);
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_WRITEREGISTERS, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxWriteEEPROM(
    IN  Word  wRegAddr,
    OUT Byte  byData,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxWriteEepromValuesRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.registerAddress = wRegAddr;
        request.bufferLength = 1;
        memcpy (request.buffer, &byData, 1);
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_WRITEEEPROMVALUES, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxReadEEPROM(
    IN  Word  wRegAddr,
    OUT Byte* pbyData,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxReadEepromValuesRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        request.chip = 0;
        request.registerAddress = wRegAddr;
        request.bufferLength = 1;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_READEEPROMVALUES, (void *)&request);
		*pbyData = request.buffer[0];
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxGetGainRange(
	IN Dword frequency,
	IN Word bandwidth,
	OUT int *MaxGain,
	OUT int *MinGain,
	IN  Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    Dword upperBound, lowerBound;
    TxGetGainRangeRequest request;

	if(g_hDriver[DevNo][1] == EAGLEI) {	//EAGLEI 
		lowerBound = 173000;
		upperBound = 9000000;
	} else {								// EAGLEII
		lowerBound = 93000;
		upperBound = 8030000;		
	}

	if (g_hDriver[DevNo][0] > 0) {
		if(frequency >= lowerBound && frequency <= upperBound)
			request.frequency = frequency;
		else {
			printf("\nSet Frequency Out of Range!\n");
			dwError = ERR_FREQ_OUT_OF_RANGE;
		}
		if(bandwidth >= 1000 && bandwidth <= 8000)
			request.bandwidth = bandwidth;
		else {
			printf("\nSet Bandwidth Out of Range!\n");
			dwError = ERR_INVALID_BW;
		}
		if(!dwError) {
			result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETGAINRANGE, (void *)&request);
			*MaxGain = (int) request.maxGain;
			*MinGain = (int) request.minGain;
			dwError = (Dword) request.error;
		}
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxGetTPS(
	OUT TPS *tps,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxGetTPSRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETTPS, (void *)&request);
		*tps = (TPS) request.tps;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxSetTPS(
	IN TPS tps,
	IN Byte DevNo)	
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetTPSRequest request; 
      
	if (g_hDriver[DevNo][0] > 0) {
		request.tps = tps;
		request.actualInfo = True;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETTPS, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxGetOutputGain(
	OUT int *gain,
	IN  Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxGetOutputGainRequest request; 
      
	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETOUTPUTGAIN, (void *)&request);
		*gain = request.gain;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword g_ITEAPI_TxGetNumOfDevice(
	OUT Byte *NumOfDev,
	IN  Byte DevNo	
)
{
	Dword dwError = ERR_NO_ERROR, ret;
    struct dirent *ptr;
    char *handle = "", *existing;
    DIR *dir = opendir("/dev");

	switch(g_hDriver[DevNo][1]){
		case EAGLEI:
			ret = asprintf(&handle, "usb-it950x");
			break;
		
		case EAGLEII:
			ret = asprintf(&handle, "usb-it951x");
			break;

		default:
			printf("Handle type error[%d]\n", g_hDriver[DevNo][1]);
			dwError = ERR_USB_INVALID_HANDLE;
			return ERR_USB_INVALID_HANDLE;
	}
    
    while((ptr = readdir(dir)) != NULL) {
		existing = strndup(ptr->d_name, 10);
		if(!strcmp(existing, handle))
			(*NumOfDev)++;
	}
	closedir(dir);
    return (dwError);
}

Dword g_ITEAPI_TxSendCustomPacketOnce(
	IN int bufferSize,
	IN Byte *TableBuffer,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSendHwPSITableRequest request; 

	if(bufferSize != 188)
		return -1;

	if (g_hDriver[DevNo][0] > 0) {
		request.pbufferAddr = (__u32) TableBuffer;	
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SENDHWPSITABLE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}
 
Dword g_ITEAPI_TxSetPeridicCustomPacket(
	IN int bufferSize,
	IN Byte *TableBuffer,
	IN Byte index,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxAccessFwPSITableRequest request; 

	if(bufferSize != 188)
		return -1;

	if (g_hDriver[DevNo][0] > 0) {
		request.psiTableIndex = index;
		request.pbufferAddr = (__u32) TableBuffer;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ACCESSFWPSITABLE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxSetPeridicCustomPacketTimer(
	IN Byte index,
	IN Word timer_interval,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetFwPSITableTimerRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.psiTableIndex = index;
		request.timer = timer_interval;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETFWPSITABLETIMER, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxSetIQTable(
	IN Byte* ptrIQtable,
	IN Word	 IQtableSize,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetIQTableRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.pIQtableAddr = (__u32) ptrIQtable;
		request.IQtableSize = IQtableSize;		
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETIQTABLE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxGetChipType(
	OUT Word* chipType,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxGetChipTypeRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETCHIPTYPE, (void *)&request);
		*chipType = request.chipType;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxSetDCCalibrationValue(
	IN int dc_i,
	IN int dc_q,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetDCCalibrationValueRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.dc_i = dc_i;
		request.dc_q = dc_q;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETDCCALIBRATIONVALUE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxWriteLowBitRateData(
	IN Byte* pBuffer,
	IN Dword BufferLength,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetLowBitRateTransferRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.pBufferAddr = (__u32) pBuffer;
		request.pdwBufferLength = (__u32) &BufferLength;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_WRITE_LOWBITRATEDATA, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxWriteCmd(
	IN Word			len,
    IN Byte*			cmd,
	IN Byte DevNo    
)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxCmdRequest request; 
	
	if (g_hDriver[0] > 0) {
		request.len = len;
		request.cmdAddr = (__u32) cmd;

		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_WRITE_CMD, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxSetISDBTChannelModulation(
	IN ISDBTModulation isdbtModulation,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TXSetISDBTChannelModulationRequest request; 
	ISDBTModulation* isdbtModulationUser = (ISDBTModulation*) malloc(sizeof(ISDBTModulation));

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}
	
	memcpy(isdbtModulationUser, &isdbtModulation, sizeof(ISDBTModulation));
	if (g_hDriver[DevNo][0] > 0) {
		request.isdbtModulationAddr = (__u32) isdbtModulationUser;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETISDBTCHANNELMODULATION, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
	free(isdbtModulationUser);
    return (dwError);
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxSetTMCCInfo(
    IN  TMCCINFO      TmccInfo,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TXSetTMCCInfoRequest request; 

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}

	if (g_hDriver[DevNo][0] > 0) {
		request.TmccInfo = TmccInfo;
		request.actualInfo = True;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETTMCCINFO, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxGetTMCCInfo(
    OUT  pTMCCINFO     pTmccInfo,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TXGetTMCCInfoRequest request; 

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}

	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETTMCCINFO, (void *)&request);
		*pTmccInfo = request.TmccInfo;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxGetTSinputBitRate(
    OUT  Word*     BitRate_Kbps,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TXGetTSinputBitRateRequest request; 

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}
	
	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETTSINPUTBITRATE, (void *)&request);
		*BitRate_Kbps = request.BitRate_Kbps;
        dwError = request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);	
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxAddPidToISDBTPidFilter(
    IN  Byte            index,
    IN  Pid             pid,
	IN	TransportLayer  layer,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TXAddPidToISDBTPidFilterRequest request; 

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}

	if (g_hDriver[DevNo][0] > 0) {
		request.index = index;
		request.pid = pid;
		request.layer = layer;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ADDPIDTOISDBTPIDFILTER, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);	
}

/* Only for EAGLEII */
Dword g_ITEAPI_TxSetPCRMode(
    IN  PcrMode		mode,
	IN  Byte			DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetPcrModeRequest request; 

	if(g_hDriver[DevNo][1] == EAGLEI) {		/* Only suppurted in EAGLEII*/
        dwError = ERR_NOT_IMPLEMENTED;		
		return (dwError);
	}

	if (g_hDriver[DevNo][0] > 0) {
		request.mode = mode;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETPCRMODE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);	
}

Dword g_ITEAPI_TxSetDCTable(
    IN  DCInfo			pDCInfo,
	IN  Byte			DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetDCTableRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.DCInfoAddr = (__u32) &pDCInfo;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETDCTABLE, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);	
}

Dword g_ITEAPI_GetFrequencyIndex(
	OUT Byte* frequencyindex,
	IN Byte DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxGetFrequencyIndexRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETFREQUENCYINDEX, (void *)&request);
		*frequencyindex = request.frequencyindex;
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}

Dword g_ITEAPI_TxGetDTVMode(
    OUT Byte* DTVMode,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;

    int result;
    TxGetDTVModeRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_GETDTVMODE, (void *)&request);
		*DTVMode = request.DTVMode;
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxEnableTPSEncryption(
    IN  Dword key,
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;

    int result;
    TxEnableTpsEncryptionRequest request;

    if (g_hDriver[DevNo][0] > 0) {
		request.key = (__u32) key;
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_ENABLETPSENCRYPTION, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxDisableTPSEncryption(
    IN  Byte  DevNo)
{
    Dword dwError = ERR_NO_ERROR;

    int result;
    TxDisableTpsEncryptionRequest request;

    if (g_hDriver[DevNo][0] > 0) {
        result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_DISABLETPSENCRYPTION, (void *)&request);
        dwError = (Dword) request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword g_ITEAPI_TxSetSpectralInversion(
    IN	Bool	isInversion,
	IN	Byte	DevNo)
{
	Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetSpectralInversionRequest request; 

	if (g_hDriver[DevNo][0] > 0) {
		request.isInversion = isInversion;
		result = ioctl(g_hDriver[DevNo][0], IOCTL_ITE_MOD_SETSPECTRALINVERSION, (void *)&request);
        dwError = (Dword) request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}
    return (dwError);
}
