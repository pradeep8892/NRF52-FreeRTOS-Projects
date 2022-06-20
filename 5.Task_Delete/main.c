/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example_freertos
 *
 * @brief Blinky FreeRTOS Example Application main file.
 *
 * This file contains the source code for a sample application using FreeRTOS to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"
#include "SEGGER_RTT/SEGGER_RTT.h"

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define TASK_DELAY        200           /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD      1000          /**< Timer period. LED1 timer will expire after 1000 ms */

TaskHandle_t  led_toggle_task_handle;   /**< Reference to LED0 toggling FreeRTOS task. */
TimerHandle_t led_toggle_timer_handle;  /**< Reference to LED1 toggling FreeRTOS timer. */

TaskHandle_t red_led_taskhandle,green_led_taskhandle,blue_led_taskhandle;

/**@brief LED0 task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */

int delete_monitor = 0, blue_delete_monitor = 0;
bool is_task_suspended = false;
static void led_toggle_task_function (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    while (true)
    {
        bsp_board_led_invert(BSP_BOARD_LED_0);

        /* Delay a task for a given number of ticks */
        vTaskDelay(TASK_DELAY);

        /* Tasks must be implemented to never return... */
    }
}

void vBlueLedControllerTask(void *vpParameters)
{
  UNUSED_PARAMETER(vpParameters);
  while(1)
  {
    bsp_board_led_invert(BSP_BOARD_LED_1);
    //printf("Blue Led Task");
    int a = 0;
    SEGGER_RTT_printf(0,"\nBLue Led Task ");
    delete_monitor++;
    if(delete_monitor>=20 && !is_task_suspended){
      SEGGER_RTT_printf(0,"\nGreen Led Task Deleted/Killed");
      is_task_suspended = true;
     
      vTaskDelete(green_led_taskhandle);
    }
    vTaskDelay(500);
  }
}


void vGreenLedControllerTask(void *vpParameters)
{
UNUSED_PARAMETER(vpParameters);
  while(1)
  {
    bsp_board_led_invert(BSP_BOARD_LED_2);
    SEGGER_RTT_printf(0,"\nGreen Led Task ");
    vTaskDelay(500);
    //if(is_task_suspended)
    //{
    //  resume_monitor++;
    //  if(resume_monitor>=10){
    //    SEGGER_RTT_printf(0,"\nBlue Led Task Resumed ");
    //    vTaskResume(blue_led_taskhandle);
    //    is_task_suspended = false;
    //    suspend_monitor = 0;
    //  }
    //}
   
  }
}
int red_suspend_monitor = 0;
void vRedLedControllerTask(void *vpParameters)
{
UNUSED_PARAMETER(vpParameters);
  while(1)
  {
      bsp_board_led_invert(BSP_BOARD_LED_3);
      SEGGER_RTT_printf(0,"\nRed Led Task ");
      vTaskDelay(500);
      red_suspend_monitor++;
      if(red_suspend_monitor>=25)
      {
      red_suspend_monitor = 0;
        SEGGER_RTT_printf(0,"\nBlue Led Task Deleted/killed");
        vTaskDelete(blue_led_taskhandle);
      }
  }
}
/**@brief The function to call when the LED1 FreeRTOS timer expires.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the timer.
 */
static void led_toggle_timer_callback (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    bsp_board_led_invert(BSP_BOARD_LED_1);
}

int main(void)
{
    ret_code_t err_code;
    SEGGER_RTT_Init();
    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    /* Configure LED-pins as outputs */
    bsp_board_init(BSP_INIT_LEDS);

    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_toggle_task_handle));
    
    UNUSED_VARIABLE(xTaskCreate(vBlueLedControllerTask,
                                "Blue Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                NULL,
                                2,
                                &blue_led_taskhandle));
    UNUSED_VARIABLE(xTaskCreate(vRedLedControllerTask,
                                "Red Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                NULL,
                                3,
                                &red_led_taskhandle));
    UNUSED_VARIABLE(xTaskCreate(vGreenLedControllerTask,
                                "Green Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                NULL,
                                1,
                                green_led_taskhandle));
    /* Start timer for LED1 blinking */
    //led_toggle_timer_handle = xTimerCreate( "LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    //UNUSED_VARIABLE(xTimerStart(led_toggle_timer_handle, 0));

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    while (true)
    {
        /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
         * in vTaskStartScheduler function. */
    }
}

/**
 *@}
 **/