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
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "led.h"
#include "switch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED 1000
#define DELAY_MOSTRAR 500
#define DELAY_MEDIR 1000
#define DELAY_TECLAS 100
#define CONFIG_PERIOD_SWITCH 1000000

uint16_t DISTANCIA;
bool ON;
bool HOLD;

/*==================[internal data definition]===============================*/
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;
TaskHandle_t uart_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

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

static void manejarLEDs(){
	
		if (DISTANCIA<10)
		{
			LedsOffAll();
			//printf("LEDs OFF\n");
			
		}
		else if (DISTANCIA >= 10 && DISTANCIA <= 20)
		{
			LedOn(LED_1);
			//printf("LED 1 ON\n");
			
		}
		else if (DISTANCIA >= 20 && DISTANCIA <= 30)
		{
			//printf("LED 1 Y LED 2 ON\n");
			LedOn(LED_1);
			LedOn(LED_2);
		}
		else if (DISTANCIA >= 30)
		{
			//printf("LED 1, LED 2 Y LED 3 ON\n");
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
	

}
//Listo
static void tareaMedir(void *pvParameter){
	while (true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
		//printf("Distancia: %d\n",DISTANCIA);
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
	
}
void FuncTimerTeclas(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
void FuncTimerMedir(void* param){
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
	vTaskNotifyGiveFromISR(uart_task_handle, pdFALSE);
}
void tecla1(){
	ON = !ON;
}
void tecla2(){
	HOLD = !HOLD;
}

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
	//xTaskCreate(&tareaTeclas, "Teclas", 2048, NULL, 5, &teclas_task_handle);
	xTaskCreate(&tareaUart, "UART", 2048, &mi_uart, 5, &uart_task_handle);


}
/*==================[end of file]============================================*/