/*! @mainpage EXAMEN
 *
 * @section Descripción
 *
 *El programa mide la distancia con un sensor HC-SR04 y activa
 o desactiva LEDs de la placa y una buzzer externo dependiendo de
 la distancia medida. Además, envia los datos por UART a un modulo
 bluetooth indicando si existe peligro o precaucion por algun vehiculo
 e informa si hubo una caida mediante las mediciones de un acelerometro
 cuyos valores son convertidos en los respectivos conversore ADC de la placa.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |   Peripheral  | ESP32        |
 * |---------------|--------------|
 * |  HC-SR04 Trig |   GPIO_3     |
 * |  HC-SR04 Echo |   GPIO_2     |
 * |  LED 1        |  GPIO_10     |
 * |  LED 2        |  GPIO_11     |
 * |  LED 3        |  GPIO_5      |
 * |   BUZZER      |  GPIO_20     |
 * |   ADC_x       |    CH0       |
 * |   ADC_y       |    CH1       |
 * |   ADC_z       |    CH2       | 
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Agustín Saravia (agustin.saravia@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include <stdbool.h>
#include "hc_sr04.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <gpio_mcu.h>
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_BLINK_PERIOD_LED
 * @brief Periodo de parpadeo de los LEDs en milisegundos 
*/ 
#define CONFIG_BLINK_PERIOD_LED 1000
/**
 * @def CONFIG_BLINK_PERIOD_BUZZER_PELIGRO
 * @brief Periodo de parpadeo de buzzer en milisegundos cuando hay peligro 
*/ 
#define CONFIG_BLINK_PERIOD_BUZZER_PELIGRO 500
/**
 * @def CONFIG_BLINK_PERIOD_BUZZER_PELIGRO
 * @brief Periodo de parpadeo de buzzer en milisegundos cuando hay precaución 
*/ 
#define CONFIG_BLINK_PERIOD_BUZZER_PRECAUCION 1000
/**
 * @def CONFIG_PERIOD_UART
 * @brief Periodo para el envío de información por UART
*/ 
#define CONFIG_PERIOD_UART 1000
/*! @brief Variable para almacenar la distancia medida en centímetros */
uint16_t DISTANCIA;
/**
 * @def DELAY_MEDIR
 * @brief Intervalo de tiempo para medir la distancia en milisegundos 
 */
#define DELAY_MEDIR 500
/**
 * @def CONFIG_PERIOD_A
 * @brief  Período de configuración para el temporizador A
 */
#define CONFIG_PERIOD_A 1000000
/**
 * @def CONFIG_BLINK_PERIOD_CAD
 * @brief Período del temporizador para la tarea CAD en milisegundos.
 */
#define CONFIG_BLINK_PERIOD_CAD 10
uint16_t v_analog_x;
uint16_t v_analog_y;
uint16_t v_analog_z;

int SUMA_EJES;

/*==================[internal data definition]===============================*/
/*! @brief Manejador de la tarea de medición de distancia */
TaskHandle_t medir_task_handle = NULL;
/*! @brief Manejador de la tarea que involucra la distancia, el manejo de LEDs y el buzzer */
TaskHandle_t distancia_task_handle = NULL;
/*! @brief Manejador de la tarea que involucra la UART*/
TaskHandle_t uart_task_handle = NULL;
/*! @brief Manejador de la tarea CAD.*/
TaskHandle_t cad_task_handle = NULL;

#define LED_VERDE LED_1
#define LED_AMARILLO LED_2
#define LED_ROJO LED_3

#define GPIO_BUZZER GPIO_20

bool HAY_PELIGRO ;
bool HAY_PRECAUCION ; 



/*==================[internal functions declaration]=========================*/
/**
 * @fn static void tareaMedir(void *pvParameter)
 * @brief Tarea que mide la distancia utilizando el sensor HC-SR04.
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaMedir(void *pvParameter){
	while (true)
	{
		DISTANCIA = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
	}
	
}
/**
 * @fn manejarLEDs()
 * @brief Función que controla el encendido de los LEDs según la distancia medida.
 */
static void manejarLEDs(){	

		if (DISTANCIA>500)
		{
			HAY_PELIGRO = false;
			HAY_PRECAUCION = false;
			LedOn(LED_VERDE);
			
		}
		else if (DISTANCIA >= 300 && DISTANCIA <= 500)
		{
			HAY_PRECAUCION = true;
			HAY_PELIGRO = false;
			LedOn(LED_VERDE);
			LedOn(LED_AMARILLO);
			
		}
		else if (DISTANCIA <= 300)
		{
			HAY_PELIGRO = true;
			HAY_PRECAUCION = false;
			LedOn(LED_VERDE);
			LedOn(LED_AMARILLO);
			LedOn(LED_ROJO);
		}
	
		vTaskDelay(CONFIG_BLINK_PERIOD_LED / portTICK_PERIOD_MS);
}
/**
 * @fn void encenderAlarmaSonoraPeligro()
 * @brief Tarea que enciende la alarma sonora a una frecuencia de medio segundo
 */
void encenderAlarmaSonoraPeligro(){
	GPIOOff(GPIO_BUZZER);
	GPIOOn(GPIO_BUZZER);

	vTaskDelay(CONFIG_BLINK_PERIOD_BUZZER_PELIGRO / portTICK_PERIOD_MS);

}
/**
 * @fn void encenderAlarmaSonoraPrecaución()
 * @brief Tarea que enciende la alarma sonora a una frecuencia de un segundo
 */
void encenderAlarmaSonoraPrecaución(){
	GPIOOff(GPIO_BUZZER);
	GPIOToggle(GPIO_BUZZER);

	vTaskDelay(CONFIG_BLINK_PERIOD_BUZZER_PRECAUCION / portTICK_PERIOD_MS);

}
/**
 * @fn static void  tareaDistancia(void *pvParameter)
 * @brief Tarea que enciende la alarma sonora dependien de si hay peligro o hay precaucion
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaDistancia(void *pvParameter){
	while (1)
	{
		manejarLEDs();

		if (HAY_PRECAUCION)
		{
			encenderAlarmaSonoraPrecaución();
		}
		else if (HAY_PELIGRO)
		{
			encenderAlarmaSonoraPeligro();
		}
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	}
	
}
/**
 * @fn void FuncTimerDistancia(void* param)
 * @brief Función asociada al temporizador para activar el manejo de la distancia y extras.
 * @param param Parametro no utilizado
 */
void FuncTimerDistancia(void* param){
    vTaskNotifyGiveFromISR(distancia_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}
/**
 * @fn static void tareaCAD(void *pvParameter)
 * @brief Tarea que lee un dato analógico de cada eje (x, y, z)
 * @param pvParameter Parámetro de FreeRTOS
 */
static void tareaCAD(void *pvParameter){
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH0, &v_analog_x);
		AnalogInputReadSingle(CH1, &v_analog_y);
		AnalogInputReadSingle(CH2, &v_analog_z);
       
    }
}
//.
/**
 * @fn void sumaEjes()
 * @brief Función que convierte los valores analogicos de los ejes y los suma
 * @param param Parametro no utilizado
 */
void sumaEjes(){
	int v_analog_x_G = (v_analog_x*5.5)/3.3 ;
	int v_analog_y_G = (v_analog_y*5.5)/3.3 ;
	int v_analog_z_G = (v_analog_z*5.5)/3.3 ;

	SUMA_EJES = v_analog_x_G + v_analog_y_G + v_analog_z_G ;
}

void tareaUART(){
	while (1)
	{
		char msg_precaucion = "Precaución, vehículo cerca";
		char msg_peligro = "Peligro, vehículo cerca";
		char msg_caida = "Caída detectada";

		sumaEjes();

		if (HAY_PRECAUCION)
		{
			UartSendString(UART_CONNECTOR, (char*)msg_precaucion);
		}
		else if (HAY_PELIGRO)
		{
			UartSendString(UART_CONNECTOR, (char*)msg_peligro);
		}
		
		
		if (SUMA_EJES > 4){
			UartSendString(UART_CONNECTOR, (char*)msg_caida);
		}

		vTaskDelay(CONFIG_PERIOD_UART / portTICK_PERIOD_MS);
	}
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
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
 	GPIOInit(GPIO_BUZZER);
	
	xTaskCreate(&tareaMedir, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&tareaDistancia, "Distancia y anexos", 2048, NULL, 5, &distancia_task_handle);
	xTaskCreate(&tareaUART, "UART", 2048, NULL, 5, &uart_task_handle);
	xTaskCreate(tareaCAD, "CAD", 2048, NULL, 5, &cad_task_handle);
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
    TimerStart(timer_CAD.timer);
	//Configuracion del conversor del eje x
	analog_input_config_t convAD_x = {
		.input = CH0,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD_x);
	//Configuracion del conversor del eje y
	analog_input_config_t convAD_y = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD_y);
	//Configuracion del conversor del eje z
	analog_input_config_t convAD_z = {
		.input = CH2,
		.mode = ADC_SINGLE,
	};
    AnalogInputInit(&convAD_z);


	TimerStart(timer_distancia.timer);
	TimerStart(timer_CAD.timer);
}
/*==================[end of file]============================================*/