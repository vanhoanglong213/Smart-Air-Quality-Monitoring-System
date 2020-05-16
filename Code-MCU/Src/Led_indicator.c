#include "Led_indicator.h"

/* Private variables ---------------------------------------------------------*/


/* -----------------------------------------------------------------------------*/


//bat led rgb
void LED_turn_ON_RGB(uint8_t color)
{
	switch(color)
	{
		case 0:{ //xanh da troi
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_SET);
			HAL_Delay(10);	
			break;
		}
		case 1:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_SET);
			HAL_Delay(10);	
			break;
		}
		case 2:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_SET);
			HAL_Delay(10);	
			break;
		}
		case 3:{ //vang
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_RED, GPIO_PIN_SET);
			HAL_Delay(10);	
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_SET);
			HAL_Delay(10);	
			break;
		}
		case 4:{ //purple
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_RED, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_SET);
			HAL_Delay(10);			
			break;
		}		
			case 5:{ //RED
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_RED, GPIO_PIN_SET);
			HAL_Delay(10);			
			break;
				}
	}		
}
//tat rbg

void LED_turn_OFF_RGB(uint8_t color)
{
	switch(color)
	{
		case 0:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_RESET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_RESET);
			HAL_Delay(10);	
			break;
		}
		case 1:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_RESET);
			HAL_Delay(10);	
			break;
		}
		case 2:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_RESET);
			HAL_Delay(10);	
			break;
		}
		case 3:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_RED, GPIO_PIN_RESET);
			HAL_Delay(10);	
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_GREEN, GPIO_PIN_RESET);
			HAL_Delay(10);	
			break;
		}
		case 4:{
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_RED, GPIO_PIN_RESET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(PORT_RGB_LED,RGB_BLUE, GPIO_PIN_RESET);
			HAL_Delay(10);			
			break;
		}		
	}		
}


//bat led do - k ket noi dc
void LED_the_result_ok(void)
{
	HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_RESET);
	HAL_Delay(800);
	HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_SET);
}
		

//bat led do - ket noi ok - sang 1 lan
void LED_the_result_no_good(void)
{
	for(int i = 0; i < 5; i++){
		HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_SET);
		HAL_Delay(200);
	}	
	
}

void LED_turn_on(void){
	HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_RESET);
	HAL_Delay(20);
}
void LED_turn_off(void){
	HAL_GPIO_WritePin(PORT_LED,LED, GPIO_PIN_SET);
	HAL_Delay(20);
}

void LED2_turn_on(void){
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_Delay(20);
}
void LED2_turn_off(void){
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2, GPIO_PIN_SET);
	HAL_Delay(20);
}
