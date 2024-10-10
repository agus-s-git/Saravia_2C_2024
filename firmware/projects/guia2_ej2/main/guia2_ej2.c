/*! @mainpage Guía 2 - Ejercicio 2
 *
 * @section Descripción
 *
 * El programa mide la distancia usando un sensor ultrasónico HC-SR04, 
 * controla LEDs basados en la distancia medida y muestra el resultado 
 * en una pantalla LCD. También permite manejar el sistema mediante botones.
 * Utilizando Timers.
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
 * | 12/09/2023 | Document creation		                         |
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
/*! @brief Manejador de la tarea de control de LEDs */
TaskHandle_t led_task_handle = NULL;
/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*! @brief Manejador de la tarea de teclas */
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @fn manejarLEDs()
 * @brief Función que controla el encendido de los LEDs según la distancia medida.
 */
static void manejarLEDs(){
	
		if (DISTANCIA<10)
		{
			LedsOffAll();
			printf("LEDs OFF\n");
			
		}
		else if (DISTANCIA >= 10 && DISTANCIA <= 20)
		{
			LedOn(LED_1);
			printf("LED 1 ON\n");
			
		}
		else if (DISTANCIA >= 20 && DISTANCIA <= 30)
		{
			printf("LED 1 Y LED 2 ON\n");
			LedOn(LED_1);
			LedOn(LED_2);
		}
		else if (DISTANCIA >= 30)
		{
			printf("LED 1, LED 2 Y LED 3 ON\n");
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
		printf("Distancia: %d\n",DISTANCIA);

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

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	LcdItsE0803Init();
	
	SwitchActivInt(SWITCH_1, *tecla1, NULL);
	SwitchActivInt(SWITCH_2, *tecla2, NULL);

	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaMostrarDistancia, "Mostrar distancia", 2048, NULL, 3, &mostrar_task_handle);
	//xTaskCreate(&tareaTeclas, "Teclas", 2048, NULL, 5, &teclas_task_handle);

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
}
/*==================[end of file]============================================*/