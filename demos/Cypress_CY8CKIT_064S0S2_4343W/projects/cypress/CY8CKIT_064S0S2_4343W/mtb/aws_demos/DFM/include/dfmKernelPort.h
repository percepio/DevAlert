/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmKernelPort.h
 *
 * The FreeRTOS kernel port header.
 * Requires the Percepio Trace Recorder.
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

#ifndef DFM_KERNEL_PORT_H
#define DFM_KERNEL_PORT_H

#include "percepio_dfmConfig.h"
#include "trcRecorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This define is mandatory and is used to determine if tracing is disabled so we can disable DFM as well */
/* TODO: Should we have this? */
#define DFM_TRACE_RECORDER_IS_ENABLED (TRC_USE_TRACEALYZER_RECORDER == 1)

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

uint32_t ulDfmFreeRTOSPortDelay(uint32_t ulDelay);

uint32_t ulDfmFreeRTOSPortReadTraceData(void** ppvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesRead);

uint32_t ulDfmFreeRTOSPortGetTraceSize(uint32_t* pulSize);

/*******************************************************************************
 * DFM_KERNEL_PORT_INIT
 *
 * Will be called during initialization of DFM.
 * Map it to a function if something needs to be done.
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_INIT() (0)

/*******************************************************************************
 * DFM_KERNEL_PORT_DELAY
 *
 * Map it to a function that calls on the kernel Delay service.
 *
 * @param ulDelay The time to delay.
 ******************************************************************************/
#define DFM_KERNEL_PORT_DELAY(ulDelay) ulDfmFreeRTOSPortDelay(ulDelay)

/*******************************************************************************
 * DFM_KERNEL_PORT_GET_TRACE_SIZE
 *
 * Map it to a function that retrieves the trace data size.
 *
 * @param pulSize Set to trace data size.
 ******************************************************************************/
#define DFM_KERNEL_PORT_GET_TRACE_SIZE(pulSize) ulDfmFreeRTOSPortGetTraceSize(pulSize)

/*******************************************************************************
 * DFM_KERNEL_PORT_READ_TRACE
 *
 * Map it to a function that reads the trace data to a buffer and returns said
 * buffer via ppvData. If a buffer is needed to hold the data (data is read
 * from file), call ulDfmGetBuffer(...), read to that buffer, and set *ppvData
 * to that address.
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ppvData Set to the buffer holding the read trace data.
 * @param ulSize The requested data size.
 * @param ulOffset The read offset. Can be used to offset a memory read pointer.
 * Ignore this parameter if data is read from a file.
 * @param pulBytesRead The amount of trace data that was read.
 ******************************************************************************/
#define DFM_KERNEL_PORT_READ_TRACE(ppvData, ulSize, ulOffset, pulBytesRead) ulDfmFreeRTOSPortReadTraceData(ppvData, ulSize, ulOffset, pulBytesRead)

/*******************************************************************************
 * DFM_KERNEL_PORT_BEGIN_READ_TRACE
 *
 * Map it to a function that begins trace reading (i.e. opening a file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_BEGIN_READ_TRACE() (0) /* On FreeRTOS we can access the trace directly in memory*/

/*******************************************************************************
 * DFM_KERNEL_PORT_END_READ_TRACE
 *
 * Map it to a function that ends trace reading (i.e. closing a file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_END_READ_TRACE() (0) /* On FreeRTOS we can access the trace directly in memory */

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */

#ifdef __cplusplus
}
#endif

#endif /* DFM_KERNEL_PORT_H */
