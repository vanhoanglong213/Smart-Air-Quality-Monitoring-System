#include <stdio.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"


#define CYAN 	0				//XANH DA TROI B - G
#define GREEN 1
#define BLUE	2
#define YELLOW 3			//R - G
#define PURPLE 4			//R -B
#define RED		5

#define PORT_RGB_LED GPIOC
#define RGB_RED			 GPIO_PIN_2
#define RGB_BLUE		 GPIO_PIN_0
#define RGB_GREEN		 GPIO_PIN_1

#define PORT_LED 			GPIOC
#define LED					 GPIO_PIN_4




void LED_the_result_ok(void);
void LED_the_result_no_good(void);
void LED_turn_off(void);
void LED_turn_on(void);



void LED_turn_ON_RGB(uint8_t color);
void LED_turn_OFF_RGB(uint8_t color);
void LED2_turn_on(void);
void LED2_turn_off(void);
