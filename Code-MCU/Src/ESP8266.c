#include "ESP8266.h"
#include "main.h"
#include "SVM30.h"
#include <math.h>
#include <string.h>
#include "stm32f1xx_hal_spi.h"

#define ESP_CMD_HEADER 0x69
#define ESP_START_SEND_DATA 0x01
#define ESP_REQUEST_SEND_DATA 0x02
#define ESP_RESPONE		0x70
#define ESP_RESEND_DATA		0x03
#define ESP_RESEND_COMMAND		0x04




extern int receive_data_spi;
uint8_t data_recieve_buff[4];
uint8_t buffer[4];
uint8_t Rx_indx = 1; 

extern uint8_t trang_thai;
extern IWDG_HandleTypeDef hiwdg;

uint8_t buffer_data_send[14];


/*
   Cmd Struct Message:
   __________________________________________________________________________________
  | HEADER    |   CMD     | PARAM LEN  |   DATA  |   DATA    |  DATA  | .. |CHECKSUM |
  |___________|___________|____________|_________|___________|________|____|_________|
  |   8 bit   |   8 bit   | 8 bit      |  8bit   |   8bit    | nbytes | .. |   8bit  |
  |___________|___________|____________|_________|___________|________|____|_________|
*/
//gui lenh theo cu phap giong hpm, 1 byte mo dau + 1 byte do dai + 1 byte lenh  + data + checksum

//extern SPI_HandleTypeDef hspi2; //esp
extern UART_HandleTypeDef huart3;
//spi 2
uint8_t ESP_CalculateChecksum(uint8_t *buffer, int length)
{
  uint16_t sum = 0;
  int i;

  for (i = 0; i < length; i++)
    sum += buffer[i];
	uint8_t kq;
	kq = (65536 - sum) % 256;
  return kq;
}



static unsigned int ESP_UartTx(uint8_t *buffer, uint16_t length)
{

	HAL_UART_Transmit(&huart3, buffer, length, 1000);

  return length;
}

//static unsigned int ESP_UartRx(uint8_t *buffer, int length)
//{
//  int errCode;

//  errCode = HAL_SPI_Receive(&hspi2, buffer, length, 500);
//	//ESP8266_ENABLE(ESP8266_CS_PIN_RESET);
//  return errCode == HAL_OK ? length : 0;
//}

static int ESP_SendCommand(uint8_t cmd, uint8_t *data, uint16_t dataLength)
{
  uint8_t buffer[1+1+1+256+1];

  /* Check the data parameters */
  if (!data && dataLength > 0)
    return HPM_ERR_BAD_ARG;

  /* Header */
  buffer[0] = ESP_CMD_HEADER;

  /* Length */
  buffer[1] =  dataLength;

  /* Command */
  buffer[2] = cmd;

  /* Data */
  if (dataLength > 0)
    memcpy(&buffer[3], data, dataLength);

  /* Checksum */
  buffer[3 + dataLength] = ESP_CalculateChecksum(buffer, 3 + dataLength);
	buffer[3 + dataLength +1 ] = 0x69;
  /* Return the length of data transmitted */
  return ESP_UartTx(buffer, 3 + dataLength + 2);
}
typedef enum
{
 ACK_POS,  //0
 ACK_NEG,   //1
 ACK_ERR,			//2
 ACK_BAD_CHECKSUM
} ESP_ACK_RESPONSE;


/*
//static ESP_ACK_RESPONSE ESP_GetSimpleAckResponse(void) //nhan phan hoi lai tu esp
//{
//  uint8_t ack[2];

//  if (ESP_UartRx(ack, 2) != 2)
//    return ACK_ERR;

//  if (ack[0] == 6 && ack[1] == 9)
//    return ACK_POS;

//  if (ack[0] == 9 && ack[1] == 9)
//    return ACK_NEG;

//  return ACK_ERR;
//}
*/



//kiem tra co ket noi dc voi esp hay khong
int ESP8266_Check_connection(void){  //kt ket noi voi esp sau khi khoi dong
	
	uint8_t command = ESP_RESPONE;
	int errCode;

  if ((errCode = ESP_SendCommand(command, NULL ,0)) < 0)
    return errCode;
	HAL_IWDG_Refresh(&hiwdg);
	HAL_Delay(500);
	if(receive_data_spi == 1)
	{
		//int kq = ESP8266_RECEIVE_DATA();
		receive_data_spi = 0;
		return 1;
	}
	else
		return 0;
}	


/*
  id_device: String,
    pm10: String,
    pm25: String,
		co2: String,	
    tvoc: String,
    temp: String,
    humd: String,
    o3: String,
    time: String
*/


void ESP8266_SEND_DATA(uint8_t *dust, uint8_t *envi_sensor, uint8_t *ozone)
{
	//pm10(2) - pm25(2) - tvoc(2) - co2(2) - nhietdo(2) -do am(2) - ozone(2) =14

  memcpy(&buffer_data_send[0], dust, 4);
	memcpy(&buffer_data_send[4], envi_sensor, 8);
	memcpy(&buffer_data_send[12], ozone, 2);


	ESP_SendCommand(ESP_START_SEND_DATA,buffer_data_send, 14 );
	HAL_Delay(200);
	
}
//nhan du lieu tu esp interup
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	if (huart->Instance == USART3)
  {
		__HAL_UART_CLEAR_FLAG(&huart3,UART_FLAG_RXNE);
		__HAL_UART_CLEAR_PEFLAG(&huart3);
		if (Rx_indx == 1) 
		{					
				for (int i = 0; i < 4; i++) {
							data_recieve_buff[i]=0; 
							buffer[i] = 0;
					}
		}	
		if(buffer[0] != 0x40){
				data_recieve_buff[Rx_indx++] = buffer[0];
		}
		/* Handle positive acknowledgement 0x40... */
		receive_data_spi = 1;
		if((trang_thai == 0)|| (trang_thai == 1)){
						trang_thai = 2;
						Rx_indx = 1;
		}
HAL_UART_Receive_IT	(&huart3, buffer,1);
	}
			

}


void Server_request_data(void){
	
	if(receive_data_spi == 1){
			
			ESP8266_RECEIVE_DATA();
			HAL_IWDG_Refresh(&hiwdg);
			receive_data_spi = 0;
			Rx_indx = 1;
	}
	else
		trang_thai = 0;
}

void ESP8266_RECEIVE_DATA(void){
	
	 //HAL_UART_Transmit(&huart3, data_recieve_buff,4, 100);
	 HAL_Delay(100);
	 HAL_IWDG_Refresh(&hiwdg);
	
					data_recieve_buff[0] = 0x40;
	uint8_t datalen = data_recieve_buff[1];
	uint8_t cmd = data_recieve_buff[2];
	
	uint8_t receivedChecksum = data_recieve_buff[3 + datalen];
  int expectedChecksum = ESP_CalculateChecksum(data_recieve_buff, 3 + datalen);

	if (receivedChecksum != expectedChecksum)
	{
			ESP_SendCommand(ESP_RESEND_COMMAND,NULL, 0 ); //send back to esp, request send command again 04
			trang_thai = 0;
	}
		else
	{
				switch(cmd)
				{
					case ESP_REQUEST_SEND_DATA: //02
					{
						trang_thai = 1;
						break;
					}
					case ESP_RESEND_DATA: //03
						resend_command();
						trang_thai = 0;
						break;

					default:
						trang_thai = 0;
						break;
				}
		}
}
void resend_command(void){
	ESP_SendCommand(ESP_START_SEND_DATA,buffer_data_send, 14 );
	HAL_Delay(100);
	
}
