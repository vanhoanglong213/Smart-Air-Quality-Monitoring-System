#include "HONEYWEL_DUS_MEAS.h"
#include <stddef.h>
#include <string.h>
#include <stm32f1xx_hal.h>
#include "Led_indicator.h"

//uart 1
extern UART_HandleTypeDef huart1;
#define HPM_CMD_HEADER 0x68
#define HPM_READ_RESULTS_CMD 0x04
#define HPM_START_MEASURE_CMD 0x01
#define HPM_STOP_MEASURE_CMD 0x02
#define HPM_SET_COEFF_CMD 0x08
#define HPM_READ_COEFF_CMD 0x10
#define HPM_STOP_AUTO_SEND_CMD 0x20
#define HPM_ENABLE_AUTO_SEND_CMD 0x40

#define DEFAULT_RX_TIMEOUT_MS 500
#define DEFAULT_TX_TIMEOUT_MS HAL_MAX_DELAY

typedef enum
{
 ACK_POS,
 ACK_NEG,
 ACK_ERR,
 ACK_BAD_CHECKSUM
} AckResponse;

static uint8_t hpmCalculateChecksum(uint8_t *buffer, int length)
{
  uint16_t sum = 0;
  int i;

  for (i = 0; i < length; i++)
    sum += buffer[i];

  return (65536U - sum) % 256;
}
static unsigned int hpmUartTx(uint8_t *buffer, int length)
{
  HAL_UART_Transmit(&huart1, buffer, length, DEFAULT_TX_TIMEOUT_MS);

  return length;
}
static unsigned int hpmUartRx(uint8_t *buffer, int length)
{
  int errCode;

  errCode = HAL_UART_Receive(&huart1, buffer, length, DEFAULT_RX_TIMEOUT_MS);

  return errCode == HAL_OK ? length : 0;
}


static int hpmSendCommand(uint8_t cmd, uint8_t *data, uint8_t dataLength)
{
  uint8_t buffer[1+1+1+256+1];

  /* Check the data parameters */
  if (!data && dataLength > 0)
    return HPM_ERR_BAD_ARG;

  /* Header */
  buffer[0] = HPM_CMD_HEADER;

  /* Length */
  buffer[1] = 1 + dataLength;

  /* Command */
  buffer[2] = cmd;

  /* Data */
  if (dataLength > 0)
    memcpy(&buffer[3], data, dataLength);

  /* Checksum */
  buffer[3 + dataLength] = hpmCalculateChecksum(buffer, 3 + dataLength);

  /* Return the length of data transmitted */
  return hpmUartTx(buffer, 3 + dataLength + 1);
}


static AckResponse hpmGetSimpleAckResponse(void)
{
  uint8_t ack[2];

  if (hpmUartRx(ack, 2) != 2)
    return ACK_ERR;

  if (ack[0] == 0xa5 && ack[1] == 0xa5)
    return ACK_POS;

  if (ack[0] == 0x96 && ack[1] == 0x96)
    return ACK_NEG;

  return ACK_ERR;
}


static AckResponse hpmGetComplexAckResponse(
    uint8_t *payloadBuffer, int maxPayloadLength,
    uint8_t *cmd, int *actualPayloadLength)
{
  uint8_t response[16];
  unsigned int respLength;
  int receivedChecksum, expectedChecksum;

  /* There should be at least 2 bytes in the response */
  if (hpmUartRx(response, 2) != 2)
    return ACK_ERR;

  /* Handle negative acknowledgement 0x9696 */
  if (response[0] == 0x96 && response[1] == 0x96)
    return ACK_NEG;

  /* Handle positive acknowledgement 0x40... */
  if (response[0] == 0x40)
  {
    /* The response length has already been read */
    respLength = response[1];

    /* The payload length is 1 less */
    *actualPayloadLength = respLength - 1;

    /* Bail out if the amount of data to read is anormal or too
     * much for the payload buffer*/
    if (respLength >= sizeof(response) - 2 ||
        *actualPayloadLength > maxPayloadLength)
      return ACK_ERR;

    /* Read the rest of the message, respLength + 1 bytes */
    if (hpmUartRx(&response[2], respLength + 1) != respLength + 1)
      return ACK_ERR;

    /* The command code */
    *cmd = response[2];

    /* The data payload */
    memcpy(payloadBuffer, &response[3], *actualPayloadLength);

    /* Verify the checksum */
    receivedChecksum = response[2 + respLength];
    expectedChecksum = hpmCalculateChecksum(response, 2 + respLength);
    if (receivedChecksum != expectedChecksum)
      return ACK_BAD_CHECKSUM;

    return ACK_POS;
  }
  else
  {
    return ACK_ERR;
  }
}



int hpmReadResults(uint16_t *pm25concentration, uint16_t *pm10concentration)
{
  int errCode;
  uint8_t data[8];
  uint8_t cmdInAck;
  int dataLength;
	unsigned int _pm10=0, _pm25=0;
  if ((errCode = hpmSendCommand(HPM_READ_RESULTS_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetComplexAckResponse(data, sizeof(data), &cmdInAck, &dataLength))
  {
    case ACK_POS:
      // Verify the command and data length are correct in the response 
      if (cmdInAck != HPM_READ_RESULTS_CMD || dataLength != 4)
        return ACK_ERR;

      _pm25 = data[0] * 256 +  data[1];
      _pm10 = data[2] * 256 +  data[3];
			
			*pm25concentration = _pm25;
			*pm10concentration = _pm10;
			
      return HPM_ERR_OK;
//      break;
    case ACK_NEG:
      return HPM_ERR_NEG_ACK;
   //   break;
    case ACK_BAD_CHECKSUM:
      return HPM_ERR_BAD_CHECKSUM;
   //   break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
//	
/*
	if(HAL_UART_Receive(&huart1,data,8,1000)!=HAL_OK)
 {
			_pm25 = 0;
			_pm10 = 0;
			*pm25concentration = _pm25;
			*pm10concentration = _pm10;
 }
 else
	 {
	//	HAL_UART_Receive(&huart1,data,8,1000);
		_pm25  = data[3] * 256 + data[4];
		_pm10 = data[5] * 256 + data[6];
 
			*pm25concentration = _pm25;
			*pm10concentration = _pm10;
		}
		*/
 return 1;
}


int hpmStartParticleMeasurement(void)
{
//		uint8_t meas_start_data[]={0x68,0x01,0x01,0x96};
// while(HAL_UART_GetState(&huart1)!= HAL_UART_STATE_READY);
// if( HAL_UART_Transmit(&huart1, meas_start_data, 4,500)!= HAL_OK)
// {
//	 while(1);
// }
//	 uint8_t ackn[2];
// if(HAL_UART_Receive(&huart1,ackn,2,500)!=HAL_OK)
// {
//	 while(1);
// }
  int errCode;

  if ((errCode = hpmSendCommand(HPM_START_MEASURE_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetSimpleAckResponse())
  {
  case ACK_POS: 
			return HPM_ERR_OK; 
			//break;
  case ACK_NEG: 
			return HPM_ERR_NEG_ACK; 
			//break;
  case ACK_ERR:
  default:
    return HPM_ERR_BAD_RESPONSE;
  }

}


int hpmStopParticleMeasurement(void)
{
  int errCode;

  if ((errCode = hpmSendCommand(HPM_STOP_MEASURE_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetSimpleAckResponse())
  {
    case ACK_POS: return HPM_ERR_OK; //break;
    case ACK_NEG: return HPM_ERR_NEG_ACK; //break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
}


int hpmSetAdjustmentCooeff(int coeff)
{
  int errCode;
  uint8_t data = coeff;

  if ((errCode = hpmSendCommand(HPM_SET_COEFF_CMD, &data, sizeof(data))) < 0)
    return errCode;

  switch(hpmGetSimpleAckResponse())
  {
    case ACK_POS: return HPM_ERR_OK;// break;
    case ACK_NEG: return HPM_ERR_NEG_ACK;// break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
}


int hpmReadAdjustmentCooeff(int *coeff)
{
  int errCode;
  uint8_t data[1];
  uint8_t cmdInAck;
  int dataLength;

  if ((errCode = hpmSendCommand(HPM_READ_COEFF_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetComplexAckResponse(data, sizeof(data), &cmdInAck, &dataLength))
  {
    case ACK_POS:
      /* Verify the command and data length are correct in the response */
      if (cmdInAck != HPM_READ_COEFF_CMD || dataLength != 1)
        return ACK_ERR;
      *coeff = data[0];
      return HPM_ERR_OK;
      //break;
    case ACK_NEG:
      return HPM_ERR_NEG_ACK;
     // break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
}


int hpmStopAutoSend(void)
{
  int errCode;

  if ((errCode = hpmSendCommand(HPM_STOP_AUTO_SEND_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetSimpleAckResponse())
  {
    case ACK_POS: 
					return HPM_ERR_OK; 
				//	break;
    case ACK_NEG: 
					return HPM_ERR_NEG_ACK; 
				//	break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
}


int hpmEnableAutoSend(void)
{
  int errCode;

  if ((errCode = hpmSendCommand(HPM_ENABLE_AUTO_SEND_CMD, NULL, 0)) < 0)
    return errCode;

  switch(hpmGetSimpleAckResponse())
  {
    case ACK_POS: 
				return HPM_ERR_OK; 
				//break;
    case ACK_NEG: 
				return HPM_ERR_NEG_ACK; 
				//break;
    case ACK_ERR:
    default:
      return HPM_ERR_BAD_RESPONSE;
  }
}


void HPM_measure(uint8_t *dataout){
		uint16_t pm10=0, pm25=0;
		uint8_t dataout_arr[4];
	
	
		union uint8_to_uint16
		{
			uint8_t uint8_data[2];
			uint16_t uint16_data;
		}one_to_for_data;
		
		hpmStartParticleMeasurement();
		HAL_Delay(5000);
		hpmReadResults(&pm25,&pm10);
		HAL_Delay(100);
		hpmStopParticleMeasurement();
		
		one_to_for_data.uint16_data = pm10;
		dataout_arr[0] = one_to_for_data.uint8_data[0];
		dataout_arr[1] = one_to_for_data.uint8_data[1];
		
		one_to_for_data.uint16_data = pm25;
		dataout_arr[2] = one_to_for_data.uint8_data[0];
		dataout_arr[3] = one_to_for_data.uint8_data[1];
		
		memcpy(dataout,dataout_arr,4);
		
}












