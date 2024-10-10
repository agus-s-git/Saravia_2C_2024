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
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2023 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include <stdbool.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "led.h"
#include "switch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de parpadeo de los LEDs en milisegundos 
*/ 
#define CONFIG_BLINK_PERIOD_LED 1000
/**
 * @def DELAY_MOSTRAR
 * @brief Delay para mostrar la distancia en la pantalla LCD  
 */

#define DELAY_MOSTRAR 500
/**
 * @def DELAY_MEDIR
 * @brief Intervalo de tiempo para medir la distancia  
 */
#define DELAY_MEDIR 1000
/**
 * @def DELAY_TECLAS
 * @brief Intervalo de tiempo para leer las teclas  
 */
#define DELAY_TECLAS 100
/**
 * @def CONFIG_PERIOD_SWITCH
 * @brief  Período de configuración para el temporizador de las teclas
 */
#define CONFIG_PERIOD_SWITCH 1000000

/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;
/*! @brief Variable que indica si el sistema está encendido */
bool ON;
/*! @brief Variable que indica si el sistema está en modo "HOLD" */
bool HOLD;

/*==================[internal data definition]===============================*/
/*! @brief Manejador de la tarea de mostrar */
TaskHandle_t mostrar_task_handle = NULL;
/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*! @brief Manejador de la tarea de teclas */
TaskHandle_t teclas_task_handle = NULL;
/*! @brief Manejador de la tarea del control de la UART */
TaskHandle_t uart_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @fn void detectarTeclas(void *param)
 * @brief Función para detectar lo que se envía por el serial monitor ("O" o "H")
 * @param param Parametro
 */
void detectarTeclas(void *param){
	uint8_t tecla;
	UartReadByte(UART_PC, &tecla);

	switch (tecla)
	{
	case 'O':
		ON = !ON;
		UartSendByte(UART_PC, (char*)&tecla);
		break;
	case 'H':
		HOLD = !HOLD;
		UartSendByte(UART_PC, (char*)&tecla);
		break;
	}
}
/**
 * @fn static void manejarLEDs()
 * @brief Función que controla el encendido de los LEDs según la distancia medida.
 */
static void manejarLEDs(){
	
		if (DISTANCIA<10)
		{
			LedsOffAll();
			
		}
		else if (DISTANCIA >= 10 && DISTANCIA <= 20)
		{
			LedOn(LED_1);
			
		}
		else if (DISTANCIA >= 20 && DISTANCIA <= 30)
		{
			LedOn(LED_1);
			LedOn(LED_2);
		}
		else if (DISTANCIA >= 30)
		{	
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
	

}
/**
 * @fn static void tareaMedir(void *pvParameter)
 * @brief Tarea que mide la distancia utilizando el sensor HC-SR04.
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaMedir(void *pvParameter){
	while (true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
		
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
	
}
/**
 * @fn void FuncTimerTeclas(void* param)
 * @brief Función asociada al temporizador para activar la lectura de teclas.
 * @param param Parametro no utilizado
 */
void FuncTimerTeclas(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
/**
 * @fn void FuncTimerMedir(void* param)
 * @brief Función asociada al temporizador para activar la medición de distancia.
 * @param param Parametro no utilizado
 */
void FuncTimerMedir(void* param){
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
	vTaskNotifyGiveFromISR(uart_task_handle, pdFALSE);
}
/**
 * @fn void tecla1()
 * @brief Función que alterna el estado del sistema (ON/OFF) al presionar la tecla 1.
 */
void tecla1(){
	ON = !ON;
}
/**
 * @fn void tecla1()
 * @brief Función que alterna el estado del sistema (ON/OFF) al presionar la tecla 2.
 */
void tecla2(){
	HOLD = !HOLD;
}
/**
 * @fn static void tareaMostrarDistancia(void *pvParameter)
 * @brief Tarea para mostrar la distancia en el LCD y controlar el encendido de los LEDs.
 * @param pvParameter Parámetro no utilizado.
 */
static void tareaMostrarDistancia(void *pvParameter){
	
	
	while(true){
		if (ON == true){

			manejarLEDs();

			if(HOLD == false)
			{
				LcdItsE0803Write(DISTANCIA);
			
			}
		}
		if (ON == false)
		{
			LcdItsE0803Off();
			LedsOffAll();
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}
/**
 * @fn void tareaUart(void *pvParameter)
 * @brief Tarea que envia la distancia medida por la UART.
 * @param pvParameter Parámetro no utilizado.
 */
void tareaUart(void *pvParameter){
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		UartSendString(UART_PC, (char*)UartItoa(DISTANCIA, 10));
		UartSendString(UART_PC, " ");
		UartSendString(UART_PC, "cm");
		UartSendString(UART_PC, "\r\n");
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	LcdItsE0803Init();
	
	SwitchActivInt(SWITCH_1, *tecla1, NULL);
	SwitchActivInt(SWITCH_2, *tecla2, NULL);

	

	 /* Inicialización de timers */
    timer_config_t timer_tecla = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_SWITCH,
        .func_p = FuncTimerTeclas,
        .param_p = NULL
    };
	TimerInit(&timer_tecla);

	timer_config_t timer_medir = {
        .timer = TIMER_B,
        .period = CONFIG_PERIOD_SWITCH,
        .func_p = FuncTimerMedir,
        .param_p = NULL
    };
	TimerInit(&timer_medir);


	TimerStart(timer_tecla.timer);
	TimerStart(timer_medir.timer);

	serial_config_t mi_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = detectarTeclas,
		.param_p = NULL
	};
	UartInit(&mi_uart);

	
	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaMostrarDistancia, "Mostrar distancia", 2048, NULL, 3, &mostrar_task_handle);
	xTaskCreate(&tareaUart, "UART", 2048, &mi_uart, 5, &uart_task_handle);


}
/*==================[end of file]============================================*/