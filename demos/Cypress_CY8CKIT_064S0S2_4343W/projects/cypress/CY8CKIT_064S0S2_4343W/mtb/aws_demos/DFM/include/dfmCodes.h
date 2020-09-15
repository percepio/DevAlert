/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCodes.h
 *
 * Contains definitions for all Alerts and Symptoms.
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

#ifndef DFM_CODES_H
#define DFM_CODES_H

/* Alert types */
#define DFM_ALERT_ASSERT_FAILED 1
#define DFM_ALERT_MALLOC_FAILED 2
#define DFM_ALERT_HARDFAULT 3
#define DFM_ALERT_SWIPE_LEFT 4

#define DFM_ALERT_BFSR_BFAR 5
#define DMF_ALERT_BFSR_LSPERR 6
#define DMF_ALERT_BFSR_STKERR 7
#define DMF_ALERT_BFSR_UNSTKERR 8
#define DMF_ALERT_BFSR_IMPRECISERR 9
#define DMF_ALERT_BFSR_PRECISEERR 10
#define DMF_ALERT_BFSR_IBUSERR 11

#define DMF_ALERT_UFSR_DIVBYZERO 12
#define DMF_ALERT_UFSR_UNALIGNED 13
#define DMF_ALERT_UFSR_NOCP 14
#define DMF_ALERT_UFSR_INVPC 15
#define DMF_ALERT_UFSR_INVSTATE 16
#define DMF_ALERT_UFSR_UNDEFINSTR 17

#define DMF_ALERT_MMFSR_MMARVALID 18
#define DMF_ALERT_MMFSR_MLSPERR 19
#define DMF_ALERT_MMFSR_MSTERR 20
#define DMF_ALERT_MMFSR_MUNSTKERR 21
#define DMF_ALERT_MMFSR_DACCVIOL 22
#define DMF_ALERT_MMFSR_IACCVIOL 23

/* Symptoms */
#define DFM_SYMPTOM_CURRENT_TASK 1
#define DFM_SYMPTOM_FILE 2
#define DFM_SYMPTOM_FUNCTION 3
#define DFM_SYMPTOM_LINE 4
#define DFM_SYMPTOM_PC 5
#define DFM_SYMPTOM_STACKPTR 6
#define DFM_SYMPTOM_ARM_SCB_FCSR 7 /*SCB->CFSR register on Arm MCUs, giving more details about fault exceptions */
#define DFM_SYMPTOM_PRODUCT 8

#endif /* DFM_CODES_H */
