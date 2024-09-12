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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED 1000
#define DELAY_MOSTRAR 500
#define DELAY_MEDIR 1000
#define DELAY_TECLAS 100


uint16_t DISTANCIA;
bool ON;
bool HOLD;

/*==================[internal data definition]===============================*/
TaskHandle_t led_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
//Listo
static void manejarLEDs(){
	
	
	//while (true)
	//{
	
		

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

	//}
}
//Listo
static void tareaMedir(void *pvParameter){
	while (true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
		printf("Distancia: %d\n",DISTANCIA);
	}
	
}

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