/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCloudPort.c
 *
 * The AWS cloud port implementation.
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

#include "dfmCloudPort.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

/* MQTT include. */
#include "iot_mqtt_agent.h"
#include "aws_clientcredential.h"

/* TODO: Can we expect a string.h to be present? */
#include <string.h>

char cTopicBuffer[DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE] = { 0 };
MQTTAgentHandle_t xHandle;

#define NOCONNECTTIME 50
#define CONNECTTIME 500
#define PRESSEDTIME 500

uint32_t ulDfmAWSPortInit(void)
{
	MQTTAgentReturnCode_t xMQTTReturn;
	MQTTAgentConnectParams_t xConnectParams = { 0 };

	xConnectParams.pucClientId = ( const uint8_t * ) ( clientcredentialIOT_THING_NAME );
	xConnectParams.usClientIdLength = strlen( clientcredentialIOT_THING_NAME );
	xConnectParams.pcURL = clientcredentialMQTT_BROKER_ENDPOINT;
	xConnectParams.usPort = clientcredentialMQTT_BROKER_PORT;
	xConnectParams.pcCertificate = NULL;
	xConnectParams.ulCertificateSize = 0;
	xConnectParams.pvUserData = NULL;
	xConnectParams.pxCallback = NULL;
	xConnectParams.xFlags = DFM_CFG_CLOUD_PORT_MQTT_AGENT_CONNECT_FLAGS;

	xMQTTReturn = MQTT_AGENT_Create(&xHandle);

	if (xMQTTReturn != eMQTTAgentSuccess)
	{
		return -1;
	}

	xMQTTReturn = MQTT_AGENT_Connect(xHandle, &xConnectParams, DFM_CFG_CLOUD_PORT_TIMEOUT);

	if (xMQTTReturn != eMQTTAgentSuccess)
	{
		return -2;
	}

	return 0;
}

uint32_t ulDfmAWSPortSend(const void* pvData, uint32_t ulSize, const char* szUniqueSessionID, uint32_t ulTraceCounter, const char* szType, uint32_t ulSlice, uint32_t ulSlices, uint32_t* pulBytesSent)
{
	MQTTAgentReturnCode_t xMQTTReturn;
	MQTTAgentPublishParams_t xPP;

	snprintf(cTopicBuffer, DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE, "$aws/rules/DevAlertRule/DevAlert/%s/%lu/%lu-%lu_%s", szUniqueSessionID, ulTraceCounter, ulSlice, ulSlices, szType);

	/* Operation parameters. */
	xPP.pucTopic = (const uint8_t*)cTopicBuffer;
	xPP.usTopicLength = strlen(cTopicBuffer);
	xPP.pvData = pvData;
	xPP.ulDataLength = ulSize;
	xPP.xQoS = eMQTTQoS0;

	xMQTTReturn = MQTT_AGENT_Publish(xHandle, &xPP, DFM_CFG_CLOUD_PORT_TIMEOUT);

	if (xMQTTReturn != eMQTTAgentSuccess)
	{
		*pulBytesSent = 0;
		return -1;
	}

	*pulBytesSent = ulSize;

	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */
