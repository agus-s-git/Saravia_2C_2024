/*! @mainpage Recuperatorio 
 *
 * @section Descripción
 *
 
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral  | ESP32        |
 * |-------------|--------------|
 * |  HC-SR04 Trig | GPIO_3     |
 * |  HC-SR04 Echo | GPIO_2     |
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/19/2024 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <gpio_mcu.h>
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de parpadeo de los LEDs en milisegundos 
*/ 
#define CONFIG_BLINK_PERIOD_LED 1000

/*==================[internal data definition]===============================*/
/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;

//*! @brief Variable para almacenar la distancia medida en centímetros para eñ calculo de velocidad*/
//const char DISTANCIA_VEL[10] = 0;
//*! @brief Variable para almacenar el valor de tiempo */
//const char TIEMPO_VEL[10] = 0;

/*! @brief Variable para almacenar la velocidad en metros sobre segundo */
uint16_t VELOCIDAD;

// /**
//  * @def DELAY_MEDIR
//  * @brief Intervalo de tiempo para medir la distancia en milisegundos 
//  */
// #define DELAY_MEDIR 100


/**
 * @def CONFIG_PERIOD_A
 * @brief  Período de configuración para el temporizador A en microsegundos
 */
#define CONFIG_PERIOD_A 100000

bool VEHICULO_DETENIDO;

/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @fn static void tareaMedir(void *pvParameter)
 * @brief Tarea que mide la distancia utilizando el sensor HC-SR04.
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaMedir(void *pvParameter){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
	}
	
}

void calcular_velocidad()
{
	char DISTANCIA_VEL [10]; //Buffer de tamaño 10
	char TIEMPO_VEL [10]; //Buffer de tamaño 10

	if (DISTANCIA < 100)
	{
		for (int i = 1; i < 10; i++)
		{
			char DISTANCIA_VEL[i] = (DISTANCIA / 100) ; //Guardo el valor de distancia en metros
			char TIEMPO_VEL[i] = (i * 0.1); //Guardo el tiempo en segundos	
		}
		
		int delta_distancia = DISTANCIA_VEL[1] - DISTANCIA_VEL[10];
		int delta_tiempo = TIEMPO_VEL[1] - TIEMPO_VEL[10];

		VELOCIDAD = delta_distancia / delta_tiempo ; 
		
	}
}

/**
 * @fn manejarLEDs()
 * @brief Función que controla el encendido de los LEDs según la velocidad medida.
 */
static void manejarLEDs(){	

	LedsOffAll();

	if (VELOCIDAD > 8)
	{
		LedOn(LED_3);
		LedOff(LED_2);
		LedOff(LED_1);
	}
	else if (VELOCIDAD > 0 && VELOCIDAD <= 8)
	{
		LedOn(LED_2);
		LedOff(LED_3);
		LedOff(LED_1);
			
	}
	else if (VELOCIDAD == 0)
	{
		VEHICULO_DETENIDO == true ;
		
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	vTaskDelay(CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
}

/**
 * @fn void FuncTimerDistancia(void* param)
 * @brief Función asociada al temporizador para activar el manejo de la distancia y extras.
 * @param param Parametro no utilizado
 */
void FuncTimerDistancia(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
/*==================[external functions definition]==========================*/
void app_main(void){
	//Iniciañizaciones
	HcSr04Init(GPIO_3, GPIO_2);

	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);

	//Configuracion de los timers
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