#include <asf.h>
#include <stdio.h>
#include "FreeRTOS/include/FreeRTOS.h"
#include "FreeRTOS/include/queue.h"
#include "FreeRTOS/include/task.h"
#include "FreeRTOS/include/timers.h"
#include "FreeRTOS/include/semphr.h"


/* Define a task */
static portTASK_FUNCTION_PROTO(vBlinkLed0, p_);
static portTASK_FUNCTION_PROTO(vBlinkLed1, q_);
static portTASK_FUNCTION_PROTO(vCounter, r_);
static portTASK_FUNCTION_PROTO(vPushButton1, s_);
static portTASK_FUNCTION_PROTO(vServo, t_);

/* Define semaphore */
SemaphoreHandle_t xSemaphore;
uint16_t counter = 0;

/* Inisiliasi Single Slope PWM */
void PWM_Init(void)
{
	/* Set output pin untuk servo */
	PORTC.DIR |= PIN0_bm;

	/* Set Register */
	TCC0.CTRLA = TC_CLKSEL_DIV8_gc;
	TCC0.CTRLB = (PIN4_bm) | (PIN2_bm) | (PIN1_bm);
	
	/* Set Period PWM dimana Servo menggunakan 20ms(50Hz) */
	TCC0.PER = 5000;

	/* Set Compare Register value default untuk 0 derajat */
	TCC0.CCA = 75;
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	pmic_init();
	gfx_mono_init();
	
	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);
	gfx_mono_draw_string("Sisnam+", 0, 0, &sysfont);

	/* Create the task */
	
	xTaskCreate(vBlinkLed0, "", 1000, NULL, tskIDLE_PRIORITY + 1, NULL);	// higher priority
	xTaskCreate(vBlinkLed1, "", 1000, NULL, tskIDLE_PRIORITY + 2, NULL);	// higher priority
	xTaskCreate(vPushButton1, "", 1000, NULL, tskIDLE_PRIORITY + 3, NULL);	// higher priority
	xTaskCreate(vCounter, "", 1000, NULL, tskIDLE_PRIORITY, NULL);			// low priority
	xTaskCreate(vServo, "", 1000, NULL, tskIDLE_PRIORITY + 4, NULL);        // highest priority
	
	/* Semaphore */
	xSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(xSemaphore);
	
	/* Start the task */
	
	vTaskStartScheduler();
}

static portTASK_FUNCTION(vBlinkLed0, p_) {
	char strbuf[128];
	int flagLed0 = 1;
	
	while(1) {
		flagLed0 = !flagLed0;
		ioport_set_pin_level(LED0_GPIO, flagLed0);
		snprintf(strbuf, sizeof(strbuf), "LED 0 : %d", !flagLed0);
		gfx_mono_draw_string(strbuf,0, 16, &sysfont);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vBlinkLed1, q_) {	
	char strbuf[128];
	int flagLed1 = 0;
	
	while(1) {
		flagLed1 = !flagLed1;
		ioport_set_pin_level(LED1_GPIO, flagLed1);
		snprintf(strbuf, sizeof(strbuf), "LED 1 : %d", !flagLed1);
		gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		vTaskDelay(375/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vPushButton1, s_) {
	char strbuf[128];
	
	while(1) {
		
		if(ioport_get_pin_level(GPIO_PUSH_BUTTON_1)==0){
			if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE) {
				counter++;
				snprintf(strbuf, sizeof(strbuf), "Counter : %d", counter);
				gfx_mono_draw_string(strbuf,0, 8, &sysfont);
				xSemaphoreGive(xSemaphore);
			}
		}
		
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(vCounter, r_) {
	char strbuf[128];
	
	while(1) {
		
		if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE) {
			counter++;
			snprintf(strbuf, sizeof(strbuf), "Counter : %d", counter);
			gfx_mono_draw_string(strbuf,0, 8, &sysfont);
			xSemaphoreGive(xSemaphore);	
		}
		
		vTaskDelay(100/portTICK_PERIOD_MS);
	}	
}

static portTASK_FUNCTION(vServo, t_) {
	PWM_Init();
	
	while(1){
		// Set Compare Register value default untuk 0 derajat
		TCC0.CCA = 75;
		
		vTaskDelay(100/portTICK_PERIOD_MS);
		
		// Set Compare Register value default untuk 180 derajat
		TCC0.CCA = 325;
		
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}