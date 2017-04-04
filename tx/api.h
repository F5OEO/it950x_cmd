/**
 * Copyright (c) 2016 ITE Technologies, Inc. All rights reserved.
 * 
 * Date:
 *    2016/09/29
 *
 * Module Name:
 *    api.h
 *
 * Abstract:
 *    ITE Linux API header file.
 */


#ifndef     _MOD_DTVAPI_
#define     _MOD_DTVAPI_

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <termios.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "type.h"
#include "iocontrol.h"
#include "error.h"


#define DTVEXPORT
#define __DTVAPI_VERSION__      "1.0.0.0"
#define INVALID_HANDLE_VALUE    0
#define TRANSMITTER_BLOCK_SIZE 188*174
//extern int g_hDriver = 0;
/*
 * The type defination of StreamType.
 */
typedef enum {
	DTVStreamType_NONE = 0,		// Invalid (Null) StreamType
	DTVStreamType_DVBH_DATAGRAM,
	DTVStreamType_DVBH_DATABURST,
	DTVStreamType_DVBT_DATAGRAM,
	DTVStreamType_DVBT_PARALLEL,
	DTVStreamType_DVBT_SERIAL,
	DTVStreamType_TDMB_DATAGRAM,
	DTVStreamType_FM_DATAGRAM,
	DTVStreamType_FM_I2S
} DTVStreamType;

/*
 * 	Chip Type Information.
 */
typedef struct {
	Word UsbMode;
	Word VendorID;
	Word ProductID;
} CHIPTYPE_INFO, *PCHIPTYPE_INFO;

/*
 * Device information.
 */
typedef struct {
    Byte	DriverVerion[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte	APIVerion[32];      /** XX.XX.XXXXXXXX.XX Ex., 1.2.3.4	*/
    Byte	FWVerionLink[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte	FWVerionOFDM[16];   /** XX.XX.XX.XX Ex., 1.2.3.4			*/
    Byte	DateTime[24];       /** Ex.,"2004-12-20 18:30:00" or "DEC 20 2004 10:22:10" with compiler __DATE__ and __TIME__  definitions */
    Byte	Company[8];         /** Ex.,"ITEtech"						*/
    Byte	SupportHWInfo[32];  /** Ex.," DVBT/ISDBT"					*/	
	Word	ProductID;
} DEVICE_INFO, *PDEVICE_INFO;

/*
 * Driver and API information.
 */
typedef struct {
    Byte  DriverVerion[16];      // XX.XX.XX.XX Ex., 1.2.3.4.
    Byte  APIVerion[32];         // XX.XX.XXXXXXXX.XX Ex., 1.2.3.4.
    Byte  FWVerionLink[16];      // XX.XX.XX.XX Ex., 1.2.3.4.
    Byte  FWVerionOFDM[16];      // XX.XX.XX.XX Ex., 1.2.3.4.
    Byte  DateTime[24];          // Ex.,"2004-12-20 18:30:00" or "DEC 20 2004 10:22:10" with compiler __DATE__ and __TIME__  definitions.
    Byte  Company[8];            // Ex.,"Afatech".
    Byte  SupportHWInfo[32];     // Ex.,"Jupiter DVBT/DVBH".
    Dword error;
    Byte  reserved[128];
} DTVDriverInfo, *PDTVDriverInfo;

/*
 * The type defination of Priority.
 */
typedef enum {
    DTVPriority_HIGH = 0,        // DVB-T and DVB-H - identifies high-priority stream.
    DTVPriority_LOW              // DVB-T and DVB-H - identifies low-priority stream.
} DTVPriority;

/*
 * The type defination of IpVersion.
 */
typedef enum {
	DTVIpVersion_IPV4 = 0,       // The IP version if IPv4.
	DTVIpVersion_IPV6 = 1        // The IP version if IPv6.
} DTVIpVersion;


/*
 * The type defination of Ip.
 */
typedef struct {
	DTVIpVersion version;        // The version of IP. See the defination of IpVersion.
	DTVPriority priority;        // The priority of IP. See the defination of Priority.
	Bool cache;                  // True: IP datagram will be cached in device's buffer. Fasle: IP datagram will be transfer to host.
	Byte address[16];            // The byte array to store IP address.
} DTVIp, *PDTVIp;

/*
 * The type defination of Platform.
 * Mostly used is in DVB-H standard
 */
typedef struct {
	Dword platformId;            // The ID of platform.
	char iso639LanguageCode[3];  // The ISO 639 language code for platform name.
	Byte platformNameLength;     // The length of platform name.
	char platformName[32];       // The char array to store platform name.
	Word bandwidth;              // The operating channel bandwith of this platform.
	Dword frequency;             // The operating channel frequency of this platform.
	Byte information[4096];      // The extra information about this platform.
	Word informationLength;      // The length of information.
	Bool hasInformation;         // The flag to indicate if there exist extra information.
	DTVIpVersion ipVersion;      // The IP version of this platform.
} DTVPlatform, *PDTVPlatform;

/*
 * The type defination of Target.
 */
typedef enum {
	DTVSectionType_MPE = 0,      // Stands for MPE data.
	DTVSectionType_SIPSI,        // Stands for SI/PSI table, but don't have to specify table ID.
	DTVSectionType_TABLE         // Stands for SI/PSI table.
} DTVSectionType;

/*
 * The type defination of FrameRow.
 */
typedef enum {
	DTVFrameRow_256 = 0,			// There should be 256 rows for each column in MPE-FEC frame.
	DTVFrameRow_512,				// There should be 512 rows for each column in MPE-FEC frame.
	DTVFrameRow_768,				// There should be 768 rows for each column in MPE-FEC frame.
	DTVFrameRow_1024				// There should be 1024 rows for each column in MPE-FEC frame.
} DTVFrameRow;

/*
 * The type defination of Pid.
 * as sectionType = SectionType_SIPSI: only value is valid.
 * as sectionType = SectionType_TABLE: both value and table is valid.
 * as sectionType = SectionType_MPE: except table all other fields is valid.
 */
typedef struct {
	Byte table;					// The table ID. Which is used to filter specific SI/PSI table.
	Byte duration;				// The maximum burst duration. It can be specify to 0xFF if user don't know the exact value.
	DTVFrameRow frameRow;        // The frame row of MPE-FEC. It means the exact number of rows for each column in MPE-FEC frame.
	DTVSectionType sectionType;	// The section type of pid. See the defination of SectionType.
	DTVPriority priority;		// The priority of MPE data. Only valid when sectionType is set to SectionType_MPE.
	DTVIpVersion version;		// The IP version of MPE data. Only valid when sectionType is set to SectionType_MPE.
	Bool cache;					// True: MPE data will be cached in device's buffer. Fasle: MPE will be transfer to host.
	Word value;					// The 13 bits Packet ID.
} DTVPid, *PDTVPid;

/*
 * The type defination of statistic
 */
typedef struct {
    Dword postVitErrorCount;    // ErrorBitCount.
    Dword postVitBitCount;      // TotalBitCount.
    Word abortCount;            // Number of abort RSD packet.
    Word signalQuality;         // Signal quality (0 - 100).
    Word signalStrength;        // Signal strength (0 - 100).
    Bool signalPresented;       // TPS lock.
    Bool signalLocked;          // MPEG lock.
    Byte frameErrorCount;       // Frame Error Ratio (error ratio before MPE-FEC) = frameErrorRate / 128.
    Byte mpefecFrameErrorCount; // MPE-FEC Frame Error Ratio (error ratio after MPE-FEC) = mpefecFrameErrorCount / 128.
} DTVStatistic, *PDTVStatistic;

/*
 * The type defination of TransmissionMode.
 */
typedef enum {
    DTVTransmissionMode_2K = 0,  // OFDM frame consists of 2048 different carriers (2K FFT mode).
    DTVTransmissionMode_8K = 1,  // OFDM frame consists of 8192 different carriers (8K FFT mode).
    DTVTransmissionMode_4K = 2   // OFDM frame consists of 4096 different carriers (4K FFT mode).
} DTVTransmissionMode;

/*
 * The type defination of Constellation.
 */
typedef enum {
    DTVConstellation_QPSK = 0,   // Signal uses QPSK constellation.
    DTVConstellation_16QAM,      // Signal uses 16QAM constellation.
    DTVConstellation_64QAM       // Signal uses 64QAM constellation.
} DTVConstellation;

/*
 * The type defination of Interval.
 */
typedef enum {
    DTVInterval_1_OVER_32 = 0,   // Guard interval is 1/32 of symbol length.
    DTVInterval_1_OVER_16,       // Guard interval is 1/16 of symbol length.
    DTVInterval_1_OVER_8,        // Guard interval is 1/8 of symbol length.
    DTVInterval_1_OVER_4         // Guard interval is 1/4 of symbol length.
} DTVInterval;

/*
 * The type defination of CodeRate.
 */
typedef enum {
    DTVCodeRate_1_OVER_2 = 0,    // Signal uses FEC coding ratio of 1/2.
    DTVCodeRate_2_OVER_3,        // Signal uses FEC coding ratio of 2/3.
    DTVCodeRate_3_OVER_4,        // Signal uses FEC coding ratio of 3/4.
    DTVCodeRate_5_OVER_6,        // Signal uses FEC coding ratio of 5/6.
    DTVCodeRate_7_OVER_8,        // Signal uses FEC coding ratio of 7/8.
    DTVCodeRate_NONE             // None, NXT doesn't have this one.
} DTVCodeRate;

/*
 * TPS Hierarchy and Alpha value.
 */
typedef enum {
    DTVHierarchy_NONE = 0,       // Signal is non-hierarchical.
    DTVHierarchy_ALPHA_1,        // Signalling format uses alpha of 1.
    DTVHierarchy_ALPHA_2,        // Signalling format uses alpha of 2.
    DTVHierarchy_ALPHA_4         // Signalling format uses alpha of 4.
} DTVHierarchy;

/*
 * The type defination of Bandwidth.
 */
typedef enum {
    DTVBandwidth_6M = 0,         // Signal bandwidth is 6MHz.
    DTVBandwidth_7M,             // Signal bandwidth is 7MHz.
    DTVBandwidth_8M,             // Signal bandwidth is 8MHz.
    DTVBandwidth_5M              // Signal bandwidth is 5MHz.
} DTVBandwidth;

/*
 * The defination of ChannelInformation.
 */
typedef struct {
    Dword frequency; 							// Channel frequency in KHz.
    DTVTransmissionMode transmissionMode;	// Number of carriers used for OFDM signal.
    DTVConstellation constellation;			// Constellation scheme (FFT mode) in use.
    DTVInterval interval;                    // Fraction of symbol length used as guard (Guard Interval).
    DTVPriority priority;                    // The priority of stream.
    DTVCodeRate highCodeRate;                // FEC coding ratio of high-priority stream.
    DTVCodeRate lowCodeRate;                 // FEC coding ratio of low-priority stream.
    DTVHierarchy hierarchy;                  // Hierarchy levels of OFDM signal.
    DTVBandwidth bandwidth;
} DTVChannelTPSInfo, *PDTVChannelTPSInfo;

/*
 * Temp data structure of Gemini.
 */
typedef struct
{
    Byte charSet;
    Word charFlag;
    Byte string[16];
} DTVLabel, *PDTVLabel;

/*
 * Temp data structure of Gemini.
 */
typedef struct
{
    Word ensembleId;
    DTVLabel ensembleLabel;
    Byte totalServices;
} DTVEnsemble, *PDTVEnsemble;

/*
 * The type defination of Service.
 * Mostly used is in T-DMB standard
 */
typedef struct {
    Byte serviceType;            // Service Type(P/D): 0x00: Program, 0x80: Data.
    Dword serviceId;
    Dword frequency;
    DTVLabel serviceLabel;
    Byte totalComponents;
} DTVService, *PDTVService;

/*
 * The type defination of Service Component.
 */
typedef struct {
    Byte serviceType;				// Service Type(P/D): 0x00: Program, 0x80: Data.
    Dword serviceId;				// Service ID.
    Word componentId;				// Stream audio/data is subchid, packet mode is SCId.
    Byte componentIdService;		// Component ID within Service.
    DTVLabel componentLabel;		// The label of component. See the defination of Label.
    Byte language;				// Language code.
    Byte primary;					// Primary/Secondary.
    Byte conditionalAccess;		// Conditional Access flag.
    Byte componentType;			// Component Type (A/D).
    Byte transmissionId;			// Transmission Mechanism ID.
} DTVComponent, *PDTVComponent;

typedef struct _MODULATION_PARAM{
	Dword IOCTLCode;
    Byte highCodeRate;
    Byte transmissionMode;
    Byte constellation; 
    Byte interval;   
} MODULATION_PARAM, *PMODULATION_PARAM;

/*
 * The type defination of handle type.
 */
typedef enum {
    EAGLEI = 0,
    EAGLEII,
} HandleType;

/*******************************************************************************
 *  PURPOSE:
 *      Open handle, Power On and Check device information.
 *
 *  PARAMETERS:
 * 		 handleType:	typedef enum HandleType.
 *      DevNo: 		device number for multi deivce.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxDeviceInit(IN HandleType handleType, IN IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Clean up & Power off device.
 *
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxDeviceExit(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set Channel Modulation to Tx.
 *
 *  PARAMETERS:
 *      struct MODULATION_PARAM.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetChannelModulation( IN MODULATION_PARAM ChannelModulation_Setting, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set Channel frequency and bandwidth to Tx.
 *
 *  PARAMETERS:
 *      bfrequency
 * 		bbandwidth
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetChannel(IN Dword bfrequency, IN Word bbandwidth, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set Mode Enable.
 *
 *  PARAMETERS:
 *      OnOff
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetModeEnable(IN Byte OnOff, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set Device Type.
 *
 *  PARAMETERS:
 *      DeviceType
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetDeviceType(IN Byte DeviceType, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Device Type.
 *
 *  PARAMETERS:
 *      DeviceType
 * 
 *  RETURNS:
 *  	0:	 		 no error. Return current device type.
 * 		non-zero:	 Using Default devicetype. return default type 
 * 					 and return error message.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetDeviceType(OUT Byte *DeviceType, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *     Adjust Output Gain to Tx.
 *
 *  PARAMETERS:
 *     Gain_value
 *		Out_Gain_valu
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 *******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxAdjustOutputGain(IN int Gain_value, OUT int *Out_Gain_valu, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Add Pid number, for DVB-T mode.
 *
 *  PARAMETERS:
 *      byIndex - 0 ~ 31.
 *      wProgId - pid number.
 *
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxAddPID(IN Byte byIndex, IN  Word wProgId, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Clear all the PID's set previously by g_ITEAPI_AddPID(),
 *  	 for DVB-T mode.
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxResetPidFilter(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Add Pid number with extension parameters, specifically for DVB-H.
 *
 *  PARAMETERS:
 *      pid - pid structure data (Specify the PID number and relevant attributes in DVB-H mode).
 *
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxAddPIDEx(IN  DTVPid pid, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Start to Transfer data from device, for DVB-T mode.
 *      The drievr starts to receive TS data and store it in the ring buffer.
 *
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
Dword g_ITEAPI_StartTransfer(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      For DTVCAM. Start to Transfer data from device, for DVB-T mode.
 *      The drievr starts to receive TS data and store it in the ring buffer.
 *
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
Dword g_ITEAPI_StartTransfer_CMD(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Stop to Transfer data from device, for DVB-T mode.
 *
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_StopTransfer(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      For DTVCAM. Stop to Transfer data from device, for DVB-T mode.
 *
 *  PARAMETERS:
 *      None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_StopTransfer_CMD(IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Control Power Saving. The function can be called by application for power saving while idle mode.
 *
 *  PARAMETERS:
 *      byCtrl - 1: Power Up, 0: Power Down.
 *               Power Up :  Resume  device to normal state.
 *               Power Down : Suspend device to hibernation state.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxPowerCtl(IN  Byte byCtrl, IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Driver & API Version.
 *
 *  PARAMETERS:
 *      pTxDriverInfo 
 * 			- Return driver information with PDEVICE_INFO structure.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetDrvInfo(
    OUT PDEVICE_INFO pDeviceInfo,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Read OFDM register.
 *
 *  PARAMETERS:
 *      dwRegAddr - Register address.
 *      pbyData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxReadRegOFDM(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write OFDM register.
 *
 *  PARAMETERS:
 *      dwRegAddr - Register address.
 *      byData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxWriteRegOFDM(
    IN  Dword dwRegAddr,
    IN  Byte byData,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Read LINK register.
 *
 *  PARAMETERS:
 *      dwRegAddr - Register address.
 *      pbyData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxReadRegLINK(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write LINK register.
 *
 *  PARAMETERS:
 *      dwRegAddr - Register address.
 *      byData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxWriteRegLINK(
    IN  Dword dwRegAddr,
    IN  Byte byData,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write EEPROM data.
 *
 *  PARAMETERS:
 *      wRegAddr - Register address.
 *      pbyData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxWriteEEPROM(
    IN  Word	wRegAddr,
    OUT Byte	byData,
	IN Byte	DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Read EEPROM data.
 *
 *  PARAMETERS:
 *      wRegAddr - Register address.
 *      pbyData - Register value.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxReadEEPROM(
    IN  Word wRegAddr,
    OUT Byte* pbyData,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Gain Range.
 *
 *  PARAMETERS:
 *      frequency, bandwidth, MaxGain, MinGain
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetGainRange(
	IN Dword frequency,
	IN Word bandwidth,
	OUT int *MaxGain,
	OUT int *MinGain,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get TPS.
 *
 *  PARAMETERS:
 *      TPS structure
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetTPS(
	OUT TPS *tps,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set TPS.
 *
 *  PARAMETERS:
 *      TPS structure
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetTPS(
	IN TPS tps,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Output Gain.
 *
 *  PARAMETERS:
 *      gain
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetOutputGain(
	OUT int *gain,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Number of Device. 
 *
 *  PARAMETERS:
 * 		NumOfDev - Number of Device.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetNumOfDevice(
	OUT Byte *NumOfDev,
	IN  Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      g_ITEAPI_TxSendHwPSITable: Send Hardware PSI Table. 
 *
 *  PARAMETERS:
 *      bufferSize - size of TableBuffer.
 *		 TableBuffer
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSendCustomPacketOnce(
	IN int bufferSize,
	IN Byte *TableBuffer,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      g_ITEAPI_TxAccessFwPSITable: Send Hardware PSI Table. 
 *
 *  PARAMETERS:
 *      bufferSize - size of TableBuffer.
 *		 TableBuffer
 *		 index - PSI Table Index.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetPeridicCustomPacket(
	IN int bufferSize,
	IN Byte *TableBuffer,
	IN Byte index,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      g_ITEAPI_TxSetFwPSITableTimer: Send Hardware PSI Table.
 *       
 *  PARAMETERS:
 *		 index - PSI Table Index.
 *		 timer_interval- timer.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetPeridicCustomPacketTimer(
	IN Byte index, 
	IN Word timer_interval,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write Data. 
 *
 *  PARAMETERS:
 *		 pBuffer.
 *		 pdwBufferLength.
 * 
 *  RETURNS:
 *      0:			no error. write sucess.
 *		non-zero:	write fail. buffer overflow or other error. maybe write again.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSendTSData(
    OUT Byte* pBuffer,
    IN OUT Dword pdwBufferLength,
	IN Byte DevNo);
    
/*******************************************************************************
 *  PURPOSE:
 *      Set IQ Table From File. 
 *
 *  PARAMETERS:
 *	 	 ptrIQtable - file content load from binary file.
 *		 IQtableSize - file size.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetIQTable(
	IN Byte* ptrIQtable,
	IN Word	 IQtableSize,	
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Chip Version ID.
 *
 *  PARAMETERS:
 *	 	 ChipType
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetChipType(
	OUT	Word*	chipType,
	IN	Byte	DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Control Pid Filter.
 *
 *  PARAMETERS:
 *		Byte control.
 *		Byte enable.
 *		Byte DevNo.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
Dword g_ITEAPI_TxControlPidFilter(
	IN Byte control,
	IN Byte enable,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set DC Calibration Value. 
 *
 *  PARAMETERS:
 *		 dc_i	
 *	 	 dc_q
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetDCCalibrationValue(
	IN int dc_i,
	IN int dc_q,
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write Low Bit Rate Data. 
 *
 *  PARAMETERS:
 *		 isUseLowBrate	
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxWriteLowBitRateData(
	IN Byte* pBuffer,
	IN Dword BufferLength,	
	IN Byte DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Write command. 
 *
 *  PARAMETERS:
 *		 isUseLowBrate	
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
Dword g_ITEAPI_TxWriteCmd(
	IN Word			len,
    IN Byte*			cmd,
	IN Byte			DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set ISDB-T Channel Modulation.
 *
 *  PARAMETERS:
 *		ISDBTModulation	isdbtModulation.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetISDBTChannelModulation(
	IN ISDBTModulation      isdbtModulation,
	IN Byte					DevNo);
	
/*******************************************************************************
 *  PURPOSE:
 *      Set TMCC Information.
 *
 *  PARAMETERS:
 *		TMCCINFO      TmccInfo.
 *		Bool		  actualInfo.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetTMCCInfo(
    IN  TMCCINFO      TmccInfo,
	IN  Byte		   DevNo);
    
/*******************************************************************************
 *  PURPOSE:
 *      Get TMCC Information.
 *
 *  PARAMETERS:
 *	    pTMCCINFO     pTmccInfo.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetTMCCInfo(
    OUT  pTMCCINFO     pTmccInfo,
	IN   Byte		DevNo);
    
/*******************************************************************************
 *  PURPOSE:
 *      Get TMCC Information.
 *
 *  PARAMETERS:
 * 		 Word*     BitRate_Kbps.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetTSinputBitRate(
    OUT  Word*     BitRate_Kbps,
	IN   Byte		DevNo);
    
/*******************************************************************************
 *  PURPOSE:
 *      Add Pid To ISDB-T Pid Filter.
 *
 *  PARAMETERS:
 *		 Word*     BitRate_Kbps.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxAddPidToISDBTPidFilter(
    IN  Byte           	index,
    IN  Pid            	pid,
	IN	TransportLayer	layer,
	IN  Byte				DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set PCR Mode.
 *
 *  PARAMETERS:
 *		 PcrMode     mode.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetPCRMode(
    IN  PcrMode		mode,
	IN  Byte			DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Set DC Table.
 *
 *  PARAMETERS:
 *		 DCInfo.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetDCTable(
    IN  DCInfo			pDCInfo,
	IN  Byte			DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get Frequency Index.
 *
 *  PARAMETERS:
 *		 frequencyindex.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetFrequencyIndex(
	IN  Byte*			frequencyindex,
	IN  Byte			DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Get DTV Mode.
 *
 *  PARAMETERS:
 *		 DTVMode.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxGetDTVMode(
    OUT Byte* DTVMode,
    IN  Byte  DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Enable TPS Encryption.
 *
 *  PARAMETERS:
 *		 Encryption key.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxEnableTPSEncryption(
    IN  Dword key,
    IN  Byte  DevNo);

/*******************************************************************************
 *  PURPOSE:
 *      Disable TPS Encryption.
 *
 *  PARAMETERS:
 *		 None.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxDisableTPSEncryption(
    IN  Byte  DevNo);
	
/*******************************************************************************
 *  PURPOSE:
 *      Set Spectral Inversion.
 *
 *  PARAMETERS:
 *		Bool		  isInversion.
 * 
 *  RETURNS:
 *      0 if no error, non-zero value otherwise.
 ******************************************************************************/
DTVEXPORT Dword g_ITEAPI_TxSetSpectralInversion(
    IN  Bool      isInversion,
	IN  Byte	   DevNo);
#endif  

