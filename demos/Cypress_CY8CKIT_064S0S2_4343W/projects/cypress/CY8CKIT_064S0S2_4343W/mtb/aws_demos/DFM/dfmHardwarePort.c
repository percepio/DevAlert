/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmHardwarePort.c
 *
 * The hardware port implementation.
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

#include "dfmHardwarePort.h"
#include "dfmCloudPortConfig.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#include "percepio_dfm.h"
#include "cy_em_eeprom.h"

#define DFM_HARDWARE_PORT_MAX_WRITE_SIZE 2000

cy_stc_eeprom_config_t DFM_Em_EEPROM_config =
{
        .eepromSize = 10000,
        .blockingWrite = 1,
        .redundantCopy = 1,
        .wearLevelingFactor = 2,
		.simpleMode = 1,
};

cy_stc_eeprom_context_t DFM_Em_EEPROM_context;

/* These are stored in flash. */
DfmSettings_t FLASH_SETTINGS __attribute__( ( section( "DFM_LOC" ), aligned (8) ) ) = { 0 };
DfmSession_t FLASH_SESSION __attribute__( ( section( "DFM_LOC" ), aligned (8) ) ) = { 0 };
DfmAlertHeader_t FLASH_ALERT_HEADER __attribute__( ( section( "DFM_LOC" ), aligned (8) ) ) = { 0 };
uint8_t FLASH_TRACE_DATA[8000] __attribute__( ( section( "DFM_LOC" ), aligned (8) ) ) = { 0 };

uint8_t InitDFM_EEPROM()
{
	DFM_Em_EEPROM_config.userFlashStartAddr = (uint32_t)&FLASH_SETTINGS;
	return Cy_Em_EEPROM_Init(&DFM_Em_EEPROM_config, &DFM_Em_EEPROM_context);
}

/* TODO: Describe usage */
uint32_t ulDfmHardwarePortHasData(uint32_t ulSection, uint32_t* pulFlag)
{
	uint32_t ulDummy = 0;
	uint32_t* pulValue = 0;
	*pulFlag = 0;
	/* Retrieve first first uint32_t of section data */
	if (ulDfmHardwarePortReadFlash(ulSection, (void**)&pulValue, sizeof(uint32_t), 0, &ulDummy) != 0)
	{
		return -1;
	}

	/* Check if it is non-zero */
	if (*pulValue != 0)
	{
		*pulFlag = 1;
	}

	return 0;
}

/* TODO: Describe usage */
/* Clears first uint32_t of section data. */
uint32_t ulDfmHardwarePortClearData(uint32_t ulSection)
{
	uint32_t ulClear = 0;
	uint32_t ulDummy;
	/* Ignore ulSection */

	/* Erase all the flash content. Send in DFM_SECTION_ALERT_HEADER since that is the only value that will cause it to be erased. */
	ulDfmHardwarePortUnlockEraseFlash(DFM_SECTION_ALERT_HEADER);
	ulDfmHardwarePortWriteFlash(DFM_SECTION_SESSION, &ulClear, sizeof(ulClear), 0, &ulDummy);
	/* TODO: FIX THIS */
	return 0;
}

uint32_t ulDfmHardwarePortUnlockEraseFlash(uint32_t ulSection)
{
	/* Only unlock/erase when ulSection == DFM_SECTION_SETTINGS since that is the first one, or even the only one, to be written */
	if (ulSection != DFM_SECTION_SETTINGS)
		return 0;

	if (Cy_Em_EEPROM_Erase(&DFM_Em_EEPROM_context) == 0){
		return 0;
	}
	else{
		return -1;
	}
}

/* TODO: Describe usage */
uint32_t ulDfmHardwarePortWriteFlash(uint32_t ulSection, void* pvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesWritten)
{
	uint32_t ulDst = 0;

	/* Get destination address */
	switch (ulSection)
	{
	case DFM_SECTION_SETTINGS:
		ulDst = (uint32_t)&FLASH_SETTINGS + ulOffset;
		break;
	case DFM_SECTION_SESSION:
		ulDst = (uint32_t)&FLASH_SESSION + ulOffset;
		break;
	case DFM_SECTION_ALERT_HEADER:
		ulDst = (uint32_t)&FLASH_ALERT_HEADER + ulOffset;
		break;
	case DFM_SECTION_TRACE_DATA:
		ulDst = (uint32_t)&FLASH_TRACE_DATA + ulOffset;
		break;
	}

	/* We assume everything goes well. FLASH_write_at will return non-zero if it doesn't. */
	*pulBytesWritten = ulSize;

	/* Write to destination */

	return Cy_Em_EEPROM_Write(ulDst - DFM_Em_EEPROM_config.userFlashStartAddr, pvData, ulSize, &DFM_Em_EEPROM_context);
}

/* TODO: Describe usage */
uint32_t ulDfmHardwarePortReadFlash(uint32_t ulSection, void** ppvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesRead)
{
	/* Since this is Flash, don't do a read to a buffer, just modify the pointer by ulOffset */
	switch (ulSection)
	{
	case DFM_SECTION_SETTINGS:
		*ppvData = (void*)((uint32_t)&FLASH_SETTINGS + ulOffset);
		break;
	case DFM_SECTION_SESSION:
		*ppvData = (void*)((uint32_t)&FLASH_SESSION + ulOffset);
		break;
	case DFM_SECTION_ALERT_HEADER:
		*ppvData = (void*)((uint32_t)&FLASH_ALERT_HEADER + ulOffset);
		break;
	case DFM_SECTION_TRACE_DATA:
		*ppvData = (void*)((uint32_t)&FLASH_TRACE_DATA + ulOffset);
		break;
	}
	*pulBytesRead = ulSize;

	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */
