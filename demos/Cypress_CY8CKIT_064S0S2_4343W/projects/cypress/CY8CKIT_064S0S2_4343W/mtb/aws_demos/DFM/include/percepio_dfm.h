/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * percepio_dfm.h
 *
 * The Percepio DFM header.
 *
 * Terms of Use
 * This file is part of the DevAlert Firmware Monitor library (SOFTWARE), which
 * is the intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The SOFTWARE may be used free of charge for the purpose of collecting and
 * transferring data to the DevAlert cloud service. It may not be used or modified for
 * other purposes without explicit permission from PERCEPIO.
 * You may distribute the SOFTWARE in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the SOFTWARE with minor modifications intended for
 * configuration or porting of the SOFTWARE, e.g., to allow using it on a 
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.  
 * The DevAlert cloud service and the Tracealyzer desktop application are 
 * provided under separate terms.
 *
 * Disclaimer
 * The SOFTWARE is being delivered to you AS IS and PERCEPIO makes no warranty
 * as to its use or performance. PERCEPIO does not and cannot warrant the 
 * performance or results you may obtain by using the SOFTWARE or documentation.
 * PERCEPIO make no warranties, express or implied, as to noninfringement of
 * third party rights, merchantability, or fitness for any particular purpose.
 * In no event will PERCEPIO, its technology partners, or distributors be liable
 * to you for any consequential, incidental or special damages, including any
 * lost profits or lost savings, even if a representative of PERCEPIO has been
 * advised of the possibility of such damages, or for any claim by any third
 * party. Some jurisdictions do not allow the exclusion or limitation of
 * incidental, consequential or special damages, or the exclusion of implied
 * warranties or limitations on how long an implied warranty may last, so the
 * above limitations may not apply to you.
 *
 * Tabs are used for indent in this file (1 tab = 4 spaces)
 *
 * Copyright Percepio AB, 2019.
 * www.percepio.com
 ******************************************************************************/

#ifndef PERCEPIO_DFM_H
#define PERCEPIO_DFM_H

#ifdef __cplusplus
extern "C" {
#endif

/* DFM Status Codes */
#define DFM_STATUS_CODE_OK										0
#define DFM_STATUS_CODE_NOT_INITIALIZED							-1
#define DFM_STATUS_CODE_TOO_SMALL_PAYLOAD_SIZE					-2
#define DFM_STATUS_CODE_TOO_SMALL_FIRMWARE_VERSION_BUFFER		-3
#define DFM_STATUS_CODE_MAX_SYMPTOMS_EXCEEDED					-4
#define DFM_STATUS_CODE_NONVOLATILE_DATA_ALREADY_PRESENT		-5

/* Hardware Port Status Codes */
#define DFM_STATUS_CODE_HARDWARE_PORT_INIT_FAILED				-20
#define DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_WRITE_FAILED		-21
#define DFM_STATUS_CODE_HARDWARE_PORT_END_WRITE_FAILED			-22
#define DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED			-23
#define DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED			-24
#define DFM_STATUS_CODE_HARDWARE_PORT_HAS_DATA_FAILED			-25
#define DFM_STATUS_CODE_HARDWARE_PORT_CLEAR_DATA_FAILED			-26
#define DFM_STATUS_CODE_HARDWARE_PORT_WRITE_NONVOLATILE_FAILED	-27
#define DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED	-28

/* Cloud Port Status Codes */
#define DFM_STATUS_CODE_CLOUD_PORT_INIT_FAILED					-40
#define DFM_STATUS_CODE_CLOUD_PORT_SEND_FAILED					-41

/* Hardware Port Status Codes */
#define DFM_STATUS_CODE_KERNEL_PORT_INIT_FAILED					-60
#define DFM_STATUS_CODE_KERNEL_PORT_DELAY						-61
#define DFM_STATUS_CODE_KERNEL_PORT_GET_TRACE_SIZE_FAILED		-62
#define DFM_STATUS_CODE_KERNEL_PORT_READ_TRACE_FAILED			-63
#define DFM_STATUS_CODE_KERNEL_PORT_BEGIN_READ_TRACE_FAILED		-64
#define DFM_STATUS_CODE_KERNEL_PORT_END_READ_TRACE_FAILED		-65

/* DFM Sections */
#define DFM_SECTION_NA											0
#define DFM_SECTION_SETTINGS									1
#define DFM_SECTION_SESSION										2
#define DFM_SECTION_ALERT_HEADER								3
#define DFM_SECTION_TRACE_DATA									4

#include "percepio_dfmConfig.h"
#include "dfmKernelPort.h"

#include "dfmCodes.h"

uint8_t InitDFM_EEPROM(void);

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#define DFM_SESSION_ID_MAX_LEN (32)
/* Make sure the length is a multiple of 4 */
#define DFM_FIRMWARE_VERSION_MAX_LEN ((((DFM_CFG_FIRMWARE_VERSION_MAX_LEN) + 3) / 4) * 4)

/*******************************************************************************
 * The DFM settings definition.
 ******************************************************************************/
typedef struct
{
	 uint32_t ulDfmDisabled;			/* Symptom ID */
} DfmSettings_t;

/*******************************************************************************
 * The DFM symptom definition.
 ******************************************************************************/
typedef struct
{
	 uint32_t ulSymptomId;			/* Symptom ID */
	 uint32_t ulValue;				/* The Symptom value */
	 /* TODO: Add a char buffer for string values? Or a union? */
} DfmSymptom_t;

/*******************************************************************************
 * The DFM header, needed by the DevAlert Firmware Monitor cloud service.
 ******************************************************************************/
typedef struct
{
	uint8_t ucStartMarkers[4];		/* The DFM start marker, must be 0x50, 0x44, 0x66, 0x6D ("PDfm") */

	uint16_t usEndianness;			/* The endianness checker, assign to 0x0FF0 */

	uint8_t ucVersion;				/* The version of the DFM subsystem, 0 for not enabled */
	uint8_t ucFirmwareVersionSize;	/* The maximum length of cFirmwareVersionBuffer */
	uint8_t ucMaxSymptoms;			/* The maximum number of symptoms, initialized to DFM_CFG_MAX_SYMPTOMS */
	uint8_t ucSymptomCount;			/* The number of registered symptoms. Must be <= DFM_CFG_MAX_SYMPTOMS */

	uint8_t ucReserved0;			/* Reserved for future use */
	uint8_t ucReserved1;			/* Reserved for future use */

	uint32_t ulAlertType;			/* The alert type */

	uint32_t ulTraceSize;			/* The size of the trace */

	DfmSymptom_t xSymptoms[DFM_CFG_MAX_SYMPTOMS]; /* The symptoms */

	char cFirmwareVersionBuffer[DFM_FIRMWARE_VERSION_MAX_LEN]; /* Size will always be 4-byte aligned */

	uint8_t ucEndMarkers[4];		/* The DFM start marker, must be 0x6D, 0x66, 0x44, 0x50 ("mfDP") */

	uint32_t ulChecksum;			/* Checksum on the whole thing, 0 for not enabled */
} DfmAlertHeader_t;

/*******************************************************************************
 * The DFM Session, used to keep track of Session data.
 * Contains data that is useful for Non-Volatile memory storage.
 ******************************************************************************/
typedef struct
{
	uint8_t ucVersion;				/* The version of the DFM subsystem, 0 for not enabled */

	uint8_t ucReserved0;			/* Reserved for future use */
	uint8_t ucReserved1;			/* Reserved for future use */
	uint8_t ucReserved2;			/* Reserved for future use */

	char cUniqueSessionIdBuffer[DFM_SESSION_ID_MAX_LEN]; /* The Session ID */

	uint32_t ulTraceCounter;		/* The trace counter */
} DfmSession_t;

/* The DFM API */
/*******************************************************************************
 * ulDfmInit
 *
 * Used to initialize DFM.
 * Must be called after a connection to the cloud service has been established.
 * 
 * NOTE: Will also send any Alert previously stored in Non-Volatile memory by
 * ulDfmSaveDataToNonVolatileMemory().
 *
 * @param szFirmwareVersion The current firmware version.
 ******************************************************************************/
uint32_t ulDfmInit(const char* szFirmwareVersion);

/*******************************************************************************
 * ulDfmEnable
 *
 * Used to enable DFM after it has been disabled. Will also clear the Disabled
 * flag in non-volatile memory.
 *
 * NOTE: Not necessary to call this function unless DFM has been previously
 * disabled by calling ulDfmDisable().
 ******************************************************************************/
uint32_t ulDfmEnable(void);

/*******************************************************************************
 * ulDfmDisable
 *
 * Used to disable DFM. Will also store a Disabled flag in non-volatile memory.
 *
 * NOTE: Must call ulDfmEnable() to enable again.
 ******************************************************************************/
uint32_t ulDfmDisable(void);

/*******************************************************************************
 * ulDfmIsDisabled
 *
 * Used to check if DFM is disabled.
 ******************************************************************************/
uint32_t ulDfmIsDisabled(void);

/*******************************************************************************
 * ulDfmGetStatus
 *
 * Returns the current status code.
 ******************************************************************************/
uint32_t ulDfmGetStatus(void);

/*******************************************************************************
 * ulDfmGetBuffer
 *
 * This function will return a buffer that can be used by the Kernel port or
 * Hardware Port to store data in.
 ******************************************************************************/
uint32_t ulDfmGetBuffer(void** ppvData, uint32_t* pulSize);

/*******************************************************************************
 * ulDfmAlert
 *
 * Used to set the Alert type for a new Alert instance.
 * Will reset any previously assigned Symptoms.
 * The available Alerts can be found in dfmCodes.h, which is configured and
 * generated via the online DFM portal. TODO: insert link possible or unique?
 *
 * @param ulAlertType The alert type.
 ******************************************************************************/
uint32_t ulDfmAlert(uint32_t ulAlertType);

/*******************************************************************************
 * ulDfmAlertReset
 *
 * Can be used to reset the Alert and Symptoms data.
 * Will also be automatically called each time ulDfmAlert() is called.
 ******************************************************************************/
uint32_t ulDfmAlertReset(void);

/*******************************************************************************
 * ulDfmAddSymptom
 *
 * Used to add a Symptom to the Alert.
 * The available Symptoms can be found in dfmCodes.h, which is configured and
 * generated via the online DFM portal. TODO: insert link possible or unique?
 *
 * @param ulSymptomId The alert type.
 * @param ulValue The alert type.
 ******************************************************************************/
uint32_t ulDfmAddSymptom(uint32_t ulSymptomId, uint32_t ulValue);

/*******************************************************************************
 * ulDfmSendDataToCloud
 *
 * Sends the Alert and Symptoms.
 ******************************************************************************/
uint32_t ulDfmSendDataToCloud(void);

/*******************************************************************************
 * ulDfmSaveDataToNonVolatileMemory
 *
 * This function will attempt to store the Alert, Symptoms and any other
 * necessary information in Non-Volatile memory. This should be used when
 * sending the Alert right away will fail, i.e. after a Hard Fault.
 * The Alert will be automatically sent after reboot when ulDfmInit() is called.
 ******************************************************************************/
uint32_t ulDfmSaveDataToNonVolatileMemory(void);

#else /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */

#define ulDfmInit(fv) ((void)fv, 0)
#define ulDfmGetBuffer(ppvData, pulSize) ((void)ppvData, (void)pulSize, 0)
#define ulDfmAlert(at) ((void)at, 0)
#define ulDfmAlertReset() (0)
#define ulDfmAddSymptom(si, v) ((void)si, (void)v, 0)
#define ulDfmSendDataToCloud() (0)
#define ulDfmSaveDataToNonVolatileMemory() (0)
	
#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */
	
#ifdef __cplusplus
}
#endif

#endif /* PERCEPIO_DFM_H */
