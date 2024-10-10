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
uint16_t v_analog;
#define CONFIG_BLINK_PERIOD_CAD 20000
/*==================[internal data definition]===============================*/
TaskHandle_t cad_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
//void AnalogInputReadSingle(adc_ch_t CH1, uint16_t v_analog*);
//void UartSendString(uart_mcu_port_t UART_PC, const char *v_analog);

void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(cad_task_handle, pdFALSE);    
}
static void tareaCAD(void *pvParameter){
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &v_analog);
        UartSendString(UART_PC, (char*)UartItoa(v_analog, 10));
        UartSendString(UART_PC, "\r\n");
    }
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
    TimerStart(timer_CAD.timer);

	analog_input_config_t convAD = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD);

    serial_config_t mi_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = tareaCAD,
		.param_p = NULL
	};
	UartInit(&mi_uart);

    xTaskCreate(tareaCAD, "CAD", 512, NULL, 5, &cad_task_handle);
}
/*==================[end of file]============================================*/