#include <stdint.h>



#define ESP8266_Enable 1
#define ESP8266_Disable 0
#define ESP8266_CS_PIN_SET 1
#define ESP8266_CS_PIN_RESET 0

#define ESP8266_ENABLE_Pin 				GPIO_PIN_12
#define ESP8266_ENABLE_GPIO_Port 	GPIOB

int ESP8266_Check_connection(void);

void ESP8266_SEND_DATA(uint8_t *dust, uint8_t *envi_sensor,uint8_t *ozone);
void ESP8266_RECEIVE_DATA(void);
void Server_request_data(void);
void resend_command(void);
