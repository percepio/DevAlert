/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmKernelPort.c
 *
 * The FreeRTOS kernel port implementation.
 * Requires the Percepio Trace Recorder.
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

#include "dfmKernelPort.h"
#include "task.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#if (TRC_CFG_RECORDER_MODE != TRC_RECORDER_MODE_SNAPSHOT)
#error "Only Snapshot recorder mode is supported with DFM!"
#endif

uint32_t ulDfmFreeRTOSPortDelay(uint32_t ulDelay)
{
	vTaskDelay(ulDelay);

	return 0;
}

uint32_t ulDfmFreeRTOSPortReadTraceData(void** ppvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesRead)
{
	/* Move the trace data pointer by ulOffset */
	*ppvData = xTraceGetTraceBuffer() + ulOffset;

	/* Check how much data we can "read" (access) */
	*pulBytesRead = uiTraceGetTraceBufferSize() - ulOffset;
	if (*pulBytesRead > ulSize)
	{
		*pulBytesRead = ulSize;
	}

	return 0;
}

uint32_t ulDfmFreeRTOSPortGetTraceSize(uint32_t* pulSize)
{
	*pulSize = uiTraceGetTraceBufferSize();

	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */
