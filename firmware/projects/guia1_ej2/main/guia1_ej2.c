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
#include <led.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

#define ON 1
#define OFF 0
#define TOGGLE 2
#define delay 100

/*==================[internal functions declaration]=========================*/


/*==================[external functions definition]==========================*/


struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds; 

void manejoLEDs(struct leds *ptrLEDs);

void app_main(void){
	LedsInit();

	my_leds.mode = 2 ;
	my_leds.n_led = 3 ;
	my_leds.n_ciclos = 1000 ;
	my_leds.periodo = 500 ;

manejoLEDs(&my_leds);

}

void manejoLEDs(struct leds *ptrLEDs){
	if (ptrLEDs->mode == ON)
	{
		if (ptrLEDs->n_led == 1)
		{
			printf("LED 1 ENCENDIDO\n");
        	LedOn(LED_1);
		}
		else if (ptrLEDs->n_led == 2)
		{
			printf("LED 2 ENCENDIDO\n");
        	LedOn(LED_2);
		}
		else if (ptrLEDs->n_led == 3)
		{
			printf("LED 3 ENCENDIDO\n");
        	LedOn(LED_3);
		}
		
	}
	else if (ptrLEDs->mode == OFF)
	{
		if (ptrLEDs->n_led == 1)
		{
        	LedOff(LED_1);
		}
		else if (ptrLEDs->n_led == 2)
		{
        	LedOff(LED_2);
		}
		else if (ptrLEDs->n_led == 3)
		{
        	LedOff(LED_3);
		}
	}
	else if (ptrLEDs->mode == TOGGLE)
	{
		for (int i = 0; i < my_leds.n_ciclos ; i++)
		{
			if (ptrLEDs->n_led == 1)
		{
        	LedToggle(LED_1);
		}
		else if (ptrLEDs->n_led == 2)
		{
        	LedToggle(LED_2);
		}
		else if (ptrLEDs->n_led == 3)
		{
        	LedToggle(LED_3);
		}
		int retardo = (ptrLEDs->periodo / delay);

		for (int j = 0; j < retardo ; j++)
		{
			vTaskDelay(delay/ portTICK_PERIOD_MS);
		}
		
		}
		
	}
	
	
}






/*==================[end of file]============================================*/
 