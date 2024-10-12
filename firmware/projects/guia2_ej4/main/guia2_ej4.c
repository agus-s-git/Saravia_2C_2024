/*! @mainpage Guía 2 - Ejercicio 4
 *
 * @section Descripción
 *
 * El programa mide wl voltaje usando el conversor ADC de la placa,
 * envía esto por la UART y simula una señal ECG usando un conversor DAC.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral  | ESP32        |
 * |-------------|--------------|
 * |  ADC_CH1    |  GPIO_1      |
 * | DAC_OUT_CH0 |  GPIO_0      |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/10/2023 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
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
/**
 * @def CONFIG_BLINK_PERIOD_CAD
 * @brief Período del temporizador para la tarea CAD en milisegundos.
 */
#define CONFIG_BLINK_PERIOD_CAD 20000

/**
 * @brief Variable para almacenar el valor leído desde el canal analógico.
 */
uint16_t v_analog;

/*==================[internal data definition]===============================*/
/**
 * @brief Manejador de la tarea CAD.
 */
TaskHandle_t cad_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn FuncTimerA(void* param)
 * @brief Función asociada al temporizador A, activa la tarea CAD.
 * @param param Parametro no utilizado
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(cad_task_handle, pdFALSE);    
}
/**
 * @fn static void tareaCAD(void *pvParameter)
 * @brief Tarea que lee un dato analógico y luego lo envía por la UART.
 * @param pvParameter Parámetro de FreeRTOS
 */
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