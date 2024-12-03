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
 * | 03/012/2024 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def DELAY_MEDIR
 * @brief Intervalo de tiempo para medir la distancia en milisegundos  
 */
#define DELAY_MEDIR 5000
/**
 * @def GPIO_AGUA
 * @brief GPIO de la electroválvula para el suministro de agua.
 */
#define GPIO_AGUA GPIO_20
/**
 * @def GPIO_ALIMENTO
 * @brief GPIO del dispenser de alimento.
 */
#define GPIO_ALIMENTO GPIO_21
/*==================[internal data definition]===============================*/
/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;
/*! @brief Variable para almacenar el peso del alimento en gramos */
uint16_t PESO_ALIMENTO;
/*! @brief Variable para almacenar el volumen del agua en cm3 */
uint16_t VOLUMEN_AGUA

/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t aguaYalimento_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @fn static void tareaAguayAlimento(void *pvParameter)
 * @brief Tarea que mide la distancia utilizando el sensor HC-SR04, luego trabaja este valor para el manejo del
 * llenado de un reciepiente de agua, además maneja el llenado de alimento con una balanza.
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaAguayAlimento(void *pvParameter){
	while (true)
	{
		
			DISTANCIA = HcSr04ReadDistanceInCentimeters();

			//Del volumen de un cilindro: 2*pi*r^2*altura despejo la altura mínima
			//y máxima para cada volumen de agua

			//altura para medio litro de agua
			float h_min = (0.5/(3.14*100)) - 30 ; //cm3 / cm2
			//altura para 2.5 litro de agua 
			float h_max = (2.5/(3.14*100)) - 30 ;

			float h_faltante = 2.5/(3.14*100) - DISTANCIA ;
			
			//Le resto al volumen total el volumen faltante y obtengo el volumen que me queda
			VOLUMEN_AGUA =  2.5 - 2*3.14*100*h_faltante;

			if (DISTANCIA < h_min)
			{
				//Activo electrovalvula
				GPIOOn(GPIO_Agua);
			}
			if (DISTANCIA > h_max)
			{
				//Apago electrovalvula
				GPIOOff(GPIO_Agua);
			}
			
			//Ahora para el alimento

			AnalogInputReadSingle(CH0, &volt_analog);

			//Conversion de mV a gramos

			PESO_ALIMENTO = (volt_analog/*mV*/ * 1000/*gramos*/) / 3300//mV
		
			if (PESO_ALIMENTO < 50)
			{
				GPIOOn(GPIO_Alimento)
			}
			if (PESO_ALIMENTO > 500)
			{
				GPIOOff(GPIO_Alimento)
			}

			vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
	}
	
}
/*==================[external functions definition]==========================*/
void app_main(void){
	HcSr04Init(GPIO_3, GPIO_2);
	GPIOInit(GPIO_AGUA, GPIO_OUTPUT);
	GPIOInit(GPIO_ALIMENTO, GPIO_OUTPUT);

	//Configuracion del conversor para la balanza de alimento
	analog_input_config_t convAD = {
		.input = CH0,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD);

	xTaskCreate(&tareaAguayAlimento, "Medir distancia y controlar agua", 2048, NULL, 5, &aguaYalimento_task_handle);
}
/*==================[end of file]============================================*/