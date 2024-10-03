/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_CAD 20000
/*==================[internal data definition]===============================*/
TaskHandle_t cad_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
void AnalogInputReadSingle(adc_ch_t CH1, uint16_t *);

void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(cad_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
static void tareaCAD(void *pvParameter){

}
/*==================[external functions definition]==========================*/
void app_main(void){
	 timer_config_t timer_CAD = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_CAD,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_CAD);

	analog_input_config_t convAD = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
//AnalogInputInit(&convAD);

 xTaskCreate(tareaCAD, "CAD", 512, NULL, 5, &cad_task_handle);
}
/*==================[end of file]============================================*/