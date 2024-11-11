/*! @mainpage Recuperatorio 
 *
 * @section Descripción
 *
 * El programa mide la velocidad de un vehiculo utilizando el sensor HC-SR04,
 * también con los leds de la placa indica la velocidad. Además, se utilizab dos
 * sensores analogicos para medir el peso del vehiculo, este peso es enviado por
 * UART al igual que la velocidad. El usuario es capaz de controlar una barrera
 * con las teclas 'o' y 'c'.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral  | ESP32        |
 * |-------------|--------------|
 * |  HC-SR04 Trig | GPIO_3     |
 * |  HC-SR04 Echo | GPIO_2     |
 * |  LED 1        |  GPIO_10     |
 * |  LED 2        |  GPIO_11     |
 * |  LED 3        |  GPIO_5      |
 * |   BARRERA     |  GPIO_20     |
 * |   ADC_GALGA_1 |    CH0       |
 * |   ADC_GALGA_2 |    CH1       |
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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de parpadeo de los LEDs en milisegundos 
*/ 
#define CONFIG_BLINK_PERIOD_LED 1000
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de calculo de la velocidad 
*/ 
#define CONFIG_BLINK_PERIOD_VELOCIDAD 1000
/**
 * @def CONFIG_PERIOD_A
 * @brief  Período de configuración para el temporizador A correspondiente a medir, en microsegundos
 */
#define CONFIG_PERIOD_A 100000
/**
 * @def CONFIG_BLINK_PERIOD_CAD
 * @brief Período del temporizador para la tarea CAD en MICROSEGUNDOS.
 */
#define CONFIG_BLINK_PERIOD_CAD 5000 //Para un periodo de 200 Hz
/**
 * @def GPIO_BARRERA
 * @brief GPIO de la barrera.
 */
#define GPIO_BARRERA GPIO_20
/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;
/*! @brief Variable para almacenar la velocidad en metros sobre segundo */
uint16_t VELOCIDAD;
/*! @brief Variable para almacenar el peso en kilogramos */
uint16_t PESO;
/*! @brief Variable donde se almacena el valor del conversor de la galga 1 */
uint16_t v_analog_g1;
/*! @brief Variable donde se almacena el valor del conversor de la galga 2 */
uint16_t v_analog_g2;
/*! @brief Variable que indica si el vehiculo está detenido(true: está detenido, false: está en marcha) */
bool VEHICULO_DETENIDO;
/*==================[internal data definition]===============================*/

/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*! @brief Manejador de la tarea CAD.*/
TaskHandle_t cad_task_handle = NULL;
/*! @brief Manejador de la tarea del control de la UART */
TaskHandle_t uart_task_handle = NULL;

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
/**
 * @fncalcular_velocidad()
 * @brief Función que calcula la velocidad.
 */
void calcular_velocidad(){

	char DISTANCIA_VEL [10]; //Buffer de tamaño 10
	char TIEMPO_VEL [10]; //Buffer de tamaño 10
	int delta_distancia;
	int delta_tiempo;
	
	if (DISTANCIA < 100)
	{
		for (int i = 0; i < 10; i++)
		{
			char DISTANCIA_VEL[i] = (DISTANCIA / 100) ; //Guardo el valor de distancia en metros
			char TIEMPO_VEL[i] = (i+1 * 0.1); //Guardo el tiempo en segundos	
		}
		
		delta_distancia = DISTANCIA_VEL[1] - DISTANCIA_VEL[10];
		delta_tiempo = TIEMPO_VEL[1] - TIEMPO_VEL[10];

		
	}
		VELOCIDAD = delta_distancia / delta_tiempo ; 

	vTaskDelay(CONFIG_BLINK_PERIOD_VELOCIDAD / portTICK_PERIOD_MS);
}

/**
 * @fn manejarLEDs()
 * @brief Función que controla el encendido de los LEDs según la velocidad medida.
 */
static void manejarLEDs(){	

	LedsOffAll();

	if (VELOCIDAD > 8)
	{
		VEHICULO_DETENIDO == false ;
		LedOn(LED_3);
		LedOff(LED_2);
		LedOff(LED_1);
	}
	else if (VELOCIDAD > 0 && VELOCIDAD <= 8)
	{
		VEHICULO_DETENIDO == false ;
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
 * @fn static void tareaCAD(void *pvParameter)
 * @brief Tarea que lee un dato analógico de cada galga (1 y 2)
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaCAD(void *pvParameter){
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH0, &v_analog_g1);
		AnalogInputReadSingle(CH1, &v_analog_g2);
       
    }
}
/**
 * @fn calcular_peso()
 * @brief Función que que calcula el peso promediado de 50 mediciones de cada balanza
 */
void calcular_peso(){

	//Pasaje de mV a Kg
	int v_analog_g1_kg = (v_analog_g1/20000) / 1000 ; 
	int v_analog_g2_kg = (v_analog_g2/20000) / 1000 ;
	int promedio_g1;
	int promedio_g2;

	if (VEHICULO_DETENIDO == true)
	{	
		
		int acum_g1_kg = 0 ;
		int acum_g2_kg = 0 ;
		for (int i = 0; i < 50; i++)
		{
			acum_g1_kg =+ v_analog_g1_kg;
			acum_g2_kg =+ v_analog_g2_kg;
		}

		promedio_g1 = acum_g1_kg/50 ;
		promedio_g2 = acum_g2_kg/50 ;

	}
		PESO = promedio_g1 + promedio_g2 ;
	

}
void tareaUart(void *pvParameter){
	while(true){
		
		calcular_peso();

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		UartSendString(UART_PC, "Peso : ");
		UartSendString(UART_PC, (char*)UartItoa(PESO, 10));
		UartSendString(UART_PC, "kg");
		UartSendString(UART_PC, "\r\n");
		UartSendString(UART_PC, "Velocidad : ");
		UartSendString(UART_PC, (char*)UartItoa(VELOCIDAD, 10));
		UartSendString(UART_PC, "m/s");
		UartSendString(UART_PC, "\r\n");
	}
}
/**
 * @fn void detectarTeclas(void *param)
 * @brief Función para detectar lo que se envía por el serial monitor ("o" o "c")
 * @param param Parametro
 */
void detectarTeclas(void *param){
	uint8_t tecla;
	UartReadByte(UART_PC, &tecla);

	switch (tecla)
	{
	case 'o':
		GPIOOn(GPIO_BARRERA);
		UartSendByte(UART_PC, (char*)&tecla);
		break;
	case 'c':
		GPIOOff(GPIO_BARRERA);
		UartSendByte(UART_PC, (char*)&tecla);
		break;
	}
}
/**
 * @fn void FuncTimerDistancia(void* param)
 * @brief Función asociada al temporizador para activar el manejo de la distancia.
 * @param param Parametro no utilizado
 */
void FuncTimerDistancia(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
/**
 * @fn FuncTimerA(void* param)
 * @brief Función asociada al temporizador A, activa la tarea CAD.
 * @param param Parametro no utilizado
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(cad_task_handle, pdFALSE);    
}

/*==================[external functions definition]==========================*/
void app_main(void){
	//Iniciañizaciones
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	GPIOInit(GPIO_BARRERA, GPIO_OUTPUT);



	//Inicializacion de UART
	serial_config_t mi_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = detectarTeclas,
		.param_p = NULL
	};
	UartInit(&mi_uart);
	
	//Configuracion de los timers
	timer_config_t timer_distancia = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_A,
        .func_p = FuncTimerDistancia,
        .param_p = NULL
    };
	TimerInit(&timer_distancia);
	
	timer_config_t timer_CAD = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_CAD,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_CAD);
   


	//Configuracion del conversor de la galga 1
	analog_input_config_t convAD_G1 = {
		.input = CH0,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD_G1);
	//Configuracion del conversor de la galga 2
	analog_input_config_t convAD_G2 = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&convAD_G2);

	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaCAD, "CAD", 2048, NULL, 5, &cad_task_handle);
	xTaskCreate(&tareaUart, "UART", 2048, &mi_uart, 5, &uart_task_handle);

	TimerStart(timer_distancia.timer);
	TimerStart(timer_CAD.timer);
}
/*==================[end of file]============================================*/