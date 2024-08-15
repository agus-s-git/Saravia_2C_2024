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
#include <math.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/



/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t vec_pines[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT} };

/*==================[internal functions declaration]=========================*/
int8_t setPines(uint8_t bcd_digit, struct gpioConf_t *vec_struct)
{
	uint8_t mask = 1;
	for (uint8_t i = 0; i < 4; i++)
	{
		GPIOInit(vec_struct[i].pin, vec_struct[i].dir)
	}

	for (u_int8_t i = 0; i < 4; i++)
	{
		if (bcd_digit & mask) != 0
		{
			//poner el pin en 1
			GPIOOn(vec_struct[i].pin); 
		}
		else
		{
			//poner en 0
			GPIOOff(vec_struct[i].pin);
		}
		mask = mask << 1 ;
	}
	
	


} 

/*==================[external functions definition]==========================*/
void app_main(void){

	GPIOInit();

	uint8_t digito = 4

	setPines(digito, vec_pines);
	
}
/*==================[end of file]============================================*/