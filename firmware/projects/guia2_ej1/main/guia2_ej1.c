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
#define CONFIG_BLINK_PERIOD_MEDIR 500
uint16_t DISTANCIA;

/*==================[internal data definition]===============================*/
TaskHandle_t led_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;


/*==================[internal functions declaration]=========================*/

static void tareaMedir(void *pvParameter){
	while (1)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR / portTICK_PERIOD_MS);
	}
	
}
//static void tareaLCD(void){	
//}
static void tareaLEDs(void *pvParameter){
	while (1)
	{
	void manejarLEDs();
	vTaskDelay (CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
	}
	
}

//static void tareaTeclas(void){
//}
/*
void run_stop_medicion(void){
	tecla = SwitchesRead();
	switch (tecla)
	{
	case SWITCH_1:

		LcdItsE0803Write(DISTANCIA);
		break;
	case SWITCH_2:
		medida_hold = LcdItsE0803Read();
		LcdItsE0803Write(medida_hold);
	default:
		break;
	}
	
}
*/

void manejarLEDs(void){
	while (true)
	{
	
	if (DISTANCIA<10)
	{
		printf("LEDs OFF\n");
		LedsOffAll();
	}
	else if (DISTANCIA > 10 && DISTANCIA < 20)
	{
		printf("LED 1 ON\n");
		LedOn(LED_1);
	}
	else if (DISTANCIA > 20 && DISTANCIA < 30)
	{
		printf("LED 1 Y LED 2 ON\n");
		LedOn(LED_1);
		LedOn(LED_2);
	}
	else if (DISTANCIA > 30)
	{
		printf("LED 1, LED 2 Y LED 3 ON\n");
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
	
	}
}



/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Deinit();
	
	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaLEDs, "LEDs", 2048, NULL, 3, &led_task_handle);
}
/*==================[end of file]============================================*/