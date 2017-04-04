#ifndef _ERROR_H_
#define _ERROR_H_

//***********************************************************************
// Error Code
//***********************************************************************
#define     ERR_NO_ERROR				0x00000000ul
#define		ERR_RESET_TIMEOUT			0x00000001ul
#define     ERR_WRITE_REG_TIMEOUT       0x00000005ul
#define     ERR_WRITE_TUNER_TIMEOUT     0x00000006ul
#define     ERR_WRITE_TUNER_FAIL        0x00000007ul
#define     ERR_READ_TUNER_TIMEOUT      0x00000004ul
#define     ERR_RSD_COUNTER_NOT_READY   0x00000008ul
#define     ERR_VTB_COUNTER_NOT_READY   0x00000009ul
#define     ERR_FEC_MON_NOT_ENABLED     0x0000000Aul
#define     ERR_INVALID_DEV_TYPE        0x0000000Bul
#define     ERR_INVALID_TUNER_TYPE      0x0000000Cul
#define     ERR_OPEN_FILE_FAIL          0x0000000Dul
#define     ERR_WRITEFILE_FAIL          0x0000000Eul
#define     ERR_READFILE_FAIL           0x0000000Ful
#define     ERR_CREATEFILE_FAIL         0x00000010ul
#define     ERR_MALLOC_FAIL             0x00000011ul
#define     ERR_INVALID_FILE_SIZE       0x00000012ul
#define     ERR_INVALID_READ_SIZE       0x00000013ul
#define     ERR_LOAD_FW_DONE_BUT_FAIL   0x00000014ul
#define     ERR_NOT_IMPLEMENTED         0x00000015ul
#define     ERR_WRITE_MBX_TUNER_TIMEOUT 0x00000016ul
#define     ERR_DIV_MORE_THAN_8_CHIPS   0x00000017ul
#define     ERR_DIV_NO_CHIPS            0x00000018ul
#define     ERR_SUPER_FRAME_CNT_0       0x00000019ul
#define     ERR_INVALID_FFT_MODE        0x0000001Aul
#define     ERR_INVALID_CONSTELLATION_MODE        0x0000001Bul
#define     ERR_RSD_PKT_CNT_0           0x0000001Cul
#define     ERR_FFT_SHIFT_TIMEOUT       0x0000001Dul
#define     ERR_WAIT_TPS_TIMEOUT        0x0000001Eul
#define     ERR_INVALID_BW				0x0000001Ful
#define     ERR_INVALID_BUF_LEN         0x00000020ul
#define     ERR_NULL_PTR                0x00000021ul
#define     ERR_MT_TUNE_FAIL            0x00000022ul
#define     ERR_MT_OPEN_FAIL            0x00000023ul
#define     ERR_INVALID_AGC_VOLT        0x00000024ul
#define		ERR_CMD_NOT_SUPPORTED		0x00000025ul
#define		ERR_CE_NOT_READY			0x00000026ul
#define     ERR_READ_TUNER_FAIL         0x00000027ul
#define     ERR_EMBX_INT_NOT_CLEARED    0x00000028ul
#define     ERR_INV_PULLUP_VOLT         0x00000029ul
#define     ERR_FREQ_OUT_OF_RANGE       0x0000002Aul
#define     ERR_MT_NOT_AVAILABLE        0x0000002Bul
#define     ERR_BACK_TO_BOOTCODE_FAIL   0x0000002Cul
#define     ERR_GET_BUFFER_VALUE_FAIL   0x0000002Dul
#define     ERR_MEM_ALLOC_FAIL          0x0000002Eul
#define     ERR_INVALID_POS             0x0000002Ful
#define     ERR_DYNA_TOP_FAIL           0x00000030ul
#define     ERR_INVALID_INDEX           0x00000031ul
#define     ERR_WAIT_PVIT_TIMEOUT       0x00000032ul
#define     ERR_FUNC_INTERRUPTED        0x00000033ul
#define     ERR_XTAL_NOT_SUPPORT        0x00000034ul
#define     ERR_CANT_FIND_ORIG_TOPS     0x00000035ul
#define     ERR_INVALID_REG_VALUE       0x00000036ul
#define     ERR_TUNER_NOT_SUPPORT       0x00000038ul
#define     ERR_UNDEFINED_SAW_BW        0x00000039ul
#define     ERR_INVALID_CHIP_REV        0x0000003Aul
#define     ERR_BUFFER_INSUFFICIENT      0x0000003Bul
#define     ERR_COUNTER_NOT_AVAILABLE   0x0000003Cul
#define     ERR_LOADFW_COMP_FAIL        0x0000003Dul
#define     ERR_CANT_FIND_EEPROM        0x0000003Eul
#define     ERR_TUNER_TYPE_NOT_SUPPORT  0x0000003Ful
#define     ERR_INV_MISC_REG            0x00000040ul
#define     ERR_CANT_FIND_USB_DEV       0x00000041ul
#define     ERR_INVALID_XTAL_FREQ       0x00000043ul
#define     ERR_INVALID_DEVICE_COUNT    0x00000044ul


//***********************************************************************
// Error Code of Parallel Module
//***********************************************************************
#define     ERR_I2C_NO_ERROR            0x00000000ul
#define     ERR_I2C_NULL_HANDLE         0x00000100ul
#define     ERR_I2C_DONT_SUPPORT        0x00000200ul

//***********************************************************************
// Error Code of Common Module
//***********************************************************************
#define     ERR_COM_NO_ERROR            0x00000000ul
#define     ERR_COM_DATA_HIGH_FAIL      0x00001000ul
#define     ERR_COM_CLK_HIGH_FAIL       0x00002000ul
#define     ERR_COM_WRITE_NO_ACK        0x00003000ul
#define     ERR_COM_DATA_LOW_FAIL       0x00004000ul

//***********************************************************************
// Error Code of USB20 Module
//***********************************************************************
#define     ERR_USB_NO_ERROR                  0x00000000ul
#define     ERR_USB_NULL_HANDLE               0x01000000ul
#define     ERR_USB_WRITEFILE_FAIL            0x02000000ul
#define     ERR_USB_READFILE_FAIL             0x03000000ul
#define     ERR_USB_INVALID_READ_SIZE         0x04000000ul
#define     ERR_USB_BAD_STATUS                0x05000000ul
#define     ERR_USB_INVALID_SN                0x06000000ul
#define     ERR_USB_INVALID_PKT_SIZE          0x07000000ul
#define     ERR_USB_INVALID_HEADER            0x08000000ul
#define     ERR_USB_NO_IR_PKT                 0x09000000ul
#define     ERR_USB_INVALID_IR_PKT            0x0A000000ul
#define     ERR_USB_INVALID_DATA_LEN          0x0B000000ul

#define     ERR_USB_EP4_READFILE_FAIL         0x0C000000ul
#define     ERR_USB_EP$_INVALID_READ_SIZE     0x0D000000ul

#define     ERR_USB_BOOT_INVALID_PKT_TYPE     0x0E000000ul

#define     ERR_USB_BOOT_BAD_CONFIG_HEADER    0x0F000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_SIZE      0x10000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_SN        0x11000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_SUBTYPE   0x12000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_VALUE     0x13000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_CHKSUM    0x14000000ul

#define     ERR_USB_BOOT_BAD_CONFIRM_HEADER   0x15000000ul
#define     ERR_USB_BOOT_BAD_CONFIRM_SIZE     0x16000000ul
#define     ERR_USB_BOOT_BAD_CONFIRM_SN       0x17000000ul
#define     ERR_USB_BOOT_BAD_CONFIRM_SUBTYPE  0x18000000ul
#define     ERR_USB_BOOT_BAD_CONFIRM_VALUE    0x19000000ul
#define     ERR_USB_BOOT_BAD_CONFIRM_CHKSUM   0x20000000ul

#define     ERR_USB_BOOT_BAD_BOOT_HEADER      0x21000000ul
#define     ERR_USB_BOOT_BAD_BOOT_SIZE        0x22000000ul
#define     ERR_USB_BOOT_BAD_BOOT_SN          0x23000000ul
#define     ERR_USB_BOOT_BAD_BOOT_PATTERN_01  0x24000000ul
#define     ERR_USB_BOOT_BAD_BOOT_PATTERN_10  0x25000000ul
#define     ERR_USB_BOOT_BAD_BOOT_CHKSUM      0x26000000ul

#define     ERR_USB_INVALID_BOOT_PKT_TYPE     0x27000000ul
#define     ERR_USB_BOOT_BAD_CONFIG_VAlUE     0x28000000ul

#define     ERR_USB_COINITIALIZEEX_FAIL           0x29000000ul
#define     ERR_USB_COCREATEINSTANCE_FAIL         0x2A000000ul
#define     ERR_USB_COCREATCLSEENUMERATOR_FAIL    0x2B000000ul
#define     ERR_USB_QUERY_INTERFACE_FAIL          0x2C000000ul
#define     ERR_USB_PKSCTRL_NULL                  0x2D000000ul
#define     ERR_USB_INVALID_HANDLE                0x2E000000ul
#define     ERR_USB_TOO_MUCH_WRITE_DATA           0x2F000000ul
#define     ERR_USB_NO_BURST_READ                 0x30000000ul
#define     ERR_USB_NULL_PENUM					  0x31000000ul

#endif // _ERROR_H_
