/**
 * Copyright (c) 2015 ITE Technologies, Inc. All rights reserved.
 * 
 * Date:
 *    2015/01/14
 *
 * Module Name:
 *    api.cpp
 *
 * Abstract:
 *    Defines the entry point for the DLL application.
 */

#include "api.h"

int g_hDriver = 0;
int g_hSPIDriverHandle = 0;
int g_hPrimaDriverHandle = 0;



//===========================================================================
// The client calls this function to get SDIO/USB/SPI device handle.
// @param void
// Return:  int if successful, INVALID_HANDLE_VALUE indicates failure.
//===========================================================================
int GetDriverHandle(Byte handleNum)
{	
	int hDriver = 0;
	char* devName = "";
	int ret;

	ret = asprintf(&devName, "/dev/usb-it950x-rx%d", handleNum);
    hDriver = open(devName, O_RDWR);
	if (hDriver > 0)
		printf("\nOpen /dev/usb-it950x-rx%d OK!\n", handleNum);
	else
		printf("\nOpen /dev/usb-it950x-rx%d Failed!\n", handleNum);

	return hDriver;
}


//===========================================================================
// The client calls this function to close Af9015 device.
// @param   Af9015 handle
// Return:  TRUE indicates success. FALSE indicates failure. 
//===========================================================================
int CloseDriverHandle(
    IN  int hObject)
{
    //here tell driver stop to read data
    //DTV_StopCapture();

    return (close(hObject));
}

Dword DTV_Initialize(Byte handleNum)
{

    Dword dwError = ERR_NO_ERROR;
    DemodDriverInfo DriverInfo;
	
    g_hDriver = GetDriverHandle(handleNum);
    //DTV_ControlPowerSaving(1);

    // Check driver is loaded correctly
    dwError = DTV_GetVersion(&DriverInfo);

    if (g_hDriver == INVALID_HANDLE_VALUE){
		printf("\nINVALID_HANDLE_VALUE fail\n");
        dwError = ERR_INVALID_DEV_TYPE;
}
    if (g_hDriver < INVALID_HANDLE_VALUE){
        dwError = ERR_INVALID_DEV_TYPE;
}
	return (dwError);
}


Dword DTV_Finalize()
{
    Dword dwError = ERR_NO_ERROR;

    //DTV_ControlPowerSaving(0);
    //DTV_ResetPIDTable();
    //DTV_DisablePIDTbl();

    if (g_hDriver) {
		CloseDriverHandle(g_hDriver);
	}
	
    return (dwError);
}


//
// ucCtrl ( 1 : Power Up, 0 : Power Down )
//
Dword DTV_ControlPowerSaving(
    IN  Byte byCtrl)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ControlPowerSavingRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.control = byCtrl;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_CONTROLPOWERSAVING, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }
	
    return dwError;
}

Dword DTV_AcquireChannel(
    IN  Dword dwFrequency,                // Channel Frequency (KHz)
    IN  Word  wBandwidth)                 // Channel Bandwidth (KHz)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    AcquireChannelRequest request;
	
    if (wBandwidth < 10)
        wBandwidth *= 1000;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.frequency = dwFrequency;
        request.bandwidth = wBandwidth;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_ACQUIRECHANNEL, (void *)&request);
        dwError = request.error;
	}
	else {
        dwError = ERR_NOT_IMPLEMENTED;
	}

    return (dwError);
}

Dword DTV_IsLocked(
    OUT Bool* pbLocked)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    IsLockedRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_ISLOCKED, (void *)&request);
        *pbLocked = request.locked;
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return (dwError);
}


Dword DTV_EnablePIDTbl(void)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ControlPidFilterRequest request;

    //RETAILMSG( 1, (TEXT("DTV_EnablePIDTbl\n\r") ));

    if (g_hDriver > 0) {
        request.chip = 0;
        request.control = 1;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_CONTROLPIDFILTER, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return(dwError);
}


Dword DTV_DisablePIDTbl(void)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ControlPidFilterRequest request;

    //RETAILMSG( 1, (TEXT("DTV_DisablePIDTbl\n\r") ));

    if (g_hDriver > 0) {
        request.chip = 0;
        request.control = 0;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_CONTROLPIDFILTER, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return(dwError);
}

Dword DTV_ResetPIDTable()
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ResetPidRequest request;

    //RETAILMSG( 1, (TEXT("DTV_ResetPIDTable\n\r")));

    if (g_hDriver > 0) {
        request.chip = 0;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_RESETPID, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return(dwError);
}

Dword DTV_GetStatistic(
    Word  wNumOfPacket,     //number of RSD packets
    Dword* pdwPostErrCnt,   // ErrorBitCount
    Dword* pdwPostBitCnt,   // TotalBitCount
    Word*  pwAbortCnt,      // number of abort RSD packet
    Word*  pwQuality,       // signal quality (0 - 100)
    Word*  pwStrength)      // signal strength (0 - 100)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    GetStatisticRequest request;
    Statistic statistic;
	memset(&statistic, 0, sizeof(statistic));      
    if (g_hDriver > 0) {
        request.chip = 0;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETSTATISTIC, (void *)&request);
        statistic = (Statistic) request.statistic;
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    *pwQuality     = statistic.signalQuality;
    *pwStrength    = statistic.signalStrength;

    GetChannelStatisticRequest request2;
    ChannelStatistic* channelStatistic = (ChannelStatistic*) malloc(sizeof(ChannelStatistic));
    if (g_hDriver > 0) {
        request2.chip = 0;
        request2.channelStatisticAddr = (__u32) channelStatistic;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETCHANNELSTATISTIC, (void *)&request2);
        dwError = request2.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    *pdwPostErrCnt = channelStatistic->postVitErrorCount;
    *pdwPostBitCnt = channelStatistic->postVitBitCount;
    *pwAbortCnt    = channelStatistic->abortCount;
	free(channelStatistic);

    return (dwError);
}


Dword DTV_GetStatisticEx(
    OUT PDTVStatistic pStatisic)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    GetStatisticRequest request;
    Statistic statistic;
	memset(&statistic, 0, sizeof(statistic));      

    if (g_hDriver > 0) {
        request.chip = 0;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETSTATISTIC, (void *)&request);
        statistic = (Statistic) request.statistic;
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }
    pStatisic->signalQuality = statistic.signalQuality;
    pStatisic->signalStrength = statistic.signalStrength;
    pStatisic->signalPresented = statistic.signalPresented;
    pStatisic->signalLocked = statistic.signalLocked;
    pStatisic->frameErrorCount = statistic.frameErrorRatio / 128;
    pStatisic->mpefecFrameErrorCount = statistic.mpefecFrameErrorRatio /128;

    GetChannelStatisticRequest request2;
    ChannelStatistic* channelStatistic = (ChannelStatistic*) malloc(sizeof(ChannelStatistic));    

    if (g_hDriver > 0) {
        request2.chip = 0;
        request2.channelStatisticAddr = (__u32) channelStatistic;        
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETCHANNELSTATISTIC, (void *)&request2);
        dwError = request2.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    pStatisic->postVitErrorCount = channelStatistic->postVitErrorCount;
    pStatisic->postVitBitCount = channelStatistic->postVitBitCount;
    pStatisic->abortCount = channelStatistic->abortCount;

	free(channelStatistic);
    return (dwError);
}

Dword DTV_StartCapture()
{
    Dword dwError = ERR_NO_ERROR;
    int result;

    /* here tell driver begin to read data */
    result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_STARTCAPTURE);

    return(dwError);
}


Dword DTV_StopCapture()
{
    Dword dwError = ERR_NO_ERROR;
    int result;

    result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_STOPCAPTURE);

    return(dwError);
}


Dword DTV_GetVersion(
    OUT PDemodDriverInfo pDriverInfo)
{
    Dword dwError = ERR_NO_ERROR;
    int result;

    if (g_hDriver > 0) {
		
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETDRIVERINFO, (void *)pDriverInfo);
        dwError = pDriverInfo->error;
        
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;	
}

Dword DTV_WriteData(
    OUT Byte* pBuffer,
    IN OUT Dword pdwBufferLength)
{
	Dword dwError = ERR_NO_ERROR;
	int Len = 0;
	
    if (g_hDriver > 0) {
		Len = write(g_hDriver, pBuffer, pdwBufferLength);
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return Len;
}

Dword DTV_GetData(
    OUT Byte* pBuffer,
    IN OUT Dword* pdwBufferLength)
{
    Dword dwError = ERR_NO_ERROR;
//    GetDatagramRequest request;

    if (g_hDriver > 0) {
//        request.bufferLength = pdwBufferLength;
//        request.buffer = pBuffer;
//			result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_GETDATAGRAM, (void *)&request);
		*pdwBufferLength = read(g_hDriver, pBuffer, *pdwBufferLength);
        dwError = *pdwBufferLength;
//		ReadFile(g_hDriver, pBuffer, *pdwBufferLength, pdwBufferLength, NULL);
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword DTV_AddPIDEx(
    IN  DTVPid pid)
{	
    Dword dwError = ERR_NO_ERROR;
    int result;
    AddPidAtRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        memcpy(&request.pid, &pid, sizeof(Pid));
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_ADDPIDAT, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return (dwError);
}

Dword DTV_ReadRegOFDM(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    ReadRegistersRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.processor = Processor_OFDM;
        request.registerAddress = dwRegAddr;
        request.bufferLength = 1;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_READREGISTERS, (void *)&request);
		*pbyData = request.buffer[0];
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}


Dword DTV_WriteRegOFDM(
    IN  Dword dwRegAddr,
    IN  Byte byData)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    WriteRegistersRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.processor = Processor_OFDM;
        request.registerAddress = dwRegAddr;
        request.bufferLength = 1;
        memcpy (request.buffer, &byData, 1);
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_WRITEREGISTERS, (void *)&request);
        dwError = request.error;
    } else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}


Dword DTV_ReadRegLINK(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData)
{
    Dword dwError = ERR_NO_ERROR;

    int result;
    ReadRegistersRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.processor = Processor_LINK;
        request.registerAddress = dwRegAddr;
        request.bufferLength = 1;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_READREGISTERS, (void *)&request);
		*pbyData = request.buffer[0];
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}


Dword DTV_WriteRegLINK(
    IN  Dword dwRegAddr,
    IN  Byte byData)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    WriteRegistersRequest request;

    if (g_hDriver > 0) {
        request.chip = 0;
        request.processor = Processor_LINK;
        request.registerAddress = dwRegAddr;
        request.bufferLength = 1;
        memcpy (request.buffer, &byData, 1);
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_WRITEREGISTERS, (void *)&request);
        dwError = request.error;
    }
    else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}

Dword DTV_SetDecrypt(
	Dword	decryptKey,
	Byte	decryptEnable)
{
    Dword dwError = ERR_NO_ERROR;
    int result;
    TxSetDecryptRequest request;

    if (g_hDriver > 0) {
        request.decryptKey = (__u32) decryptKey;
        request.decryptEnable = decryptEnable;
        result = ioctl(g_hDriver, IOCTL_ITE_DEMOD_SETDECRYPT, (void *)&request);
        dwError = request.error;
    } else {
        dwError = ERR_NOT_IMPLEMENTED;
    }

    return dwError;
}
