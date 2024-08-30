/*! @mainpage Guia 1 - Ejercicio 6
 *
 * @section Descripcion
 *
 * El programa recibe un valor numérico de 32 bit y luego se muestra en una pantalla LCD
 *
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1		 	| 	GPIO_20		|
 * | 	D2		 	| 	GPIO_21		|
 * | 	D3		 	| 	GPIO_22		|
 * | 	D4		 	| 	GPIO_23		| 
 * | 	SEL_1	 	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V	 		| 	+5V			|
 * | 	GND		 	| 	GND			|

 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 29/08/2024 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
 *
 */


/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @struct gpioConf_t 
 * @brief Estructura que representa el número de pin y la dirección de un GPIO
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/**
 * @var vec_pines 
 * @brief vector de los pines de los conversores de BCD-7 segmentos
 */
gpioConf_t vec_pines[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT} };

/**
 * @var vec_pines_select
 * @brief vector de los pines selectores de los conversores BCD-7 segmentos
 */

gpioConf_t vec_pines_select [3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};


/*==================[internal functions declaration]=========================*/

/**
 * @fn int8_t setPines(uint8_t bcd_digit, gpioConf_t *vec_struct)
 * @brief función que cambia el estado del GPIO a '0' o '1'  según el estado del bit correspondiente en el BCD ingresado
 * @param bcd_digit  digito BCD
 * @param vec_struct vector de estructura del struct gpioConf_t
 * @return devuelve 1 si es la operacion es exitosa
 */

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

/**
 * @fn int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief La función convierte el dato recibido a BCD, luego guarda cada uno de los dígitos de salida en el arreglo pasado como puntero.
 * @param data dato de 32 bits
 * @param digits cantidad de digitos de salida
 * @param bcd_number puntero a un arreglo donde se almacenan los digitos
 */

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{

for (int i = 0; i < digits; i++)
{
	bcd_number[digits-i-1] = data % 10;
	data = data / 10;
}

return 1;
}

/**
 * @fn void Numero_aLCD (uint32_t dato, uint8_t dig_out, gpioConf_t* vect_struct, gpioConf_t* puerto_LCD)
 * @brief La función muestra en el display el valor numérico que recibe.
 * @param dato valor numérico que recibe y deseamos mostrar.
 * @param dig_out cantidad de digitos de salida.
 * @param vect_struct vector de estructura del tipo gpioConf_t.
 * @param puerto_LCD vector que mapea los puertos con el dígito del LCD, sería una especie de llave selectora.
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

	Numero_aLCD(007, 3, vec_pines, vec_pines_select);
}
/*==================[end of file]============================================*/