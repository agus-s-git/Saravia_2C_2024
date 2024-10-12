/*! @mainpage Guía 2 - Ejercicio 1
 *
 * @section Descripción
 *
 * El programa mide la distancia usando un sensor ultrasónico HC-SR04, 
 * controla LEDs basados en la distancia medida y muestra el resultado 
 * en una pantalla LCD. También permite manejar el sistema mediante botones.
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
 * @brief Intervalo de tiempo para medir la distancia en milisegundos 
 */
#define DELAY_MEDIR 1000
/**
 * @def DELAY_TECLAS
 * @brief Intervalo de tiempo para leer las teclas  
 */
#define DELAY_TECLAS 100

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
	
		vTaskDelay(CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
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
	}
	
}
/**
 * @fn static void tareaTeclas(void *pvParameter)
 * @brief Tarea para manejar el estado de las teclas.
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaTeclas(void *pvParameter){
	
	while (true)
	{
		int8_t tecla = SwitchesRead();
		switch (tecla)
		{
			case SWITCH_1:
				ON = !ON;
				printf("Tecla 1\n");
			break;

			case SWITCH_2:
				HOLD = !HOLD;
				printf("Tecla 2\n");
			break;
		}
	
		vTaskDelay(DELAY_TECLAS / portTICK_PERIOD_MS);
	}
}

/**
 * @fn static void tareaMostrarDistancia(void *pvParameter)
 * @brief Tarea para mostrar la distancia medida en la pantalla LCD.
 * @param pvParameter Parámetro de FreeRTOS
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

		vTaskDelay(DELAY_MOSTRAR / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	LcdItsE0803Init();
	

	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaMostrarDistancia, "Mostrar distancia", 2048, NULL, 3, &led_task_handle);
	xTaskCreate(&tareaTeclas, "Teclas", 2048, NULL, 5, &teclas_task_handle);

	 


}
/*==================[end of file]============================================*/