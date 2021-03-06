
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

#if LEDS_NUMBER <= 2
#error "Board is not equipped with enough amount of LEDs"
#endif

#define TASK_DELAY        200           /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD      1000          /**< Timer period. LED1 timer will expire after 1000 ms */

TaskHandle_t  led_toggle_task_handle;   /**< Reference to LED0 toggling FreeRTOS task. */
TimerHandle_t led_toggle_timer_handle;  /**< Reference to LED1 toggling FreeRTOS timer. */


const uint32_t *red_led = (uint32_t*)BSP_BOARD_LED_1;
const uint32_t *blue_led = (uint32_t*)BSP_BOARD_LED_2;
const uint32_t *green_led = (uint32_t*)BSP_BOARD_LED_3;
/**@brief LED0 task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
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

void vLedControllerTask(void *vpParameters)
{
//UNUSED_PARAMETER(vpParameters);
  while(1)
  {
      bsp_board_led_invert((uint32_t)vpParameters);
      vTaskDelay(500);
  }
}

int main(void)
{
    ret_code_t err_code;

    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    /* Configure LED-pins as outputs */
    bsp_board_init(BSP_INIT_LEDS);

    /* Create task for LED0 blinking with priority set to 2 */
    UNUSED_VARIABLE(xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_toggle_task_handle));
    
    UNUSED_VARIABLE(xTaskCreate(vLedControllerTask,
                                "Blue Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                red_led,
                                2,
                                NULL));
    UNUSED_VARIABLE(xTaskCreate(vLedControllerTask,
                                "Red Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                green_led,
                                2,
                                NULL));
    UNUSED_VARIABLE(xTaskCreate(vLedControllerTask,
                                "Green Led Task",
                                configMINIMAL_STACK_SIZE + 200,
                                blue_led,
                                2,
                                NULL));
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
