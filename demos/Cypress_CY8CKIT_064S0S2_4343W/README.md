This is a demo created by Percepio AB to show Percepios DevAlert feature
working on a Cypress PSOC64 board. The demo is based on the Amazon FreeRTOS
demo then changed to work with DevAlert.
 
The demo is built and configured in the same way as the Amazon FreeRTOS demo, the instructions to set up the board and application can be found here: https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_cypress_psoc64.html
 
When the board is programmed and connected to a serial terminal the text "[iot_thread] Press BTN0 or swipe left to generate Alerts." will show if the board has started as it should (the boot process will take about 30 seconds). After that the CapSense buttons and slider can be used to generate alerst to DevAlert that will show in Tracealyzer if it is configured. 
 
In the project the recorder from Percepio is located, to learn more about how to use the recorder and change the settings take a look at the getting started guide: https://percepio.com/gettingstarted-freertos/
 
In the DFM folder the DevAlert part of the project is located this is the part that makes it possible to share the traces to the cloud. 
