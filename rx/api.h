/**
 * Copyright (c) 2015 ITE Technologies, Inc. All rights reserved.
 * 
 * Date:
 *    2015/01/14
 *
 * Module Name:
 *    api.h
 *
 * Abstract:
 *    ITE Linux API header file.
 */


#ifndef     _DEMOD_DTVAPI_
#define     _DEMOD_DTVAPI_

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "type.h"
#include "iocontrol.h"
#include "error.h"


#define DTVEXPORT

#define __DTVAPI_VERSION__      "1.0.0.0"

#define INVALID_HANDLE_VALUE    0

//extern int g_hDriver = 0;
//
// The type defination of StreamType.
//
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

//
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
	DTVFrameRow_256 = 0,         // There should be 256 rows for each column in MPE-FEC frame.
	DTVFrameRow_512,             // There should be 512 rows for each column in MPE-FEC frame.
	DTVFrameRow_768,	         // There should be 768 rows for each column in MPE-FEC frame.
	DTVFrameRow_1024             // There should be 1024 rows for each column in MPE-FEC frame.
} DTVFrameRow;

/*
 * The type defination of Pid.
 * as sectionType = SectionType_SIPSI: only value is valid.
 * as sectionType = SectionType_TABLE: both value and table is valid.
 * as sectionType = SectionType_MPE: except table all other fields is valid.
 */
typedef struct {
	Byte table;	                 // The table ID. Which is used to filter specific SI/PSI table.
	Byte duration;               // The maximum burst duration. It can be specify to 0xFF if user don't know the exact value.
	DTVFrameRow frameRow;        // The frame row of MPE-FEC. It means the exact number of rows for each column in MPE-FEC frame.
	DTVSectionType sectionType;  // The section type of pid. See the defination of SectionType.
	DTVPriority priority;        // The priority of MPE data. Only valid when sectionType is set to SectionType_MPE.
	DTVIpVersion version;        // The IP version of MPE data. Only valid when sectionType is set to SectionType_MPE.
	Bool cache;                  // True: MPE data will be cached in device's buffer. Fasle: MPE will be transfer to host.
	Word value;                  // The 13 bits Packet ID.
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
    Dword frequency;                         // Channel frequency in KHz.
    DTVTransmissionMode transmissionMode;    // Number of carriers used for OFDM signal.
    DTVConstellation constellation;          // Constellation scheme (FFT mode) in use.
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
    Byte serviceType;            // Service Type(P/D): 0x00: Program, 0x80: Data.
    Dword serviceId;             // Service ID.
    Word componentId;            // Stream audio/data is subchid, packet mode is SCId.
    Byte componentIdService;     // Component ID within Service.
    DTVLabel componentLabel;     // The label of component. See the defination of Label.
    Byte language;               // Language code.
    Byte primary;                // Primary/Secondary.
    Byte conditionalAccess;      // Conditional Access flag.
    Byte componentType;          // Component Type (A/D).
    Byte transmissionId;         // Transmission Mechanism ID.
} DTVComponent, *PDTVComponent;



// -----------------------------------------------------------------------------
//  PURPOSE:
//      Initialize Chip and Power on device.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_Initialize(Byte);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Clean up & Power off device.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_Finalize(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Specify the bandwidth of channel and tune the channel to the specific frequency.
//
//  PARAMETERS:
//      dwFrequency - Channel frequnecy in KHz.
//      wBandwidth - Channel bandwidth in KHz or MHz (if the bandwidth input is less than 10,
//                   it will be multiplied by 1000 (and coverted to KHz) automatically).
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_AcquireChannel(
    IN Dword dwFrequency,
    IN Word  wBandwidth);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Check if TPS and MPEG2 module both are locked.
//
//  PARAMETERS:
//      bpLocked - return 1 if channel is locked.
//
//  RETURNS:
//      0 if no error , non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_IsLocked(
    OUT Bool* pbLocked);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Get post-vit ber, signal quality, signal strength.
//
//  PARAMETERS:
//      wNumOfPacket - number of RSD packets.
//      pdwPostErrCnt - ErrorBitCount.
//      pdwPostBitCnt - TotalBitCount.
//      pwAbortCnt - number of abort RSD packet.
//      pwQuality - signal quality (0 - 100).
//      pwStrength - signal strength (0 - 100).
//
//  RETURNS:
//      0 if no error , non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_GetStatistic(
    IN  Word  wNumOfPacket,
    OUT Dword* pdwPostErrCnt,
    OUT Dword* pdwPostBitCnt,
    OUT Word*  pwAbortCnt,
    OUT Word*  pwQuality,
    OUT Word*  pwStrength);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Extension for DTV_GetStatistic, further information for DVB-H.
//
//  PARAMETERS:
//      pStatisic - Return the structure that store all statistic values.
//
//  RETURNS:
//      0 if no error , non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_GetStatisticEx(
    OUT PDTVStatistic pStatisic);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Get channel TPS(Transmission Parameter Signaling) information.
//
//  PARAMETERS:
//      pChannelTPSInfo - Return the structure that store all TPS information.
//
//  RETURNS:
//      0 if no error , non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_GetChannelTPSInfo(
    OUT PDTVChannelTPSInfo pChannelTPSInfo);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Enable Pid Table, for DVB-T mode.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_EnablePIDTbl(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Disable Pid Table, for DVB-T mode.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_DisablePIDTbl(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Clear all the PID's set previously by DTV_AddPID(), for DVB-T mode.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_ResetPIDTable(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Add Pid number with extension parameters, specifically for DVB-H.
//
//  PARAMETERS:
//      pid - pid structure data (Specify the PID number and relevant attributes in DVB-H mode).
//
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_AddPIDEx(
    IN  DTVPid pid);

Dword DTV_StartCapture(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Stop to Capture data from device, for DVB-T/TDMB/FM mode.
//
//  PARAMETERS:
//      None.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_StopCapture(void);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Control Power Saving. The function can be called by application for power saving while idle mode.
//
//  PARAMETERS:
//      byCtrl - 1: Power Up, 0: Power Down.
//               Power Up :  Resume  device to normal state.
//               Power Down : Suspend device to hibernation state.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_ControlPowerSaving(
    IN  Byte byCtrl);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Get Driver & API Version.
//
//  PARAMETERS:
//      pDriverInfo - Return driver information with DTVDriverInfo structure.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
Dword DTV_GetVersion(
    OUT PDemodDriverInfo pDriverInfo);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Get data from driver, for DVB-T/TDMB/FM mode.
//
//  PARAMETERS:
//      pbyBuffer - Data buffer point.
//      pdwBufferLength - IN: Data buffer length, OUT: Data buffer filled length actually.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_GetData(
    OUT Byte* pbyBuffer,
    IN OUT Dword* pdwBufferLength);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Read OFDM register.
//
//  PARAMETERS:
//      dwRegAddr - Register address.
//      pbyData - Register value.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_ReadRegOFDM(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Write OFDM register.
//
//  PARAMETERS:
//      dwRegAddr - Register address.
//      byData - Register value.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_WriteRegOFDM(
    IN  Dword dwRegAddr,
    IN  Byte byData);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Read LINK register.
//
//  PARAMETERS:
//      dwRegAddr - Register address.
//      pbyData - Register value.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_ReadRegLINK(
    IN  Dword dwRegAddr,
    OUT Byte* pbyData);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Write LINK register.
//
//  PARAMETERS:
//      dwRegAddr - Register address.
//      byData - Register value.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_WriteRegLINK(
    IN  Dword dwRegAddr,
    IN  Byte byData);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Read EEPROM I2C address from hardware.
//
//  PARAMETERS:
//      byData - I2C address.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_ReadEEPROMI2CAddr(
    OUT Byte* pbyData);

// -----------------------------------------------------------------------------
//  PURPOSE:
//      Set TPS Decryption.
//
//  PARAMETERS:
//      decryptKey - decrypt key.
//      decryptEnable - decrypt enable.
// 
//  RETURNS:
//      0 if no error, non-zero value otherwise.
// -----------------------------------------------------------------------------
DTVEXPORT Dword DTV_SetDecrypt(
	IN Dword	decryptKey,
	IN Byte	decryptEnable);
#endif 
