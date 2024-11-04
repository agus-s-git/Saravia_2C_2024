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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <gpio_mcu.h>
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de parpadeo de los LEDs en milisegundos 
*/ 
#define CONFIG_BLINK_PERIOD_LED 1000
/**
 * @def CONFIG_BLINK_PERIOD_BUZZER_PELIGRO
 * @brief Periodo de parpadeo de buzzer en milisegundos cuando hay peligro 
*/ 
#define CONFIG_BLINK_PERIOD_BUZZER_PELIGRO 500
/**
 * @def CONFIG_BLINK_PERIOD_BUZZER_PELIGRO
 * @brief Periodo de parpadeo de buzzer en milisegundos cuando hay precaución 
*/ 
#define CONFIG_BLINK_PERIOD_BUZZER_PRECAUCION 1000
/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;
/**
 * @def DELAY_MEDIR
 * @brief Intervalo de tiempo para medir la distancia en milisegundos 
 */
#define DELAY_MEDIR 500
/**
 * @def CONFIG_PERIOD_A
 * @brief  Período de configuración para el temporizador A
 */
#define CONFIG_PERIOD_A 1000000
/*==================[internal data definition]===============================*/
/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*! @brief Manejador de la tarea que involucra la distancia, el manejo de LEDs y el buzzer */
TaskHandle_t distancia_task_handle = NULL;

#define LED_VERDE LED_1
#define LED_AMARILLO LED_2
#define LED_ROJO LED_3

#define GPIO_BUZZER GPIO_20

bool HAY_PELIGRO ;
bool HAY_PRECAUCION ; 
/*==================[internal functions declaration]=========================*/
static void tareaMedir(void *pvParameter){
	while (true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
		printf("Distancia: %d\n",DISTANCIA);
	}
	
}
static void manejarLEDs(){	

		if (DISTANCIA>500)
		{
			HAY_PELIGRO = false;
			HAY_PRECAUCION = false;
			LedOn(LED_VERDE);
			
		}
		else if (DISTANCIA >= 300 && DISTANCIA <= 500)
		{
			HAY_PRECAUCION = true;
			HAY_PELIGRO = false;
			LedOn(LED_VERDE);
			LedOn(LED_AMARILLO);
			
		}
		else if (DISTANCIA <= 300)
		{
			HAY_PELIGRO = true;
			HAY_PRECAUCION = false;
			LedOn(LED_VERDE);
			LedOn(LED_AMARILLO);
			LedOn(LED_ROJO);
		}
	
		vTaskDelay(CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
}
/**
 * @fn static void tareaMedir(void *pvParameter)
 * @brief Tarea que mide la distancia utilizando el sensor HC-SR04.
 * @param pvParameter Parámetro de FreeRTOS
 */


void encenderAlarmaSonoraPeligro(){
	GPIOOff(GPIO_BUZZER);
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(CONFIG_BLINK_PERIOD_BUZZER_PELIGRO / portTICK_PERIOD_MS);

}
void encenderAlarmaSonoraPrecaución(){
	GPIOOff(GPIO_BUZZER);
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(CONFIG_BLINK_PERIOD_BUZZER_PRECAUCION / portTICK_PERIOD_MS);

}
static void tareaDistancia(void *pvParameter){
	while (1)
	{
		manejarLEDs();

		if (HAY_PRECAUCION)
		{
			encenderAlarmaSonoraPrecaución();
		}
		else if (HAY_PELIGRO)
		{
			encenderAlarmaSonoraPeligro();
		}
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	}
	
}
/**
 * @fn void FuncTimerDistancia(void* param)
 * @brief Función asociada al temporizador para activar el manejo de la distancia y extras.
 * @param param Parametro no utilizado
 */
void FuncTimerDistancia(void* param){
    vTaskNotifyGiveFromISR(distancia_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);

	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaDistancia, "Distancia y anexos", 2048, NULL, 5, &distancia_task_handle);

	timer_config_t timer_distancia = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_A,
        .func_p = FuncTimerDistancia,
        .param_p = NULL
    };
	TimerInit(&timer_distancia);

	TimerStart(timer_distancia.timer);
}
/*==================[end of file]============================================*/