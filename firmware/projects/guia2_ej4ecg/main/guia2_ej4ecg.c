/*! @mainpage Guía 2 - Ejercicio 3
 *
 * @section Descripción
 *
 * El programa modifica la actividad 2 agregando
 * un puerto serie. Se envían datos de las mediciones
 * para que puedan ser observados en un terminal en 
 * la PC. Además, con las teclas "O" y "H" se enciende
 * y se mantiene la medición respectivamente.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral  | ESP32        |
 * |-------------|--------------|
 * |  HC-SR04 Trig | GPIO_3     |
 * |  HC-SR04 Echo | GPIO_2     |
 * |  LED 1      | GPIO_20      |
 * |  LED 2      | GPIO_21      |
 * |  LED 3      | GPIO_22      |
 * |  Switch 1   | GPIO_4      |
 * |  Switch 2   | GPIO_15      |
 * |  CH1        | GPIO_1      |
 * |  CH0        | GPIO_0      |
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
 * @def CONFIG_BLINK_PERIOD_ECG
 * @brief Período del temporizador para la tarea ECG en milisegundos.
 */
#define CONFIG_BLINK_PERIOD_ECG 40000

/**
 * @def BUFFER_SIZE
 * @brief Tamaño del buffer que almacena los valores del ECG.
 */
#define BUFFER_SIZE 231

/**
 * @brief Contador que controla el avance en el buffer de la señal ECG.
 */
int contador_ecg = 0;

/**
 * @brief Variable para almacenar el valor leído desde el canal analógico.
 */
uint16_t v_analog;

/*==================[internal data definition]===============================*/
/**
 * @brief Manejador de la tarea CAD.
 */
TaskHandle_t cad_task_handle = NULL;

/**
 * @brief Manejador de la tarea ECG.
 */
TaskHandle_t ecg_task_handle = NULL;

/**
 * @brief Buffer que contiene la señal ECG para enviar como salida analógica.
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

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
 * @fn FuncTimerB(void* param)
 * @brief Función asociada al temporizador A, activa la tarea ECG.
 * @param param Parametro no utilizado
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(ecg_task_handle, pdFALSE);    
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
/**
 * @fn static void tareaECG(void *pvParameter)
 * @brief Tarea que envía como señal analógica una señal de ECG digital
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaECG(void *pvParameter){
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		if (contador_ecg<BUFFER_SIZE)
		{
			AnalogOutputWrite(ecg[contador_ecg]);
			contador_ecg++;
			
		}
		else{
			contador_ecg = 0;
		}
		
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
	timer_config_t timer_ECG = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_ECG,
        .func_p = FuncTimerB,
        .param_p = NULL
    };

	TimerInit(&timer_ECG);
    TimerStart(timer_ECG.timer);

    TimerInit(&timer_CAD);
    TimerStart(timer_CAD.timer);

	analog_input_config_t convAD = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD);
	AnalogOutputInit();

    serial_config_t mi_uart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = tareaCAD,
		.param_p = NULL
	};
	UartInit(&mi_uart);

    xTaskCreate(tareaCAD, "CAD", 512, NULL, 5, &cad_task_handle);
	xTaskCreate(tareaECG, "ECG", 512, NULL, 5, &ecg_task_handle);
}
/*==================[end of file]============================================*/