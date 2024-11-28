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
 * CORRECCIONES
 * 
 * 
- Programa organizado en tres tareas. Se inicializa la tarea tareaUart pasándole como parámetro la estructura de inicialización de UART. 
La tarea tareaUart nunca es notificada por un timer.
- Existen dos funciones (manejarLEDs() y calcular_velocidad()) que nunca son llamadas, y no están asignadas a ninguna tarea. 
- Sin embargo, la función que maneja los LEDs nunca va a ser ejecutada.
- la tarea que envía los mensajes nunca es notificada.
- La implementación del cálculo de velocidad tiene varios problemas. Los valores de distancia y tiempo se almacenan en variables de tipo char, 
con lo cual se perdería información. Se recomienda usar variables de tipo float. 
Los valores que se guardan en el arreglo DISTANCIA_VEL son el mismo valor que se midió antes de entrar al for de forma repetida. 
El cálculo podría plantearse como la diferencia entre mediciones sucesivas del sensor de ultrasonido, 
dividida por el tiempo entre mediciones (100ms), realizando las correspondientes conversiones de unidades. No se calcula la velocidad máxima.
- El pasaje de mV a kg está planteado de forma incorrecta. Podría plantearse como v_analog_g1_kg = v_analog_g1 * 20000/3300. 
El cálculo de promedio de peso está realizado promediando sobre una variable que acumula dentro de un for, 
por lo que el valor que se va acumulando es siempre el mismo, el último medido antes del ingreso al ciclo for. 
Se sumará 50 veces el mismo número, entonces el promedio dará ese mismo valor. 
Similar al caso de las mediciones del ultrasonido, se deberían acumular mediciones sucesivas de las galgas, no una misma muchas veces. 
El operador =+ no corresponde, es +=.
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

/*! @brief Tamaño de los buffer que almacenan las mediciones de peso de ambas galgas*/
#define BUFFER_SIZE 50
/*! @brief Vector que almacena las 50 mediciones de peso de la galga 1*/
float PESOS_GALGA_1 [BUFFER_SIZE];
/*! @brief Vector que almacena las 50 mediciones de peso de la galga 2*/
float PESOS_GALGA_2 [BUFFER_SIZE];
/*! @brief Variable que almacena el valor de la velocidad maxima del vehículo*/
float VELOCIDAD_MAX;

/*! @brief Variable que almacena el valor de la distancia medida anterior*/
int DISTANCIA_ANTERIOR;
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
//NUEVO
static void taskMedirVelocidad(void *pvParameter)
{
	while(true)
	{
		DISTANCIA_ANTERIOR = DISTANCIA;//Guardo la distancia anterior en DISTANCIA
		DISTANCIA = HcSr04ReadDistanceInCentimeters();//Leo nuevamente la distancia
		
		if (DISTANCIA < 1000) //Si el vehículo se encuentra a menos de 1000 cm...
		{
			VELOCIDAD = (DISTANCIA_ANTERIOR - DISTANCIA)/(0.1*100); 		//delta de desplazamiento sobre el tiempo que 																	
			manejarLEDs();//controlarLeds();//Llamo al manejo de leds		//transcurrió entre ambos puntos (es decir el tiempo de muestreo)
																			//Además se divide por 100 para convertir de cm a m
			if (VELOCIDAD > VELOCIDAD_MAX)
			{
				VELOCIDAD_MAX = VELOCIDAD;
			}
		}																	
		else 
		{
			LedsOffAll(); //si no hay vehículo se apagan los leds
			VELOCIDAD_MAX = -1; //La velocidad no podria ser negativa porque asumo que el vehiculo no retrocede según la consigna
								//Reinicio la v_max para el proximo camión
		}																	
			
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @fn calcular_velocidad()
 * @brief Función que calcula la velocidad.
 */
// void calcular_velocidad(){

// 	char DISTANCIA_VEL [10]; //Buffer de tamaño 10
// 	char TIEMPO_VEL [10]; //Buffer de tamaño 10
// 	int delta_distancia;
// 	int delta_tiempo;
	
// 	if (DISTANCIA < 100)
// 	{
// 		for (int i = 0; i < 10; i++)
// 		{
// 			char DISTANCIA_VEL[i] = (DISTANCIA / 100) ; //Guardo el valor de distancia en metros
// 			char TIEMPO_VEL[i] = (i+1 * 0.1); //Guardo el tiempo en segundos	
// 		}
		
// 		delta_distancia = DISTANCIA_VEL[1] - DISTANCIA_VEL[10];
// 		delta_tiempo = TIEMPO_VEL[1] - TIEMPO_VEL[10];

		
// 	}
// 		VELOCIDAD = delta_distancia / delta_tiempo ; 

// 	vTaskDelay(CONFIG_BLINK_PERIOD_VELOCIDAD / portTICK_PERIOD_MS);
// }

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
}
/**
 * @fn static void tareaCAD(void *pvParameter)
 * @brief Tarea que lee un dato analógico de cada galga (1 y 2)
 * @param pvParameter Parámetro de FreeRTOS
 */
//Puedo evitarme esta task y unirla con otra tarea, podria hacer calcular peso y leer el valor analog
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
// void calcular_peso(){

//Esto esta estrepitosamente mal

// 	//Pasaje de mV a Kg
// 	int v_analog_g1_kg = (v_analog_g1/20000) / 1000 ; 
// 	int v_analog_g2_kg = (v_analog_g2/20000) / 1000 ;
// 	int promedio_g1;
// 	int promedio_g2;

//El pasaje correcto seria:
/*
if (contador < BUFFER_SIZE)
			{
				AnalogInputReadSingle(CH_GALGA1, &voltaje_galga_1); //salida en mV
				AnalogInputReadSingle(CH_GALGA2, &voltaje_galga_2);

				PESOS_GALGA_1[contador] = (voltaje_galga_1 * 20000)/(3.3*1000); //Paso a volts acá directamente dividendo por mil
				PESOS_GALGA_2[contador] = (voltaje_galga_2 * 20000)/(3.3*1000);

				contador++;
			}
//Guardo los valores convertidos en un vector de tamaño 50 para luego trabajarlo
Luego:
else
			{
				float Promedio1 = 0; Inicializo los promedios en 0
				float Promedio2 = 0;

				contador = 0;

				for (int i = 0; i < BUFFER_SIZE; i++) //Hago una acumulacion de los valores que guarde anteriormente en los vectores
				{
					Promedio1 = PESOS_GALGA_1[i] + Promedio1;
					Promedio2 = PESOS_GALGA_2[i] + Promedio2; 
				}

				float PESO_PROMEDIO_1 = Promedio1 / BUFFER_SIZE; Aquí saco el promedio 
				float PESO_PROMEDIO_2 = Promedio2 / BUFFER_SIZE;

				PESO = PESO_PROMEDIO_1 + PESO_PROMEDIO_2;

				Luego podria informar por uart el peso y velocid maxima

*/

// 	if (VEHICULO_DETENIDO == true)
// 	{	
		
// 		int acum_g1_kg = 0 ;
// 		int acum_g2_kg = 0 ;
// 		for (int i = 0; i < 50; i++)
// 		{
// 			acum_g1_kg =+ v_analog_g1_kg;
// 			acum_g2_kg =+ v_analog_g2_kg;
// 		}

// 		promedio_g1 = acum_g1_kg/50 ;
// 		promedio_g2 = acum_g2_kg/50 ;

// 	}
// 		PESO = promedio_g1 + promedio_g2 ;
	

// }

//Esta tarea tambien podria evitarla, puedo obtener el peso y de ahi enviarlo por UART
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