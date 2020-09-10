/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * percepio_dfmConfig.h
 *
 * The Percepio DFM user configuration.
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

#ifndef PERCEPIO_DFM_CONFIG_H
#define PERCEPIO_DFM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOSConfig.h"

/* DFM_CFG_ENABLED - Global flag used to completely exclude all DFM functionality from compilation */
#define DFM_CFG_ENABLED 1

/* DFM_CFG_MAX_SYMPTOMS - The max number of symptoms for each alert */
#define DFM_CFG_MAX_SYMPTOMS 8

/* DFM_CFG_FIRMWARE_VERSION_MAX_LEN - The max firmware version string length */
#define DFM_CFG_FIRMWARE_VERSION_MAX_LEN 32

/* DFM_CFG_DELAY_BETWEEN_SEND - A value that will be used to create a delay between transfers. Was necessary in certain situations. */
#define DFM_CFG_DELAY_BETWEEN_SEND 10

/* N/A */
#define DFM_CFG_PRINTF configPRINTF

/* DFM_CFG_GET_UNIQUE_SESSION_ID - Should retrieve a unique Session ID and write it to the buffer */
/*#define DFM_CFG_GET_UNIQUE_SESSION_ID(cUniqueSessionBuffer, ulBufferSize) \
	{ \
		uint32_t ulRandomNumber = GetRandomNumber(); \
		snprintf(cUniqueSessionBuffer, ulBufferSize, "%08X", (unsigned int)ulRandomNumber); \
	}
*/

#ifdef __cplusplus
}
#endif

#endif /* PERCEPIO_DFM_CONFIG_H */
