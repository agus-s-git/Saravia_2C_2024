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
/*
Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos 
de salida y dos vectores de estructuras del tipo  gpioConf_t. Uno  de estos 
vectores es igual al definido en el punto anterior y el otro vector mapea 
los puertos con el dígito del LCD a donde mostrar un dato:

Dígito 1 -> GPIO_19
Dígito 2 -> GPIO_18
Dígito 3 -> GPIO_9

La función deberá mostrar por display el valor que recibe. 
Reutilice las funciones creadas en el punto 4 y 5. 
Realice la documentación de este ejercicio usando Doxygen.

*/
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t vec_pines[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT} };

gpioConf_t vec_pines_select [3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};




/*==================[internal functions declaration]=========================*/
//Función ej 5
int8_t setPines(uint8_t bcd_digit, gpioConf_t *vec_struct)
{
	uint8_t mask = 1;
	
	for (uint8_t i = 0; i < 4; i++)
	{
		GPIOInit(vec_struct[i].pin, vec_struct[i].dir);
	}

	for (u_int8_t i = 0; i < 4; i++)
	{
		if ((bcd_digit & mask) != 0)
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
	
	
return 1;

} 
//Función ej 4
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{

for (int i = 0; i < digits; i++)
{
	bcd_number[digits-i-1] = data % 10;
	data = data / 10;
}

return 1;
}
/*
dato: numero que queremos mostrar
dig_out: digitos de salida
vect_struct: vector de los pines de los leds enel LCD
puerto_LCD: sería la llave selectora para cada digito
*/
void Numero_aLCD (uint32_t dato, uint8_t dig_out, gpioConf_t* vect_struct, gpioConf_t* puerto_LCD){
uint8_t vec_digit[3];
convertToBcdArray(dato, dig_out, vec_digit);
//Inicializamos los pines
for (uint8_t i = 0; i < 3; i++)
{
	GPIOInit(puerto_LCD[i].pin, puerto_LCD[i].dir);
}
//Usamos la función set pines
for (uint8_t i = 0; i < dig_out; i++)
{
	setPines(vec_digit[i], vect_struct);
	GPIOOn(puerto_LCD[i].pin);
	GPIOOff(puerto_LCD[i].pin);
}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	Numero_aLCD(007, 3, vec_pines, vec_pines_select);
}
/*==================[end of file]============================================*/