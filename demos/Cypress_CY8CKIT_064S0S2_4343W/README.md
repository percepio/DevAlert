# Introduction
[Percepio DevAlert](https://percepio.com/devalert/) is a solution for IoT device monitoring and remote diagnostics, allowing for continuous improvement of the device software, even after deployment. DevAlert provides an easy way for uploading "Alerts" from the device software on runtime issues, resulting in notifications to the developers. The alerts include event traces of the device software, allowing for remote diagnostics. 

DevAlert is intended for use in deployed operation, which facilitates catching real-world problems like missed bugs. With DevAlert you can also learn how your software performs with respect to user experience and key performance metrics for your product. DevAlert can also be used during product testing to collect diagnostics on encountered issues in a systematic way, providing better feedback from testers to developers.

This is a demo of Percepio DevAlert on the [Cypress PSoC® 64 Secure AWS IoT Pioneer Kit](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit). The demo is based on the Cypress/AWS demo for ModusToolbox provided with this kit, with only small changes in order to integrate the DevAlert Firmware Monitor and some simulated bugs to demonstrate the solution.

Note that DevAlert can easily be added to any firmware project based on FreeRTOS, assuming you have AWS IoT Core connectivity via MQTT. Support for other RTOSes can be provided on request. A secure and FreeRTOS-qualified processor like PSoC 64 is however recommended.

##  Prerequisites

In general, Percepio DevAlert requires the following:

1. A device and development project using FreeRTOS and AWS IoT Core
2. An AWS account to upload the Alerts. 
3. The DevAlert evaluation package

For this demo, you will need [Cypress PSoC® 64 Secure AWS IoT Pioneer Kit](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit) and the latest version of [ModusToolbox](https://www.cypress.com/products/modustoolbox-software-environment).

Then apply for the DevAlert evaluation package at https://percepio.com/evaluate-devalert/. Applications are subject to manual review, so it might take a few hours or perhaps a working day for us to respond, depending on your time zone.

The evaluation package includes:
- Access to the DevAlert cloud service in your own AWS account
- Percepio Tracealyzer with a DevAlert evaluation license
- Setup instructions and a CloudFormation script for setting up AWS
- Technical support from Percepio

The evaulation package is limited to 90 days and 10 devices. 

The Tracealyzer software runs on Windows and Linux.
 
## Quick start for PSoC 64

Begin by checking the prerequisites above and apply for the DevAlert evaluation package from Percepio.

The PSoC 64 demo project is built and configured in the same way as the original Cypress/AWS demo. We have only made small changes to include the DevAlert Firmware Monitor and suitable demo alerts. Follow the [official getting started guide at the AWS website]( https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_cypress_psoc64.html), using this ModusToolbox project instead of the original one. The demo should then be ready to run. 

The USB connection used for programming and debugging also includes a serial connection that provides some diagnostic messages. Make sure you have a serial terminal program connected to the right COM port (usually the last one in the list). For Windows users, Teraterm is recommended.

Program the board with the demo application and restart it using the SW1/XRES button. You should see a lot of messages when the board starts up, which takes about 30 seconds. When the board has acquired an IP address and connected to AWS, the following message should appear in the terminal:

"[iot_thread] Press BTN0 or swipe left to generate Alerts." 

After that, the CapSense buttons and slider can be used to generate alerts to DevAlert.

To view the reported alerts, see "Tracealyzer" below.

To see how to configure Tracealyzer go to: https://percepio.com/evaluate-devalert-cypress-cy8ckit-064/

NOTE: This project uses git submodules so they need to be initialized befored the project can be build. If the project is downloaded as a .zip file the submodules will not be included.
NOTE1: On Windows it can be a problem with the search path if it's to long. So the project should be placed as closed as C:\ as possible.
 
# Percepio DevAlert

Percepio DevAlert is a solution for alerting developers about errors or other issues in their deployed IoT devices, e.g. during field testing or customer operation. The alerts include a software trace for remote diagnostics of the reported issues.

DevAlert is a hybrid cloud/desktop/device solution including the following parts:
- DevAlert Firmware Monitor (DFM) - a C library for integation on the IoT device
- The DevAlert cloud service, for data processing and notifications to developers
- Percepio Tracealyzer, a desktop application for visual trace diagnostics

Central concepts in DevAlert are:

- Alert: an individual report uploaded by the device. Contains the alert signature and a software trace.
- Alert signature: The data provided to the central DevAlert cloud service. This includes:
-- Alert type: describes the overall type, for instance "Out of memory", "Assert failed", "Stack overflow" etc. 
-- Symptoms: additional details about an alert, e.g. the location in the program code.
- Issue: All alerts with the same alert type and symptoms.

The DevAlert Firmware Monitor (DFM) only uploads data when an Alert is generated by the device software, by calling the DFM API. Alerts are uploaded using the existing IoT connection, in the same way as regular application data, and DFM is not using the connectivity in any other way.

After an upload, alerts are processed and stored in the cloud, in order to group the individual alerts into unique issues. Whenever a new issue is found, notifications are sent to the provided email addresses.

All issues and alerts can be viewed in the DevAlert dashboard, found in the Tracealyzer application. A web-based dashboard is under development and should be available during Q4 2020. 

The cloud part of DevAlert is divided into two domains (different AWS accounts):
- The customer account: the regular AWS account used by the device, responsible for device-to-cloud connectivity and for storing the uploaded Alerts. 
- The DevAlert cloud service: hosted by Percepio and allows for classification and relevant notifications. This is a fully managed service which reduces complexity in the customer account.

A CloudFormation script that sets up AWS in the customer account is provided as part of the evaluation package. The script also establishes a connection to the DevAlert cloud service, which needs certain information about each alert to allow for classification and notifications. This data transfer is completely transparent and only includes the alert signature with information like the device name, firmware version, and numerical codes for the alert type and symptoms. As mentioned, the alerts also include a trace of recent software events, but this remains on the customer side at all times and is not processed by the DevAlert cloud service.

## DevAlert Firmware Monitor (DFM)

The device side of DevAlert is called the DevAlert Firmware Monitor (DFM). DFM is provided in C source code and provides an API for generating Alerts. DFM has one major subcomponent, the TraceRecorder library, which stores software events from the kernel and application logging to a ring-buffer in RAM. When an Alert is generated, you may either upload the Alert data to the cloud directly or store the it to non volatile memory (e.g. MCU flash). The latter is valuable if the board needs to be restarted before an upload is possible (e.g. in case of hard faults); DFM will then upload the Alert when it is initialized again after restart.
 
DFM Library file structure:

 - percepio_dfm.c/h: Public API for initializing DFM and generating Alerts.
 - percepio_dfmConfig.h: Configuration options for the DFM Library.
 - dfmKernelPort.c/h: Interface towards the kernel; can be modified to fit another RTOS kernel.
 - dfmCloudPort.c/h: Defines how to upload data to the cloud.
 - dfmCloudPortConfig.h: Configuration options for the cloud port.
 - dfmHardwarePort.c/h: Defines how to store data in non-volatile memory; can be replaced to work with other MCUs or storage methods.
 - dfmCodes.h: Defines the Alert codes and Symptom codes. This header file can be generated from the DevAlert cloud service.
 
You can define your own alert types and symptoms to match your needs, but you should generally not edit dfmCodes.h directly. The DevAlert cloud service needs to be aware of your definitions to show the Dashboard correctly, so it provides a form where you can define your alert types and symptoms, and from which you can generate a new dfmCodes.h file. Note that the numeric codes are assigned automatically and are specific for each customer, which provides additional security.

In the PSoC 64 demo project, the DFM library is already integrated. In other cases,  follow these steps to integrate the DFM library in your project: 

 - Make sure your project can record trace data in snapshot mode. See the TraceRecorder section, below.
 - Add the DFM library source code to the project and put the header files where they can be found by the compiler.
 - Review percepio_dfmConfig.h and dfmCloudPortConfig.h and modify if needed to fit your system.
 - Add a call to ulDfmInit(…) after a connection to the cloud has been established.
 - Create an alert using ulDfmAlert(…) and ulDfmAddSymptom(…).
 - Finally call ulDfmSendDataToCloud() or DfmSaveDataToNonVolatileMemory().

See the "Example" section below for more information, and the API documentation in percepio_dfm.h.

## TraceRecorder library
The DevAlert Firmware Monitor includes the Percepio TraceRecorder library in order to provide software traces with the alerts. The trace data may include both RTOS kernel events (e.g. scheduling events and API calls) and custom "user events" logged by the application code. 
The trace data is in a compact binary data format intended for Percepio Tracealyzer, that provides visual trace diagnostics. This is very useful for debugging and for general analysis of the runtime behavior. The traces provides important context about the Alert – what was going on in the software when the problem was detected? The trace may reveal the problem directly, or at least help you replicate the problem in the lab. Note that you may add user events in your application code to log additional information, for instance input values and important state changes. 

The TraceRecorder library has been refined since 2009 and is trusted by hundreds of development teams world wide. It designed to be highly memory efficient.

For DevAlert, the recorder should be configured for snapshot mode, where the trace data is kept in a RAM ring buffer. To integrate the recorder library, follow the getting started guide for FreeRTOS [here](https://percepio.com/gettingstarted-freertos/). 

You can verify that the snapshot trace recording works as intended by using our [Eclipse plugin](https://marketplace.eclipse.org/content/percepio-trace-exporter). It is compatible with ModusToolbox.

## Tracealyzer
Tracealyzer is the desktop application that is used to visualize and analyze trace data collected from the device. For more information about Tracealyzer see [here](https://percepio.com/tracealyzer/). From version 4.4.0 Tracealyzer includes DevAlert support, most notably a DevAlert Dashboard on the welcome page. Note, however, that you need the special license that is included in the DevAlert evaluation package to enable  DevAlert integration.

Once you have received the DevAlert evaluation package, install and start Percepio Tracealyzer.
Enter your license key and start the DevAlert Connection Wizard, which is available on the welcome screen. This requires two configuration files provided with the evaluation package.

Once the DevAlert Connection Wizard has finished, the DevAlert dashboard should load and show a table with your recent alerts. This is updated in real time. Try pushing BTN0, making sure a confirmation appears in the terminal. After about 10 seconds you should see the reported Alert in the DevAlert dashboard. There is one row per issue and the row colors indicates the time since the last alert.

As of September 2020, DevAlert has been tested with FreeRTOS and ThreadX. It should also work with SafeRTOS, Micrium µC/OS-III and VxWorks with minor adjustments, but this has not yet been verified. These and other forms of RTOS support can be provided on request.

# Example
Follow these steps to integrate DevAlert into an existing project. This assumes you are using FreeRTOS and AWS IoT Core.

- Integrate the TraceRecorder as described in this [quick start guide](https://percepio.com/gettingstarted-freertos/). More information is found in the Tracealyzer user manual.
- Verify that the TraceRecorder works as intended in snapshot mode, e.g. using the [Eclipse plugin](https://marketplace.eclipse.org/content/percepio-trace-exporter).
- Add the DFM library source code to the project and put the header files where they can be found by the compiler.
- Review percepio_dfmConfig.h and dfmCloudPortConfig.h and modify if necessary to fit your system. 
- Call ulDfmInit(..) right after the point where the application has connected to the cloud.
- Add calls to generate Alerts at suitable locations, e.g. in existing error handlers. See below for an example. Details are found in percepio_dfm.h.
- Add #include "percepio_dfm.h" in all .c files that use the DFM library. 

Below is an example showing how to generate an alert in case of a failed malloc call (out of heap memory).
```
/* Error handler - Failed to allocate requested memory */
void vApplicationMallocFailedHook()
{
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
```

Important DFM functions:

- `vTracePrintF` adds a user event to the trace, in this case the string "Malloc failed". To learn more about user events, search for user event in the Tracealyzer manual.

- `vTraceStop` stops the trace recorder so no new events are recorded while we generate the alert. The trace buffer should not be modified while reading from it.

- `ulDfmAlert` starts a new Alert and sets the alert type. The alert types are defined in dfmCodes.h. Note that dfmCodes.h needs to match the definitions in the DevAlert cloud service, so it is recommended to generate this header file from the cloud service, using the provided code generation feature.

- `ulDfmAddSymptom` adds symptoms to an alert. Symptoms provide additional information about the alert, for example the location where the error occurred. Symptoms only allow for numerical data (32-bit integers or addresses). Each symptom has a numerical ID, defined in dfmCodes.h, and multiple symptoms can be added to an alert. Like alert types (see above), symptoms should normally be defined through the cloud service.

- `ulDfmSendDataToCloud` sends the alert to the cloud. If the error cannot be recovered and the system needs to be restarted before an upload is possible, use `ulDfmSaveDataToNonVolatileMemory` instead. This function saves the data to non-volatile memory; the data is then uploaded to the cloud after the system has restarted and reconnected to the cloud.

- `vTraceClear` clears any old trace data.

- `uiTraceStart` starts the trace recorder again.

To see more examples in the demo code, use the built-in `tasks` view in ModusToolbox (Window -> Show view -> Tasks) and look for the tag `FIXME DevAlert example`. 

# Learning more
If you have questions about this demo, DevAlert, or Tracealyzer, feel free to contact support@percepio.com.
For licensing information, please contact sales@percepio.com.
