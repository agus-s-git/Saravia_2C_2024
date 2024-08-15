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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{

for (int i = 0; i < digits; i++)
{
	bcd_number[i] = data % 10;
	data = data / 10;
}

return 1;
}



/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t vector[3] ;
	//int *ptr_vector ;
	//ptr_vector = vector ;

	convertToBcdArray(123, 3, vector);

	for (int i = 0; i < 3; i++)
	{
		printf("%d\n", vector[i]);
	}
	
}
/*==================[end of file]============================================*/