/*
 * This is a demo created by Percepio AB to show Percepios DevAlert feature
 * working on a Cypress PSOC64 board. The demo is based on the Amazon FreeRTOS
 * demo then changed to work with DevAler.
 *
 * Amazon FreeRTOS V1.4.7
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 * The DevAlert cloud service and the Tracealyzer desktop application are 
 * provided under separate terms.
 */

/* FIX ME: If you are using Ethernet network connections and the FreeRTOS+TCP stack,
 * uncomment the define:
 * #define CY_USE_FREERTOS_TCP 1
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "stdint.h"

#ifdef CY_BOOT_USE_EXTERNAL_FLASH
#include "flash_qspi.h"
#include "cy_smif_psoc6.h"
#endif

#ifdef CY_USE_LWIP
#include "lwip/tcpip.h"
#endif

/* Demo includes */
#include "aws_demo.h"
#include "led_task.h"
#include "capsense_task.h"
#include "cycfg_capsense.h"
#include "demo_settings_flash_storage.h"
#include "cy_em_eeprom.h"
#include "cy_utils.h"
#include "cy_mutex_pool.h"
#include "percepio_dfm.h"


/* AWS library includes. */
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "iot_wifi.h"
#include "aws_clientcredential.h"
#include "aws_application_version.h"
#include "aws_dev_mode_key_provisioning.h"

/* BSP & Abstraction inclues */
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "sysflash.h"

#include "iot_network_manager_private.h"

#if BLE_ENABLED
    #include "bt_hal_manager_adapter_ble.h"
    #include "bt_hal_manager.h"
    #include "bt_hal_gatt_server.h"

    #include "iot_ble.h"
    #include "iot_ble_config.h"
    #include "iot_ble_wifi_provisioning.h"
    #include "iot_ble_numericComparison.h"

    #include "cyhal_uart.h"

    #define INPUT_MSG_ALLOC_SIZE             (50u)
    #define DELAY_BETWEEN_GETC_IN_TICKS      (1500u)
#endif

#ifdef CY_TFM_PSA_SUPPORTED
#include "tfm_multi_core_api.h"
#include "tfm_ns_interface.h"
#include "tfm_ns_mailbox.h"
#endif

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 8 )

/* The task delay for allowing the lower priority logging task to print out Wi-Fi
 * failure status before blocking indefinitely. */
#define mainLOGGING_WIFI_STATUS_DELAY       pdMS_TO_TICKS( 1000 )

/* Unit test defines. */
#define mainTEST_RUNNER_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 16 )

/* The name of the devices for xApplicationDNSQueryHook. */
#define mainDEVICE_NICK_NAME				"cypress_demo" /* FIX ME.*/


/* Static arrays for FreeRTOS-Plus-TCP stack initialization for Ethernet network
 * connections are declared below. If you are using an Ethernet connection on your MCU
 * device it is recommended to use the FreeRTOS+TCP stack. The default values are
 * defined in FreeRTOSConfig.h. */

#ifdef CY_USE_FREERTOS_TCP
/* Default MAC address configuration.  The application creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition for information on how to configure
 * the real network connection to use. */
const uint8_t ucMACAddress[ 6 ] =
{
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

/* The default IP and MAC address used by the application.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
static const uint8_t ucIPAddress[ 4 ] =
{
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};
static const uint8_t ucNetMask[ 4 ] =
{
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};
static const uint8_t ucGatewayAddress[ 4 ] =
{
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};
static const uint8_t ucDNSServerAddress[ 4 ] =
{
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};
#endif /* CY_USE_FREERTOS_TCP */

/* Priorities of user tasks in this project. configMAX_PRIORITIES is defined in
 * the FreeRTOSConfig.h and higher priority numbers denote high priority tasks.
 */
#define TASK_CAPSENSE_PRIORITY      (4)
#define TASK_LED_PRIORITY           (3)

/* Stack sizes of user tasks in this project */
#define TASK_CAPSENSE_STACK_SIZE    (2 * configMINIMAL_STACK_SIZE)
#define TASK_LED_STACK_SIZE         (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE        (1u)

/* EEPROM Configuration details. All the sizes mentioned are in bytes.
 * For details on how to configure these values refer to cy_em_eeprom.h. The
 * library documentation is provided in Em EEPROM API Reference Manual. The user
 * can access it from the project explorer at Application_name-> libs->
 * emeeprom-> docs.
 */

/* Logical Size of Emulated EEPROM in bytes. */
#define LOGICAL_EEPROM_SIZE     (8192u)
#define LOGICAL_EEPROM_START    (0u)

/* Location of reset counter in Em_EEPROM. */
#define RESET_COUNT_LOCATION    (13u)
/* Size of reset counter in bytes. */
#define RESET_COUNT_SIZE        (2u)

#define Em_EEPROM_PHYSICAL_SIZE (8192u)
#define EEPROM_SIZE             (8192u)
#define BLOCKING_WRITE          (1u)
#define REDUNDANT_COPY          (1u)
#define WEAR_LEVELLING_FACTOR    (2u)

/* Set the macro FLASH_REGION_TO_USE to either USER_FLASH or
 * EMULATED_EEPROM_FLASH to specify the region of the flash used for
 * emulated EEPROM.
 */

#define USER_FLASH              (0u)
#define EMULATED_EEPROM_FLASH   (1u)
#define FLASH_REGION_TO_USE     EMULATED_EEPROM_FLASH

#define GPIO_LOW                (0u)
#define STATUS_SUCCESS          (0u)

/**
 * @brief Application task startup hook for applications using Wi-Fi. If you are not
 * using Wi-Fi, then start network dependent applications in the vApplicationIPNetorkEventHook
 * function. If you are not using Wi-Fi, this hook can be disabled by setting
 * configUSE_DAEMON_TASK_STARTUP_HOOK to 0.
 */
void vApplicationDaemonTaskStartupHook( void );

/**
 * @brief Connects to Wi-Fi.
 */
static void prvWifiConnect( void );

/**
 * @brief Initializes the board.
 */
static void prvMiscInitialization( void );

static void DFMTask(void *pvParameters);

void demo_issue_1(int* ptr);
void demo_issue_2(int size);
void demo_issue_3(int a, int b, int c);

void cy_toolchain_init(void);

/*-----------------------------------------------------------*/
cy_stc_eeprom_config_t Em_EEPROM_config = { .eepromSize = EEPROM_SIZE,
	.blockingWrite = BLOCKING_WRITE, .redundantCopy = REDUNDANT_COPY,
	.wearLevelingFactor = WEAR_LEVELLING_FACTOR, .simpleMode = 1, };

cy_stc_eeprom_context_t Em_EEPROM_context;

#if (FLASH_REGION_TO_USE)
CY_SECTION("SETTING_LOC")
#endif /* #if(FLASH_REGION_TO_USE) */
CY_ALIGN(CY_EM_EEPROM_FLASH_SIZEOF_ROW)

	/* EEPROM storage in user flash or emulated EEPROM flash. */
	ClientCredentials_t flashClientCredentials = { 0 };


// Copy in RAM
ClientCredentials_t tmpClientCredentials = { 0 };

// Memory allocated when these are changed. Otherwise the real ones are found in flash by mbedTLS.
uint8_t* tmpCertificate = NULL;
uint8_t* tmpPrivateKey = NULL;

RecorderDataType xRecorderData __attribute__ ((aligned (8)));


#ifdef CY_TFM_PSA_SUPPORTED
static struct ns_mailbox_queue_t ns_mailbox_queue;

static void tfm_ns_multi_core_boot(void)
{
    int32_t ret;

    printf("Non-secure code running on non-secure core.\r\n");

    if (tfm_ns_wait_for_s_cpu_ready()) {
        printf("Error sync'ing with secure core.\r\n");

        /* Avoid undefined behavior after multi-core sync-up failed */
        for (;;) {
        }
    }

    ret = tfm_ns_mailbox_init(&ns_mailbox_queue);
    if (ret != MAILBOX_SUCCESS) {
        printf("Non-secure mailbox initialization failed.\r\n");

        /* Avoid undefined behavior after NS mailbox initialization failed */
        for (;;) {
        }
    }
}
#endif

/*-----------------------------------------------------------*/

traceString devalert_user_event_channel = NULL;

/**
 * @brief Application runtime entry point.
 */
int main( void )
{
    /* Perform any hardware initialization that does not require the RTOS to be
     * running.  */
    prvMiscInitialization();

	/* Enable global interrupts */
	__enable_irq();

	//Enable all fault handlers!
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk
		| SCB_SHCSR_MEMFAULTENA_Msk;

	// Enable Usage Faults for DivByZero and Unanligned access. (the latter however occurs "naturally" in this project...)
	SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk; //| SCB_CCR_UNALIGN_TRP_Msk;

#ifdef CY_TFM_PSA_SUPPORTED
    tfm_ns_multi_core_boot();
#endif


    /*** DevAlert Trace Library Initialization *******************************/

    // Specify the RAM buffer location
    vTraceSetRecorderDataBuffer(&xRecorderData);

    // Initialize the trace recorder (don't start it yet)
	vTraceEnable(TRC_INIT);

    // Initialize a "User Event Channel" for DevAlert events in the trace
    devalert_user_event_channel = xTraceRegisterString("DevAlert");

    /*************************************************************************/

    cy_toolchain_init();

    led_command_data_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
    			sizeof(led_command_data_t));
    capsense_command_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
    			sizeof(capsense_command_t));
    bug_command_data_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
    			sizeof(bug_command_data_t));

    /* Create tasks that are not dependent on the Wi-Fi being initialized. */
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            tskIDLE_PRIORITY,
                            mainLOGGING_MESSAGE_QUEUE_LENGTH );
    xTaskCreate(task_led, "Led Task", TASK_LED_STACK_SIZE,
    			NULL, TASK_LED_PRIORITY, NULL);
    xTaskCreate(task_capsense, "CapSense Task", TASK_CAPSENSE_STACK_SIZE,
    			NULL, TASK_CAPSENSE_PRIORITY, NULL);
    xTaskCreate(DFMTask, "TaskWithBugs", 1400, NULL, 4, NULL);

#ifdef CY_TFM_PSA_SUPPORTED
    /* Initialize TFM interface */
    tfm_ns_interface_init();
#endif

#ifdef CY_USE_FREERTOS_TCP
    FreeRTOS_IPInit( ucIPAddress,
                     ucNetMask,
                     ucGatewayAddress,
                     ucDNSServerAddress,
                     ucMACAddress );
#endif /* CY_USE_FREERTOS_TCP */

    /* Start the scheduler.  Initialization that requires the OS to be running,
     * including the Wi-Fi initialization, is performed in the RTOS daemon task
     * startup hook. */
    vTaskStartScheduler();

    return 0;
}
/*-----------------------------------------------------------*/

static void prvMiscInitialization( void )
{
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        printf(  "BSP initialization failed \r\n" );
    }
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        printf( "Retarget IO initialization failed \r\n" );
    }

    Em_EEPROM_config.userFlashStartAddr = (uint32_t) &flashClientCredentials;
	result = Cy_Em_EEPROM_Init(&Em_EEPROM_config, &Em_EEPROM_context);
	if (result != CY_RSLT_SUCCESS) {
		printf("Eeprom initializatoin failed \r\n");
	}
	result = InitDFM_EEPROM();
	if (result != CY_RSLT_SUCCESS) {
		printf("DFM Eeprom initializatoin failed \r\n");
	}

    #if BLE_ENABLED
        NumericComparisonInit();
    #endif

#ifdef CY_BOOT_USE_EXTERNAL_FLASH
    __enable_irq();

#ifdef PDL_CODE
    if (qspi_init_sfdp(1) < 0)
    {
        printf("QSPI Init failed\r\n");
        while (1);
    }
#else   /* PDL_CODE */
    if (psoc6_qspi_init() != 0)
    {
       printf("psoc6_qspi_init() FAILED!!\r\n");
    }
#endif /* PDL_CODE */
#endif /* CY_BOOT_USE_EXTERNAL_FLASH */
}
/*-----------------------------------------------------------*/
void vApplicationDaemonTaskStartupHook( void )
{
    /* FIX ME: Perform any hardware initialization, that require the RTOS to be
     * running, here. */


    /* FIX ME: If your MCU is using Wi-Fi, delete surrounding compiler directives to
     * enable the unit tests and after MQTT, Bufferpool, and Secure Sockets libraries
     * have been imported into the project. If you are not using Wi-Fi, see the
     * vApplicationIPNetworkEventHook function. */
    if( SYSTEM_Init() == pdPASS )
    {
#ifdef CY_USE_LWIP
        /* Initialize lwIP stack. This needs the RTOS to be up since this function will spawn 
         * the tcp_ip thread.
         */
        tcpip_init(NULL, NULL);
#endif
        /* Connect to the Wi-Fi before running the tests. */
        prvWifiConnect();

#if ( pkcs11configVENDOR_DEVICE_CERTIFICATE_SUPPORTED == 0 )
        /* Provision the device with AWS certificate and private key. */
        vDevModeKeyProvisioning();
#endif

        vTraceClearError();

        /* Start the demo tasks. */
        DEMO_RUNNER_RunDemos();
    }
}

/*-----------------------------------------------------------*/

static void DFMTask(void *pvParameters)
{

	/* Suppress warning for unused parameter */
	(void)pvParameters;

	/*counter is declared static so it won't reset after recovering from a fault.*/
	static int counter = 0;

	int* ptr = NULL;
	int size = 3000000;

	BaseType_t rtos_api_result;
	bug_command_data_t bug_cmd_data;

	vTaskDelay(5000);

	configPRINTF(("Demo task ready...\n\r"));

	for (;;)
	{

		/* Block until a command has been received over queue */
		rtos_api_result = xQueueReceive(bug_command_data_q, &bug_cmd_data,
				portMAX_DELAY);

		if (rtos_api_result == pdTRUE) {

			switch(bug_cmd_data.command){

				case BUTTON_PRESSED:
					{
						configPRINTF(("Button pressed!\n\r"));
						if (ulDfmIsDisabled() == 0)
						{
							switch (counter++)
							{
								case 0:
									configPRINTF(("\r\nDevAlert - Testing 'ASSERT' error.\r\n"));
									demo_issue_1(ptr);
									vTaskDelay(1000);
									break;
								case 1:
									configPRINTF(
											("\r\nDevAlert - Testing 'Malloc failed' error.\r\n"));
									demo_issue_2(size);
									vTaskDelay(1000);
									break;
								case 2:
									configPRINTF(
											("\r\nDevAlert - Testing 'Division by zero' error.\r\n"));
									demo_issue_3(42, 4, 2);
									counter = 0;
									vTaskDelay(1000);
									break;

								default:
									counter = 0;
									break;
							}
						} else {
							configPRINTF(("DevAlert is disabled\r\n"));
						}
						break;
					}
				case SWIPE_LEFT:

						// FIXME DevAlert example
						configPRINTF(("\r\nDevAlert - Testing 'Swipe left' error.\r\n"));

						// Adding a "user event" to the trace buffer
						vTracePrintF(devalert_user_event_channel, "Swiped left? Not allowed!\r\n");

						// Pause tracing during upload
						vTraceStop();

						// Compose the Alert
						ulDfmAlert( DFM_ALERT_SWIPE_LEFT );
						ulDfmAddSymptom( DFM_SYMPTOM_PRODUCT, APP_PRODUCT_IDENTIFIER);
						ulDfmAddSymptom( DFM_SYMPTOM_CURRENT_TASK, 	(uint32_t) pcTaskGetName(NULL) );
						ulDfmAddSymptom( DFM_SYMPTOM_STACKPTR, 		(uint32_t) __get_PSP() );

						// Upload the Alert
						ulDfmSendDataToCloud();

						// Resume tracing
						vTraceClear();
						uiTraceStart();

						vTaskDelay(1000);
						break;

				default:
					{
						configPRINTF(( "Invalid command.\r\n" ));
						break;
					}
			}

		}

	}
}
/*-----------------------------------------------------------*/

void prvWifiConnect( void )
{
    WIFINetworkParams_t xNetworkParams;
    WIFIReturnCode_t xWifiStatus;
    uint8_t ucTempIp[4] = { 0 };

    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess )
    {

        configPRINTF( ( "Wi-Fi module initialized. Connecting to AP...\r\n" ) );
    }
    else
    {
        configPRINTF( ( "Wi-Fi module failed to initialize.\r\n" ) );

        /* Delay to allow the lower priority logging task to print the above status.
            * The while loop below will block the above printing. */
        vTaskDelay( mainLOGGING_WIFI_STATUS_DELAY );

        while( 1 )
        {
        }
    }

    /* Setup parameters. */
    xNetworkParams.pcSSID = clientcredentialWIFI_SSID;
    xNetworkParams.ucSSIDLength = sizeof( clientcredentialWIFI_SSID ) - 1;
    xNetworkParams.pcPassword = clientcredentialWIFI_PASSWORD;
    xNetworkParams.ucPasswordLength = sizeof( clientcredentialWIFI_PASSWORD ) - 1;
    xNetworkParams.xSecurity = clientcredentialWIFI_SECURITY;
    xNetworkParams.cChannel = 0;

    xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "Wi-Fi Connected to AP. Creating tasks which use network...\r\n" ) );

        xWifiStatus = WIFI_GetIP( ucTempIp );
        if ( eWiFiSuccess == xWifiStatus )
        {
            configPRINTF( ( "IP Address acquired %d.%d.%d.%d\r\n",
                            ucTempIp[ 0 ], ucTempIp[ 1 ], ucTempIp[ 2 ], ucTempIp[ 3 ] ) );
        }
    }
    else
    {
        /* Connection failed, configure SoftAP. */
        configPRINTF( ( "Wi-Fi failed to connect to AP %s.\r\n", xNetworkParams.pcSSID ) );

        xNetworkParams.pcSSID = wificonfigACCESS_POINT_SSID_PREFIX;
        xNetworkParams.pcPassword = wificonfigACCESS_POINT_PASSKEY;
        xNetworkParams.xSecurity = wificonfigACCESS_POINT_SECURITY;
        xNetworkParams.cChannel = wificonfigACCESS_POINT_CHANNEL;

        configPRINTF( ( "Connect to SoftAP %s using password %s. \r\n",
                        xNetworkParams.pcSSID, xNetworkParams.pcPassword ) );

        while( WIFI_ConfigureAP( &xNetworkParams ) != eWiFiSuccess )
        {
            configPRINTF( ( "Connect to SoftAP %s using password %s and configure Wi-Fi. \r\n",
                            xNetworkParams.pcSSID, xNetworkParams.pcPassword ) );
        }

        configPRINTF( ( "Wi-Fi configuration successful. \r\n" ) );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief User defined Idle task function.
 *
 * @note Do not make any blocking operations in this function.
 */
void vApplicationIdleHook( void )
{
    /* FIX ME. If necessary, update to application idle periodic actions. */


}

void vApplicationTickHook()
{
	Cy_CapSense_IncrementGestureTimestamp(&cy_capsense_context);
}

/*
 * @brief Warn user if pvPortMalloc fails.
 *
 * Called if a call to pvPortMalloc() fails because there is insufficient
 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
 * internally by FreeRTOS API functions that create tasks, queues, software
 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
 *
 */

/* Error handler - Failed to allocate requested memory */
void vApplicationMallocFailedHook()
{
	// FIXME DevAlert example
	// Adding a "user event" to the trace buffer
	vTracePrintF(devalert_user_event_channel, "Malloc failed.\r\n");

	// Pause tracing during upload
	vTraceStop();

	// Compose the Alert
	ulDfmAlert( DFM_ALERT_MALLOC_FAILED );
	ulDfmAddSymptom( DFM_SYMPTOM_PRODUCT, APP_PRODUCT_IDENTIFIER);
	ulDfmAddSymptom( DFM_SYMPTOM_CURRENT_TASK, (uint32_t) pcTaskGetName(NULL));
	ulDfmAddSymptom( DFM_SYMPTOM_STACKPTR, (uint32_t) __get_PSP());

	// Upload the Alert
	ulDfmSendDataToCloud();

	// Resume tracing
	vTraceClear();
	uiTraceStart();

	/* Ordinary error handling follows, restart? */
}

void demo_issue_1(int* ptr)
{
    // Triggers an Alert if ptr is NULL
	configASSERT(ptr != NULL);

	// Just to avoid compiler warnings, since not used...
	*ptr = 0;
}

void demo_issue_2(int size)
{
	int i;

	// Tries to allocate memory. Error handler is called on failure.
	uint8_t* buf = pvPortMalloc(size);

	// Do something with the allocated memory...
	if (buf != NULL)
	{
		for (i = 0; i < size; i++)
			buf[i] = 0;

		vPortFree(buf);
	}

}

void (*function_ptr)(int);
volatile int* ptr;

volatile int dummy;
volatile int divisor = 0;

void demo_issue_3(int a, int b, int c)
{
	/* This might cause a hard fault (division by zero...) */
	dummy = a / (b - 2 * c);
}

/*-----------------------------------------------------------*/
char fmtbuf[80];

#define HF_TEST 0

#define BFSR_BFAR_Msk (1 << 7)
#define BFSR_LSPERR_Msk (1 << 5) //Only present in Cortex-M4F devices.
#define BFSR_STKERR_Msk (1 << 4)
#define BFSR_UNSTKERR_Msk (1 << 3)
#define BFSR_IMPRECISERR_Msk (1 << 2)
#define BFSR_PRECISEERR_Msk (1 << 1)
#define BFSR_IBUSERR_Msk (1 << 0)

#define UFSR_DIVBYZERO_Msk (1 << 9)
#define UFSR_UNALIGNED_Msk (1 << 8)
#define UFSR_NOCP_Msk (1 << 3)
#define UFSR_INVPC_Msk (1 << 2)
#define UFSR_INVSTATE_Msk (1 << 1)
#define UFSR_UNDEFINSTR_Msk (1 << 0)

#define MMFSR_MMARVALID_Msk (1 << 7)
#define MMFSR_MLSPERR_Msk (1 << 5) //Only available if a FPU is available.
#define MMFSR_MSTERR_Msk (1 << 4)
#define MMFSR_MUNSTKERR_Msk (1 << 3)
#define MMFSR_DACCVIOL_Msk (1 << 1)
#define MMFSR_IACCVIOL_Msk (1 << 0)

typedef struct __attribute__((packed)) ContextStateFrame {
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r12;
	unsigned int lr;
	unsigned int pc;
	unsigned int psr;
} sContextStateFrame;

#define FAULT_HANDLING_ASM(_x)               \
	__asm volatile(                                \
			"tst lr, #4 \n"                            \
			"ite eq \n"                                \
			"mrseq r0, msp \n"                         \
			"mrsne r0, psp \n"                         \
			"b UsageFault_Handler2 \n"                  \
			)

void UsageFault_Handler2(sContextStateFrame *frame) {
	uint32_t _CFSR;
	uint16_t _UFSR;
	uint16_t _BFSR;

	// If the system should recover from the error or restart, 0 for restart.
	uint8_t recover_error = 0;
	uint8_t do_not_send_to_cloud = 0;

	// The address to the CFSR register where all faults are stored.
	volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;

	// Load all the different parts of the CFSR register.
	_CFSR = SCB->CFSR;
	_UFSR = (_CFSR >> 16) & 0xffff;
	_BFSR = (_CFSR >> 8);

	// FIXME DevAlert example
	// Stops the recording and checks the error type and adds right symptoms.
	if(_UFSR != 0){

		// A divide by zero instruction was executed. This fault must be enabled to trigger.
		if (_UFSR & UFSR_DIVBYZERO_Msk)
		{
			ulDfmAlert(DMF_ALERT_UFSR_DIVBYZERO);

			snprintf(fmtbuf, sizeof(fmtbuf), "Division by Zero fault"); /* Add details here */
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

		// Indicates that an unaligned access operation occured.
		if (_UFSR & UFSR_UNALIGNED_Msk)
		{
			ulDfmAlert(DMF_ALERT_UFSR_UNALIGNED);

			snprintf(fmtbuf, sizeof(fmtbuf), "Unaligned access fault"); /* Add details here */
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

		// Coprocessor instruction issued with no coprocessor present or enabled.
		if(_UFSR & UFSR_NOCP_Msk)
		{
			ulDfmAlert((uint32_t) DMF_ALERT_UFSR_NOCP);

			snprintf(fmtbuf, sizeof(fmtbuf), "Coprocessor instruction issued with no coprocessor present or enabled.");
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

		// Integrity check failure on EXC_RETURN from exception.
		if(_UFSR & UFSR_INVPC_Msk)
		{
			ulDfmAlert((uint32_t) DMF_ALERT_UFSR_INVPC);

			snprintf(fmtbuf, sizeof(fmtbuf), "Integrity check failure on EXC_RETURN from exception.");
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

		// Tried to execute instruction with an invalid EPSR value.
		if(_UFSR & UFSR_INVSTATE_Msk)
		{
			ulDfmAlert((uint32_t) DMF_ALERT_UFSR_INVSTATE);

			snprintf(fmtbuf, sizeof(fmtbuf), "Tried to execute instruction with an invalid EPSR value.");
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

		// Tried to execute undefined instruction, may be caused by a corrupt stack.
		if (_UFSR & UFSR_UNDEFINSTR_Msk)
		{
			ulDfmAlert((uint32_t) DMF_ALERT_UFSR_UNDEFINSTR);

			snprintf(fmtbuf, sizeof(fmtbuf), "Tried to execute undefined instruction, may be caused by a corrupt stack.");
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
		}

	}

	if(_BFSR != 0){

		// The BFAR hold the address which triggered the fault.
		if(_BFSR & BFSR_BFAR_Msk)
		{
			ulDfmAlert(DFM_ALERT_BFSR_BFAR);

			snprintf(fmtbuf, sizeof(fmtbuf), "\r\n BFAR bit set, fault occured at address: 0x%08X", (uint)SCB->BFAR);
			vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
			recover_error = 1;
		}

		// Error on the data bus. If this error occurs the hardwaare wasn't able to determine the exact location of the fault.
		if(_BFSR & BFSR_IMPRECISERR_Msk){
			recover_error = 1;
			//Don't send the trace to the cloud since this fault is triggered by the ASSERT demo.
			do_not_send_to_cloud = 1;
		}

	}

	// print the values of PC and registers to trace.
	snprintf(fmtbuf, sizeof(fmtbuf), "Value of PC: 0x%08X\r\n",
			frame->pc);
	//vMainUARTPrintString(fmtbuf);
	vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
	snprintf(fmtbuf, sizeof(fmtbuf),
			"Value of R0: 0x%08X, R1: 0x%08X, R2: 0x%08X, R3: 0x%08X\r\n",
			frame->r0, frame->r1, frame->r2, frame->r3);
	//vMainUARTPrintString(fmtbuf);
	vTracePrintF(devalert_user_event_channel, "%s", fmtbuf);
	vTraceStop();

	//ulDfmAlert(DFM_ALERT_HARDFAULT);
	ulDfmAddSymptom(DFM_SYMPTOM_PRODUCT, APP_PRODUCT_IDENTIFIER);
	ulDfmAddSymptom(DFM_SYMPTOM_ARM_SCB_FCSR, SCB->CFSR);
	ulDfmAddSymptom(DFM_SYMPTOM_PC, frame->pc);



	if(recover_error == 0){
		ulDfmSaveDataToNonVolatileMemory();
		NVIC_SystemReset();
	}
	else{
		if(do_not_send_to_cloud == 0){
			ulDfmSendDataToCloud();
		}

		vTraceClear();
		uiTraceStart();

		//Clear CFSR register
		*cfsr |= *cfsr;

		// The instruction we will return to when we exit from the exception
		frame->pc = (uint32_t)DFMTask;

		// The function we are returning to should never branch
		// so set lr to a pattern that would fault if it did
		frame->lr = 0xdeadbeef;

		// Reset the psr state and only leave the
		// "thumb instruction interworking" bit set
		frame->psr = (1 << 24);
	}


}

void UsageFault_Handler(void) {
	FAULT_HANDLING_ASM();
}

void HardFault_Handler(void) {
	FAULT_HANDLING_ASM();
	// TODO: If the fault hadlers fail it will end up in hardfault, save trace and reset.

}

void MemManage_Handler(void) {
	FAULT_HANDLING_ASM();
}

void BusFault_Handler(void) {
	// Ignore busfault, otherwise assert demo will cause busfault.
	FAULT_HANDLING_ASM();
}

/**
* @brief User defined application hook to process names returned by the DNS server.
*/
#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )
    BaseType_t xApplicationDNSQueryHook( const char * pcName )
    {
        /* FIX ME. If necessary, update to applicable DNS name lookup actions. */

        BaseType_t xReturn;

        /* Determine if a name lookup is for this node.  Two names are given
         * to this node: that returned by pcApplicationHostnameHook() and that set
         * by mainDEVICE_NICK_NAME. */
        if( strcmp( pcName, pcApplicationHostnameHook() ) == 0 )
        {
            xReturn = pdPASS;
        }
        else if( strcmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }

        return xReturn;
    }

#endif /* if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) */
/*-----------------------------------------------------------*/

/**
 * @brief User defined assertion call. This function is plugged into configASSERT.
 * See FreeRTOSConfig.h to define configASSERT to something different.
 */
//void vAssertCalled(const char * pcFile,
//	uint32_t ulLine)
//{
//    /* FIX ME. If necessary, update to applicable assertion routine actions. */
//
//	const uint32_t ulLongSleep = 1000UL;
//	volatile uint32_t ulBlockVariable = 0UL;
//	volatile char * pcFileName = (volatile char *)pcFile;
//	volatile uint32_t ulLineNumber = ulLine;
//
//	(void)pcFileName;
//	(void)ulLineNumber;
//
//	printf("vAssertCalled %s, %ld\n", pcFile, (long)ulLine);
//	fflush(stdout);
//
//	/* Setting ulBlockVariable to a non-zero value in the debugger will allow
//	* this function to be exited. */
//	taskDISABLE_INTERRUPTS();
//	{
//		while (ulBlockVariable == 0UL)
//		{
//			vTaskDelay( pdMS_TO_TICKS( ulLongSleep ) );
//		}
//	}
//	taskENABLE_INTERRUPTS();
//}
/*-----------------------------------------------------------*/

/**
 * @brief User defined application hook need by the FreeRTOS-Plus-TCP library.
 */
#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
    const char * pcApplicationHostnameHook(void)
    {
        /* FIX ME: If necessary, update to applicable registration name. */

        /* This function will be called during the DHCP: the machine will be registered
         * with an IP address plus this name. */
        return clientcredentialIOT_THING_NAME;
    }

#endif

#if BLE_ENABLED
    /**
     * @brief "Function to receive user input from a UART terminal. This function reads until a line feed or 
     * carriage return character is received and returns a null terminated string through a pointer to INPUTMessage_t.
     * 
     * @note The line feed and carriage return characters are removed from the returned string.
     * 
     * @param pxINPUTmessage Message structure using which the user input and the message size are returned.
     * @param xAuthTimeout Time in ticks to be waited for the user input.
     * @returns pdTrue if the user input was successfully captured, else pdFalse.
     */
    BaseType_t getUserMessage( INPUTMessage_t * pxINPUTmessage, TickType_t xAuthTimeout )
    {
        BaseType_t xReturnMessage = pdFALSE;
        TickType_t xTimeOnEntering;
        uint8_t *ptr;
        uint32_t numBytes = 0;
        uint8_t msgLength = 0;

        /* Dynamically allocate memory to store user input. */
        pxINPUTmessage->pcData = ( uint8_t * ) pvPortMalloc( sizeof( uint8_t ) * INPUT_MSG_ALLOC_SIZE ); 

        /* ptr points to the memory location where the next character is to be stored. */
        ptr = pxINPUTmessage->pcData;   

        /* Store the current tick value to implement a timeout. */
        xTimeOnEntering = xTaskGetTickCount();

        do
        {
            /* Check for data in the UART buffer with zero timeout. */
            numBytes = cyhal_uart_readable(&cy_retarget_io_uart_obj);
            if (numBytes > 0)
            {
                /* Get a single character from UART buffer. */
                cyhal_uart_getc(&cy_retarget_io_uart_obj, ptr, 0);

                /* Stop checking for more characters when line feed or carriage return is received. */
                if((*ptr == '\n') || (*ptr == '\r'))
                {
                    *ptr = '\0';
                    xReturnMessage = pdTRUE;
                    break;
                }

                ptr++;
                msgLength++;

                /* Check if the allocated buffer for user input storage is full. */
                if (msgLength >= INPUT_MSG_ALLOC_SIZE) 
                {
                    break;
                }
            }

            /* Yield to other tasks while waiting for user data. */
            vTaskDelay( DELAY_BETWEEN_GETC_IN_TICKS );

        } while ((xTaskGetTickCount() - xTimeOnEntering) < xAuthTimeout); /* Wait for user data until timeout period is elapsed. */

        if (xReturnMessage == pdTRUE)
        {
            pxINPUTmessage->xDataSize = msgLength;
        }
        else if (msgLength >= INPUT_MSG_ALLOC_SIZE)
        {
            configPRINTF( ( "User input exceeds buffer size !!\n" ) );
        }
        else
        {
            configPRINTF( ( "Timeout period elapsed !!\n" ) );
        }       

        return xReturnMessage;
    }

#endif /* if BLE_ENABLED */  
