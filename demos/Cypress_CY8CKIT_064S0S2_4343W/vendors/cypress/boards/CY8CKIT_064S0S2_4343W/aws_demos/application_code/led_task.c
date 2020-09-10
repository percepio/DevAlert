/******************************************************************************
* File Name: led_task.c
*
* Description: This file contains the task that handles led.
*
* Related Document: README.md
*
********************************************************************************
* Copyright (2019), Cypress Semiconductor Corporation.
********************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*****************************************​**************************************/


/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "led_task.h"
#include "cybsp.h"
#include "cyhal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cycfg.h"


/*******************************************************************************
* Global constants
*******************************************************************************/
#define PWM_LED_FREQ_HZ     (1000000u)  /* in Hz */
#define GET_DUTY_CYCLE(x)   (100 - x)   /* subtracting from 100 since the LED 
                                         * is connected in active low 
                                         * configuration
                                         */

/*******************************************************************************
 * Global variable
 ******************************************************************************/
/* Queue handle used for LED data */
QueueHandle_t led_command_data_q;


/*******************************************************************************
* Function Name: task_led
********************************************************************************
* Summary:
*  Task that controls the LED.
*
* Parameters:
*  void *param : Task parameter defined during task creation (unused)
*
*******************************************************************************/
void task_led(void* param)
{
    cyhal_pwm_t pwm_led;
    bool led_on = true;
    BaseType_t rtos_api_result;
    led_command_data_t led_cmd_data;

    /* Suppress warning for unused parameter */
    (void)param;

    /* Configure the TCPWM for driving led */
    cyhal_pwm_init(&pwm_led, CYBSP_USER_LED, NULL);

    cyhal_pwm_start(&pwm_led);

    /* set to partial brightness initially */
    cyhal_pwm_set_duty_cycle(&pwm_led, GET_DUTY_CYCLE(LED_MIN_BRIGHTNESS),
                             PWM_LED_FREQ_HZ);

    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over queue */
        rtos_api_result = xQueueReceive(led_command_data_q, &led_cmd_data,
                            portMAX_DELAY);

        /* Command has been received from queue */
        if(rtos_api_result == pdTRUE)
        {
            switch(led_cmd_data.command)
            {
                /* Toggle the LED. */
                case LED_TOGGLE:
                {
                    if (!led_on)
                    {
                        /* Start PWM to turn the LED on */
                        cyhal_pwm_start(&pwm_led);
                        led_on = true;
                    }
                    else
                    {
                        /* Stop PWM to turn the LED off */
                    	cyhal_pwm_stop(&pwm_led);
                    	led_on = false;
                    }
                    break;
                }

                /* Turn off the LED. */
                case LED_OFF:
                {
                    if (led_on)
                    {
                        /* Stop PWM to turn the LED off */
                    	cyhal_pwm_stop(&pwm_led);
                    	led_on = false;
                    }
                    break;
                }

                /* Turn on the LED. */
                case LED_ON:
                {
                    if (!led_on)
                    {
                        /* Start PWM to turn the LED on */
                        cyhal_pwm_start(&pwm_led);
                        led_on = true;
                    }
                    break;
                }

                /* Update LED brightness */
                case LED_UPDATE_BRIGHTNESS:
                {
                    if (led_on)
                    {
                        uint32_t brightness = (led_cmd_data.brightness < LED_MIN_BRIGHTNESS) ?
                                               LED_MIN_BRIGHTNESS : led_cmd_data.brightness;

                        /* Drive the LED with brightness */
                        cyhal_pwm_set_duty_cycle(&pwm_led, GET_DUTY_CYCLE(brightness),
                                                 PWM_LED_FREQ_HZ);
                    }
                    break;
                }

                /* Invalid command */
                default:
                {
                    /* Handle invalid command here */
                    break;
                }
            }
        }

        /* Task has timed out and received no data during an interval of
         * portMAXDELAY ticks.
         */
        else
        {
            /* Handle timeout here */
        }
    }
}


/* END OF FILE [] */
