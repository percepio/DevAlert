/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * percepio_dfm.c
 *
 * The Percepio DFM implementation.
 *
 * Terms of Use
 * This file is part of the DevAlert Firmware Monitor library (SOFTWARE), which
 * is the intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The SOFTWARE may be used free of charge for the purpose of collecting and
 * transferring data to the DFM service. It may not be used or modified for
 * other purposes without explicit permission from PERCEPIO.
 * You may distribute the SOFTWARE in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the SOFTWARE with minor modifications intended for
 * configuration or porting of the SOFTWARE, e.g., to allow using it on a 
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.  
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

#include "percepio_dfm.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#define DFM_VERSION 1

#if (!defined(DFM_VERSION) || (DFM_VERSION < 1))
#error "Invalid DFM_VERSION!"
#endif

#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "dfmCloudPort.h"

#include "dfmHardwarePort.h"

#if (defined(DFM_CFG_MAX_SYMPTOMS))
	#if (DFM_CFG_MAX_SYMPTOMS <= 0)
		#error "DFM_CFG_MAX_SYMPTOMS invalid"
	#endif /*(DFM_CFG_MAX_SYMPTOMS < 0) */
#else /* (defined(DFM_CFG_MAX_SYMPTOMS)) */
	#error "DFM_CFG_MAX_SYMPTOMS not defined"
#endif /* (defined(DFM_CFG_MAX_SYMPTOMS)) */

#if (defined(DFM_CFG_FIRMWARE_VERSION_MAX_LEN))
	#if (DFM_CFG_FIRMWARE_VERSION_MAX_LEN <= 0)
		#error "DFM_CFG_FIRMWARE_VERSION_MAX_LEN invalid"
	#endif /* (DFM_CFG_FIRMWARE_VERSION_MAX_LEN < 0) */
#else /* (defined(DFM_CFG_FIRMWARE_VERSION_MAX_LEN)) */
	#error "DFM_CFG_FIRMWARE_VERSION_MAX_LEN not defined"
#endif /* (defined(DFM_CFG_FIRMWARE_VERSION_MAX_LEN)) */

#if (defined(DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE))
	#if (DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE <= 0)
		#error "DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE invalid"
	#endif /*(DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE <= 0) */
#else /*(defined(DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE)) */
		#error "DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE is not defined"
#endif /* (defined(DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE)) */

/* DFM specific data */

/* Global kill switch TODO: Use this. */
static uint32_t ulDfmDisabled = 0;

/* The buffer that can be used internally, or by the Kernel Port and Hardware Port for reading data */
static uint8_t ucDataBuffer[DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE] __attribute__ ((aligned (8)));

/* Session ID */
static char cUniqueSessionIdBuffer[DFM_SESSION_ID_MAX_LEN];

/* Firmware Version */
static char cFirmwareVersionBuffer[DFM_FIRMWARE_VERSION_MAX_LEN];

/* Trace Counter */
static uint32_t ulTraceCounter = 0;

/* DFM Status */
static uint32_t ulDfmStatus = DFM_STATUS_CODE_NOT_INITIALIZED;

static uint32_t ulAlertType = 0;			/* The alert type */

static uint8_t ucSymptomCount = 0;			/* The number of registered symptoms. Must be <= DFM_CFG_MAX_SYMPTOMS */

DfmSymptom_t xSymptoms[DFM_CFG_MAX_SYMPTOMS];

/*******************************************************************************
 *
 * Private function definitions
 *
 ******************************************************************************/
static uint32_t prvDfmSetDisabled(uint32_t ulDisabled);
static void prvDfmResetSymptoms(void);
static uint32_t prvDfmSendNonVolatileDataToCloud();
static void prvDfmCopySessionData(DfmSession_t* pxDst, DfmSession_t* pxSrc);
static void prvDfmCopyHeaderData(DfmAlertHeader_t* pxDst, DfmAlertHeader_t* pxSrc);
static uint32_t prvDfmWriteSessionData(DfmSession_t* pxDfmSession);
static uint32_t prvDfmWriteHeaderData(DfmAlertHeader_t* pxDfmHeader);
static uint32_t prvDfmSaveSectionData(uint32_t ulSection);

/*******************************************************************************
 *
 * Public functions
 *
 ******************************************************************************/
 
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
uint32_t ulDfmInit(const char* szFirmwareVersion)
{
	uint32_t i;
	void* pvData = 0;

	/* First check if DFM is disabled */
	if (DFM_HARDWARE_PORT_BEGIN_READ(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_READ_NONVOLATILE_MEMORY(DFM_SECTION_SETTINGS, &pvData, sizeof(DfmSettings_t), 0, &i) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_END_READ(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED;
		return -1;
	}

	/* Assign it to variable for easy access */
	//ulDfmDisabled = ((DfmSettings_t*)pvData)->ulDfmDisabled;  need to implement flash read.
	ulDfmDisabled = 0;

	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_NOT_INITIALIZED)
	{
		if (ulDfmStatus == DFM_STATUS_CODE_OK)
		{
			/* We seem to have already initialized. Let's hope all is well. */
			return 0;
		}
		else
		{
			/* Something has previously gone wrong. */
			return -1;
		}
	}

#if (defined(DFM_CFG_GET_UNIQUE_SESSION_ID))
	DFM_CFG_GET_UNIQUE_SESSION_ID(cUniqueSessionIdBuffer, sizeof(cUniqueSessionIdBuffer));
#else
	/* TODO: This will not work. Will be almost identical values each time. */
	snprintf(cUniqueSessionIdBuffer, DFM_SESSION_ID_MAX_LEN, "%08X_%08X", (unsigned int)(RecorderDataPtr->absTimeLastEvent), (unsigned int)(TRC_HWTC_COUNT));
#endif

	if (DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE < sizeof(DfmAlertHeader_t))
	{
		/* DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE must be larger than the header size */
		ulDfmStatus = DFM_STATUS_CODE_TOO_SMALL_PAYLOAD_SIZE;
		return -1;
	}

	/* TODO: Can we use strlen? Implement our own? */
	if (DFM_CFG_FIRMWARE_VERSION_MAX_LEN < strlen(szFirmwareVersion))
	{
		/* DFM_CFG_FIRMWARE_VERSION_MAX_LEN must be larger than the string length */
		ulDfmStatus = DFM_STATUS_CODE_TOO_SMALL_FIRMWARE_VERSION_BUFFER;
		return -1;
	}

	if (DFM_HARDWARE_PORT_INIT() != 0)
	{
		/* DFM_HARDWARE_PORT_INIT() failed */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_INIT_FAILED;
		return -1;
	}

	if (DFM_KERNEL_PORT_INIT() != 0)
	{
		/* DFM_KERNEL_PORT_INIT() failed */
		ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_INIT_FAILED;
		return -1;
	}

	if (DFM_CLOUD_PORT_INIT() != 0)
	{
		/* DFM_CLOUD_PORT_INIT() failed */
		ulDfmStatus = DFM_STATUS_CODE_CLOUD_PORT_INIT_FAILED;
		return -1;
	}

	ulDfmStatus = DFM_STATUS_CODE_OK;

	for (i = 0; i < DFM_CFG_FIRMWARE_VERSION_MAX_LEN; i++)
	{
		cFirmwareVersionBuffer[i] = szFirmwareVersion[i];
		if (cFirmwareVersionBuffer[i] == 0)
			break;
	}

	/* Send previous Alert if present in NonVolatile storage */
	if (prvDfmSendNonVolatileDataToCloud() != 0)
	{
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * ulDfmEnable
 *
 * Used to enable DFM after it has been disabled. Will also clear the Disabled
 * flag in non-volatile memory.
 *
 * NOTE: Not necessary to call this function unless DFM has been previously
 * disabled by calling ulDfmDisable().
 ******************************************************************************/
uint32_t ulDfmEnable()
{
	if (prvDfmSetDisabled(0) != 0)
	{
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * ulDfmDisable
 *
 * Used to disable DFM. Will also store a Disabled flag in non-volatile memory.
 *
 * NOTE: Must call ulDfmEnable() to enable again.
 ******************************************************************************/
uint32_t ulDfmDisable()
{
	if (prvDfmSetDisabled(1) != 0)
	{
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * ulDfmIsDisabled
 *
 * Used to check if DFM is disabled.
 ******************************************************************************/
uint32_t ulDfmIsDisabled()
{
	return ulDfmDisabled;
}

/*******************************************************************************
 * ulDfmGetStatus
 *
 * Returns the current status code.
 ******************************************************************************/
uint32_t ulDfmGetStatus()
{
	return ulDfmStatus;
}

/*******************************************************************************
 * ulDfmGetBuffer
 *
 * This function will return a buffer that can be used by the Kernel port or
 * Hardware Port to store data in.
 ******************************************************************************/
uint32_t ulDfmGetBuffer(void** ppvData, uint32_t* pulSize)
{
	*ppvData = (void*)ucDataBuffer;
	*pulSize = sizeof(ucDataBuffer);

	return 0;
}

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
uint32_t ulDfmAlert(uint32_t _ulAlertType)
{
	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_OK)
	{
		return -1;
	}

	/* Always clear the alert structure before creating a new alert, so we don't mix symptoms from multiple alerts. */
	ulDfmAlertReset();

	ulAlertType = _ulAlertType;
	
	return 0;
}

/*******************************************************************************
 * ulDfmAlertReset
 *
 * Can be used to reset the Alert and Symptoms data.
 * Will also be automatically called each time ulDfmAlert() is called.
 ******************************************************************************/
uint32_t ulDfmAlertReset()
{
	ulAlertType = 0;

	prvDfmResetSymptoms();

	return 0;
}

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
uint32_t ulDfmAddSymptom(uint32_t ulSymptomId, uint32_t ulValue)
{
	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_OK)
	{
		return -1;
	}

	if (ucSymptomCount < DFM_CFG_MAX_SYMPTOMS)
	{
		xSymptoms[ucSymptomCount].ulSymptomId = ulSymptomId;
		xSymptoms[ucSymptomCount].ulValue = ulValue;
		ucSymptomCount++;
	}
	else
	{
		/* Incrementing ucSymptomCount to indicate how many Symptoms have been added */
		ucSymptomCount++;
		ulDfmStatus = DFM_STATUS_CODE_MAX_SYMPTOMS_EXCEEDED;
		return -1;
	}
	
	return 0;
}

/*******************************************************************************
 * ulDfmSendDataToCloud
 *
 * Sends the Alert and Symptoms.
 ******************************************************************************/
uint32_t ulDfmSendDataToCloud(void)
{
	void* pvData = (void*)ucDataBuffer;
	uint32_t ulBytesTransferred = 0;
	uint32_t ulTraceSize = 0;
	uint32_t ulBytesOffset = 0;
	uint32_t ulBytesToSend = 0;
	uint32_t ulSlice = 0;
	uint32_t ulSlices = 1;	/* 1 slice for the header */

	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_OK)
	{
		return -1;
	}

	if (prvDfmWriteHeaderData((DfmAlertHeader_t*)pvData) != 0)
	{
		/* TODO: Something? Just return? */
		return -1;
	}

	ulTraceSize = ((DfmAlertHeader_t*)pvData)->ulTraceSize;

	/* Add the slices required for the trace */
	ulSlices += ((ulTraceSize - 1) / DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE) + 1;

	if (DFM_CLOUD_PORT_SEND(pvData, sizeof(DfmAlertHeader_t), cUniqueSessionIdBuffer, ulTraceCounter, "header", ulSlice, ulSlices - 1, &ulBytesTransferred) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_CLOUD_PORT_SEND_FAILED;
		return -1;
	}

	if (sizeof(DfmAlertHeader_t) != ulBytesTransferred)
	{
		/* TODO: Do something? */
	}

	ulSlice++;

	if (DFM_KERNEL_PORT_BEGIN_READ_TRACE() != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_BEGIN_READ_TRACE_FAILED;
		return -1;
	}

	while (ulSlice < ulSlices)
	{
		vTaskDelay(5);

		ulBytesToSend = ulTraceSize - ulBytesOffset;

		/* Cap at max payload size? */
		if (ulBytesToSend > DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE)
		{
			ulBytesToSend = DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE;
		}

		if (DFM_KERNEL_PORT_READ_TRACE(&pvData, ulBytesToSend, ulBytesOffset, &ulBytesTransferred) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_READ_TRACE_FAILED;
			return -1;
		}

		if (ulBytesToSend != ulBytesTransferred)
		{
			/* TODO: How to handle this???? */
		}

		if (DFM_CLOUD_PORT_SEND(pvData, ulBytesToSend, cUniqueSessionIdBuffer, ulTraceCounter, "trace", ulSlice, ulSlices - 1, &ulBytesTransferred) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_CLOUD_PORT_SEND_FAILED;
			return -1;
		}

		if (ulBytesToSend != ulBytesTransferred)
		{
			/* TODO: How to handle this???? */
		}

		ulBytesOffset += ulBytesTransferred;
		ulSlice++;

	}

	if (DFM_KERNEL_PORT_END_READ_TRACE() != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_END_READ_TRACE_FAILED;
		return -1;
	}

	ulTraceCounter++;

	return 0;
}

/*******************************************************************************
 * ulDfmSaveDataToNonVolatileMemory
 *
 * This function will attempt to store the Alert, Symptoms and any other
 * necessary information in Non-Volatile memory. This should be used when
 * sending the Alert right away will fail, i.e. after a Hard Fault.
 * The Alert will be automatically sent after reboot when ulDfmInit() is called.
 ******************************************************************************/
uint32_t ulDfmSaveDataToNonVolatileMemory(void)
{
	uint32_t ulFlag = 0;

	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_OK)
	{
		return -1;
	}

	/* Get Session Buffer Read pointer */
	if (DFM_HARDWARE_PORT_HAS_DATA(DFM_SECTION_SESSION, &ulFlag) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_HAS_DATA_FAILED;
		return -1;
	}

	if (ulFlag != 0)
	{
		/* There is already data! */
		/* TODO: It wasn't previously sent, so clear it and continue? */
		ulDfmStatus = DFM_STATUS_CODE_NONVOLATILE_DATA_ALREADY_PRESENT;
		return -1;
	}


	if (prvDfmSaveSectionData(DFM_SECTION_ALERT_HEADER) != 0)
	{
		return -1;
	}

	if (prvDfmSaveSectionData(DFM_SECTION_TRACE_DATA) != 0)
	{
		return -1;
	}

	/* We save Session last since it is used to indicate if there is saved data */
	if (prvDfmSaveSectionData(DFM_SECTION_SESSION) != 0)
	{
		return -1;
	}

	ulTraceCounter++;

	return 0;
}

/*******************************************************************************
 *
 * Private functions
 *
 ******************************************************************************/

/*******************************************************************************
 * prvDfmSetDisabled
 *
 * This function will write ulDisabled to non-volatile memory, if necessary.
 ******************************************************************************/
static uint32_t prvDfmSetDisabled(uint32_t ulDisabled)
{
	uint32_t ulSize;
	void* pvData;
	DfmSettings_t xSettings;

	/* First we read the current state to see if we need to do anything */
	if (DFM_HARDWARE_PORT_BEGIN_READ(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_READ_NONVOLATILE_MEMORY(DFM_SECTION_SETTINGS, &pvData, sizeof(DfmSettings_t), 0, &ulSize) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_END_READ(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED;
		return -1;
	}

	ulDfmDisabled = ((DfmSettings_t*)pvData)->ulDfmDisabled;

	if (ulDfmDisabled == ulDisabled)
	{
		/* Already set. Do nothing. */
		return 0;
	}

	xSettings.ulDfmDisabled = ulDisabled;

	/* Must update non-volatile memory */
	if (DFM_HARDWARE_PORT_BEGIN_WRITE(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_WRITE_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_WRITE_NONVOLATILE_MEMORY(DFM_SECTION_SETTINGS, &xSettings, sizeof(DfmSettings_t), 0, &ulSize) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_WRITE_NONVOLATILE_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_END_WRITE(DFM_SECTION_SETTINGS) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_WRITE_FAILED;
		return -1;
	}

	ulDfmDisabled = ulDisabled;

	return 0;
}

/*******************************************************************************
 * prvDfmResetSymptoms
 *
 * This function will loop over ALL symptom slots and reset them.
 ******************************************************************************/
static void prvDfmResetSymptoms()
{
	for (int i = 0; i < DFM_CFG_MAX_SYMPTOMS; i++)
	{
		xSymptoms[i].ulSymptomId = 0;
		xSymptoms[i].ulValue = 0;
	}
	
	ucSymptomCount = 0;
}

/*******************************************************************************
 * prvDfmWriteSessionData
 *
 * This function will write all the required data to pxDfmSession.
 *
 * @param pxDfmSession Pointer to a DfmSession_t instance.
 ******************************************************************************/
static uint32_t prvDfmWriteSessionData(DfmSession_t* pxDfmSession)
{
	uint32_t i;

	pxDfmSession->ucVersion = DFM_VERSION;
	pxDfmSession->ulTraceCounter = ulTraceCounter;

	for (i = 0; i < sizeof(cUniqueSessionIdBuffer); i++)
	{
		pxDfmSession->cUniqueSessionIdBuffer[i] = cUniqueSessionIdBuffer[i];
	}

	return 0;
}

/*******************************************************************************
 * prvDfmWriteHeaderData
 *
 * This function will write all the required data to pxDfmHeader.
 *
 * @param pxDfmHeader Pointer to a DfmAlertHeader_t instance.
 ******************************************************************************/
static uint32_t prvDfmWriteHeaderData(DfmAlertHeader_t* pxDfmHeader)
{
	uint32_t i;
	uint32_t ulTraceSize = 0;

	if (DFM_KERNEL_PORT_GET_TRACE_SIZE(&ulTraceSize) != 0)
	{
		ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_GET_TRACE_SIZE_FAILED;
		return -1;
	}

	pxDfmHeader->ucStartMarkers[0] = 0x50; /* 'P' */
	pxDfmHeader->ucStartMarkers[1] = 0x44; /* 'D' */
	pxDfmHeader->ucStartMarkers[2] = 0x66; /* 'f' */
	pxDfmHeader->ucStartMarkers[3] = 0x6D; /* 'm' */

	pxDfmHeader->usEndianness = 0x0FF0;
	pxDfmHeader->ucVersion = DFM_VERSION;
	pxDfmHeader->ucMaxSymptoms = DFM_CFG_MAX_SYMPTOMS;
	pxDfmHeader->ucFirmwareVersionSize = DFM_FIRMWARE_VERSION_MAX_LEN;
	pxDfmHeader->ucSymptomCount = ucSymptomCount;
	pxDfmHeader->ulTraceSize = ulTraceSize;

	pxDfmHeader->ulAlertType = ulAlertType;

	for (i = 0; i < DFM_CFG_MAX_SYMPTOMS; i++)
	{
		pxDfmHeader->xSymptoms[i] = xSymptoms[i];
	}

	for (i = 0; i < DFM_FIRMWARE_VERSION_MAX_LEN; i++)
	{
		pxDfmHeader->cFirmwareVersionBuffer[i] = cFirmwareVersionBuffer[i];
	}

	pxDfmHeader->ucEndMarkers[0] = 0x6D; /* 'm' */
	pxDfmHeader->ucEndMarkers[1] = 0x66; /* 'f' */
	pxDfmHeader->ucEndMarkers[2] = 0x44; /* 'D' */
	pxDfmHeader->ucEndMarkers[3] = 0x50; /* 'P' */

	return 0;
}

/*******************************************************************************
 * prvDfmCopySessionData
 *
 * This function will write copy all data from pxSrc to pxDst.
 *
 * @param pxDst Pointer to a destination DfmSession_t instance.
 * @param pxSrc Pointer to a source DfmSession_t instance.
 ******************************************************************************/
static void prvDfmCopySessionData(DfmSession_t* pxDst, DfmSession_t* pxSrc)
{
	uint32_t i;

	pxDst->ucVersion = pxSrc->ucVersion;
	pxDst->ulTraceCounter = pxSrc->ulTraceCounter;

	for (i = 0; i < sizeof(pxDst->cUniqueSessionIdBuffer); i++)
	{
		pxDst->cUniqueSessionIdBuffer[i] = pxSrc->cUniqueSessionIdBuffer[i];
	}
}

/*******************************************************************************
 * prvDfmCopyHeaderData
 *
 * This function will write copy all data from pxSrc to pxDst.
 *
 * @param pxDst Pointer to a destination DfmAlertHeader_t instance.
 * @param pxSrc Pointer to a source DfmAlertHeader_t instance.
 ******************************************************************************/
static void prvDfmCopyHeaderData(DfmAlertHeader_t* pxDst, DfmAlertHeader_t* pxSrc)
{
	uint32_t i;

	pxDst->ucStartMarkers[0] = pxSrc->ucStartMarkers[0];
	pxDst->ucStartMarkers[1] = pxSrc->ucStartMarkers[1];
	pxDst->ucStartMarkers[2] = pxSrc->ucStartMarkers[2];
	pxDst->ucStartMarkers[3] = pxSrc->ucStartMarkers[3];

	pxDst->usEndianness = pxSrc->usEndianness;
	pxDst->ucVersion = pxSrc->ucVersion;
	pxDst->ucMaxSymptoms = pxSrc->ucMaxSymptoms;
	pxDst->ucFirmwareVersionSize = pxSrc->ucFirmwareVersionSize;
	pxDst->ucSymptomCount = pxSrc->ucSymptomCount;
	pxDst->ulTraceSize = pxSrc->ulTraceSize;

	pxDst->ulAlertType = pxSrc->ulAlertType;

	for (i = 0; i < pxSrc->ucMaxSymptoms; i++)
	{
		pxDst->xSymptoms[i] = pxSrc->xSymptoms[i];
	}

	for (i = 0; i < pxSrc->ucFirmwareVersionSize; i++)
	{
		pxDst->cFirmwareVersionBuffer[i] = pxSrc->cFirmwareVersionBuffer[i];
	}

	pxDst->ucEndMarkers[0] = pxSrc->ucEndMarkers[0];
	pxDst->ucEndMarkers[1] = pxSrc->ucEndMarkers[1];
	pxDst->ucEndMarkers[2] = pxSrc->ucEndMarkers[2];
	pxDst->ucEndMarkers[3] = pxSrc->ucEndMarkers[3];
}

/*******************************************************************************
 * prvDfmSaveSectionData
 *
 * This function will save a Section to Non-Volatile memory.
 *
 * @param ulSection The section that should be saved.
 ******************************************************************************/
static uint32_t prvDfmSaveSectionData(uint32_t ulSection)
{
	void *pvData = (void*)ucDataBuffer;
	uint32_t ulTransferSize;
	uint32_t ulTransferred = 0;
	uint32_t ulToTransfer = 0;
	uint32_t ulBytesOffset = 0;

	
	switch (ulSection)
	{
	case DFM_SECTION_NA:
		break;
	case DFM_SECTION_SETTINGS:
		((DfmSettings_t*)pvData)->ulDfmDisabled = ulDfmDisabled;
		ulTransferSize = sizeof(DfmSettings_t);
		break;
	case DFM_SECTION_SESSION:
		/* Write Session data to buffer */
		if (prvDfmWriteSessionData((DfmSession_t*)pvData) != 0)
		{
			return -1;
		}
		ulTransferSize = sizeof(DfmSession_t);
		break;
	case DFM_SECTION_ALERT_HEADER:
		/* Write Alert Header data to buffer */
		if (prvDfmWriteHeaderData((DfmAlertHeader_t*)pvData) != 0)
		{
			return -1;
		}
		ulTransferSize = sizeof(DfmAlertHeader_t);
		break;
	case DFM_SECTION_TRACE_DATA:
		if (DFM_KERNEL_PORT_BEGIN_READ_TRACE() != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_BEGIN_READ_TRACE_FAILED;
			return -1;
		}
		if (DFM_KERNEL_PORT_GET_TRACE_SIZE(&ulTransferSize) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_GET_TRACE_SIZE_FAILED;
			return -1;
		}
		break;
	}

	/* Begin Section Write */
	if (DFM_HARDWARE_PORT_BEGIN_WRITE(ulSection) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_WRITE_FAILED;
		return -1;
	}

	/* Perform Write */
	while (ulBytesOffset < ulTransferSize)
	{
		ulToTransfer = ulTransferSize - ulBytesOffset;
		if (ulToTransfer > DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE)
		{
			ulToTransfer = DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE;
		}

		if (ulSection == DFM_SECTION_TRACE_DATA)
		{
			/* Read more trace data */
			if (DFM_KERNEL_PORT_READ_TRACE(&pvData, ulToTransfer, ulBytesOffset, &ulTransferred) != 0)
			{
				/* TODO: Something? Just return? */
				ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_READ_TRACE_FAILED;
				return -1;
			}
		}

		if (DFM_HARDWARE_PORT_WRITE_NONVOLATILE_MEMORY(ulSection, pvData, ulToTransfer, ulBytesOffset, &ulTransferred))
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_WRITE_NONVOLATILE_FAILED;
			return -1;
		}

		ulBytesOffset += ulTransferred;
	}

	/* End Session Write */
	if (DFM_HARDWARE_PORT_END_WRITE(DFM_SECTION_SESSION) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_WRITE_FAILED;
		return -1;
	}

	if (ulSection == DFM_SECTION_TRACE_DATA)
	{
		/* End reading trace data */
		if (DFM_KERNEL_PORT_END_READ_TRACE())
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_KERNEL_PORT_END_READ_TRACE_FAILED;
			return -1;
		}
	}

	return 0;
}

/*******************************************************************************
 * prvDfmSendNonVolatileDataToCloud
 *
 * This function will attempt to send a previously stored Alert
 *
 ******************************************************************************/
static uint32_t prvDfmSendNonVolatileDataToCloud()
{
	void* pvData = 0;
	DfmSession_t xDfmSession = { 0 };
	DfmAlertHeader_t xDfmHeader = { 0 };
	uint32_t ulFlag = 0;
	uint32_t ulTransferred = 0;
	uint32_t ulBytesOffset = 0;
	uint32_t ulToTransfer = 0;
	uint32_t ulSlice = 0;
	uint32_t ulSlices = 1; /* 1 slice for the header */

	if (ulDfmDisabled != 0)
	{
		/* We are disabled. Silently do nothing. */
		return 0;
	}

	if (ulDfmStatus != DFM_STATUS_CODE_OK)
	{
		return -1;
	}

	/* Is there data to send? */
	if (DFM_HARDWARE_PORT_HAS_DATA(DFM_SECTION_SESSION, &ulFlag) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_HAS_DATA_FAILED;
		return -1;
	}

	if (ulFlag == 0)
	{
		/* No data */
		return 0;
	}

	/*DFM_CFG_PRINTF("Old data found. Sending...");*/

	/* There is data to send. */
	if (DFM_HARDWARE_PORT_BEGIN_READ(DFM_SECTION_SESSION) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_READ_NONVOLATILE_MEMORY(DFM_SECTION_SESSION, &pvData, sizeof(DfmSession_t), 0, &ulTransferred) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED;
		return -1;
	}

	if (DFM_HARDWARE_PORT_END_READ(DFM_SECTION_SESSION) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED;
		return -1;
	}

	if (sizeof(DfmSession_t) != ulTransferred)
	{
		/* TODO: How to handle this???? */
	}

	/* Only continue if the version matches. Otherwise this is old data and we will discard it before function returns */
	if (((DfmSession_t*)pvData)->ucVersion == DFM_VERSION)
	{
		prvDfmCopySessionData(&xDfmSession, (DfmSession_t*)pvData);


		if (DFM_HARDWARE_PORT_BEGIN_READ(DFM_SECTION_ALERT_HEADER) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED;
			return -1;
		}

		if (DFM_HARDWARE_PORT_READ_NONVOLATILE_MEMORY(DFM_SECTION_ALERT_HEADER, &pvData, sizeof(DfmAlertHeader_t), 0, &ulTransferred) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED;
			return -1;
		}

		if (DFM_HARDWARE_PORT_END_READ(DFM_SECTION_ALERT_HEADER) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED;
			return -1;
		}

		if (sizeof(DfmAlertHeader_t) != ulTransferred)
		{
			/* TODO: How to handle this???? */
		}

		prvDfmCopyHeaderData(&xDfmHeader, (DfmAlertHeader_t*)pvData);

		/* Add the slices required for the trace */
		ulSlices += ((xDfmHeader.ulTraceSize - 1) / DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE) + 1;

		if (DFM_CLOUD_PORT_SEND(pvData, sizeof(DfmAlertHeader_t), xDfmSession.cUniqueSessionIdBuffer, xDfmSession.ulTraceCounter, "header", ulSlice, ulSlices - 1, &ulTransferred) != 0)
		{
			/* TODO: Something? Just return? */
			ulDfmStatus = DFM_STATUS_CODE_CLOUD_PORT_SEND_FAILED;
			return -1;
		}

		if (sizeof(DfmAlertHeader_t) != ulTransferred)
		{
			/* TODO: How to handle this???? */
		}

		/* Time to send all the stored trace data */
		ulSlice++;

		while (ulSlice < ulSlices)
		{
			ulToTransfer = xDfmHeader.ulTraceSize - ulBytesOffset;

			/* Cap at max payload size? */
			if (ulToTransfer > DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE)
			{
				ulToTransfer = DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE;
			}

			if (DFM_HARDWARE_PORT_BEGIN_READ(DFM_SECTION_TRACE_DATA) != 0)
			{
				/* TODO: Something? Just return? */
				ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_BEGIN_READ_FAILED;
				return -1;
			}

			if (DFM_HARDWARE_PORT_READ_NONVOLATILE_MEMORY(DFM_SECTION_TRACE_DATA, &pvData, ulToTransfer, ulBytesOffset, &ulTransferred) != 0)
			{
				/* TODO: Something? Just return? */
				ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_READ_NONVOLATILE_FAILED;
				return -1;
			}

			if (DFM_HARDWARE_PORT_END_READ(DFM_SECTION_TRACE_DATA) != 0)
			{
				/* TODO: Something? Just return? */
				ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_END_READ_FAILED;
				return -1;
			}

			if (ulToTransfer != ulTransferred)
			{
				/* TODO: How to handle this???? */
			}

			if (DFM_CLOUD_PORT_SEND(pvData, ulToTransfer, xDfmSession.cUniqueSessionIdBuffer, xDfmSession.ulTraceCounter, "trace", ulSlice, ulSlices - 1, &ulTransferred) != 0)
			{
				/* TODO: Something? Just return? */
				ulDfmStatus = DFM_STATUS_CODE_CLOUD_PORT_SEND_FAILED;
				return -1;
			}

			if (ulToTransfer != ulTransferred)
			{
				/* TODO: How to handle this???? */
			}

			ulBytesOffset += ulTransferred;
			ulSlice++;
		}
	}

	configPRINTF(("\r\nDevAlert - Trace data found stored on the device.\r\n"));
	configPRINTF(("\r\nDevAlert - Trace data was uploaded to the cloud.\r\n"));


	/* Clear the data so we don't send it again */
	if (DFM_HARDWARE_PORT_CLEAR_DATA(DFM_SECTION_SESSION) != 0)
	{
		/* TODO: Something? Just return? */
		ulDfmStatus = DFM_STATUS_CODE_HARDWARE_PORT_CLEAR_DATA_FAILED;
		return -1;
	}

	/*DFM_CFG_PRINTF("Old data cleared.");*/

	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */
