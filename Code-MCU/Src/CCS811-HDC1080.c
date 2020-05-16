#include "CCS811-HDC1080.h"
#include "Led_indicator.h"
#include <string.h> 
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "stm32f1xx_hal_i2c.h"
#include "stm32f1xx_hal_flash_ex.h"

	 extern I2C_HandleTypeDef hi2c1;
   
	 uint8_t BurnIn_Time_Complete=0;
	 uint8_t RunIn_Time_Complete=0;
	 uint8_t Baseline_Time_Complete=0;
   uint8_t EBaseline_Time_Complete=0;
  	//These are the air quality values obtained from the sensor
	 uint16_t tVOC = 0;
	 uint16_t CO2 = 0;
	 uint16_t adc_raw=0;
	 uint8_t current_value=0;
	 uint8_t dummyread=0;
	 uint8_t appvalue=0;
	 uint8_t errvalue=0;
	 uint8_t mosetting=0;
	 uint8_t dtvalue =0;	 
	 uint8_t appStart=0;
   uint32_t ELBaseline_period=0;
	 uint32_t ALBaseline_period=0;
	 uint8_t  Mode_CCS811=1;
		
/*
  * @brief  Updates the total voltatile organic compounds (TVOC) in parts per billion (PPB) and the CO2 value (PPM).
  * @param  NONE.
  * @retval None.
 */	

void readAlgorithmResults()
{
uint8_t data_rq[4];
uint8_t status;
status = HAL_I2C_Mem_Read( &hi2c1, CCS811_ADDRD, ( uint8_t )CSS811_ALG_RESULT_DATA, I2C_MEMADD_SIZE_8BIT, data_rq, 4, 100 );
		
		uint8_t co2MSB = data_rq[0];
		uint8_t co2LSB = data_rq[1];
		uint8_t tvocMSB = data_rq[2];
		uint8_t tvocLSB = data_rq[3];
	 
	/*	TVOC value, in parts per billion (ppb)
		eC02 value, in parts per million (ppm) */
	if(status == 0)
	{
		CO2 = ((uint16_t)co2MSB << 8) | co2LSB;
		tVOC = ((uint16_t)tvocMSB << 8) | tvocLSB;
	}else{
		CO2 = 0;
		tVOC = 0;
	}
		
	
}

/*
  * @brief  configureCCS811.
  * @param  NONE.
  * @retval None.
 */
uint8_t configureCCS811(void)
{
		//softRest();	
		HAL_Delay(2000);
		//Verify the hardware ID is what we expect
		
		uint8_t hwID = readRegister(0x20); //Hardware ID should be 0x81
		if (hwID != 0x81)
		{
			//Serial.println("CCS811 not found. Please check wiring.");
			return 0;
		}
		//Check for app valid
		if (appValid() == 1)
		{
				return 0;
		}
		//start application
	  uint8_t    lodata[1];
	  lodata[0]= CSS811_APP_START;	
	  HAL_I2C_Master_Transmit(&hi2c1, CCS811_ADDRD, lodata, 1, 100); 
		HAL_Delay(100);
		//Check status error
		if (checkForError() == 1)
		{
				return 0;
		}	
	
		//Set Drive Mode
		setDriveMode(Mode_CCS811); //Read every second  
		HAL_Delay(10);	
		
		disableInterrupts();

	
		float tem_hcd=25, Humi_hdc = 65 ;
		hdc1080_start_measurement(&tem_hcd,&Humi_hdc);
		setEnvironmentalData( Humi_hdc, tem_hcd);
    //RunIn_Time_Complete= BURN_IN_TIME_ADDRESS;
    BurnIn_Time_Complete=BURN_IN_TIME_ADDRESS_VAL ;
		Baseline_Time_Complete=BASELINE_EARLYLIFE_PERIOD_ADDRESS_VAL;
		return 1;
}

	//Checks to see if error bit is set
	int checkForError()
	{
		errvalue=readRegister(CSS811_ERROR_ID);
	        errvalue = readRegister(CSS811_STATUS);
		return (errvalue & 1 << 0);
	}

	 

	//Checks to see if DATA_READ flag is set in the status register
	int dataAvailable()
	{   
	  dtvalue = readRegister(CSS811_STATUS);
		return (dtvalue & 1 << 3);
	}

	//Checks to see if APP_VALID flag is set in the status register
	int appValid()
	{
		appvalue = readRegister(CSS811_STATUS);
		return (appvalue & (1 << 4));
	}

	//Enable the nINT signal
	void enableInterrupts(void)
	{
		uint8_t setting = readRegister(CSS811_MEAS_MODE); //Read what's currently there
		setting |= (1 << 3); //Set INTERRUPT bit
		writeRegister(CSS811_MEAS_MODE, setting);
	}

	//Disable the nINT signal
	void disableInterrupts(void)
	{
		uint8_t setting = readRegister(CSS811_MEAS_MODE); //Read what's currently there
		setting &= ~(1 << 3); //Clear INTERRUPT bit
		writeRegister(CSS811_MEAS_MODE, setting);
	}

/*
  * @brief  //Mode 0 = Idle
	    //Mode 1 = read every 1s
	    //Mode 2 = every 10s
	    //Mode 3 = every 60s
	    //Mode 4 = RAW mode.
  * @param  MODE.
  * @retval None.
 */
void setDriveMode(uint8_t mode)
{
		if (mode > 4) mode = 4; //Error correction
	 
		mosetting = readRegister(CSS811_MEAS_MODE); //Read what's currently there
	 
		mosetting &=~(7<<4); //Clear DRIVE_MODE bits
		mosetting |= (mode << 4); //Mask in mode
	 
		writeRegister(CSS811_MEAS_MODE, mosetting);
		mosetting = readRegister(CSS811_MEAS_MODE); //Read what's currently there
	 
}

 
 /*
  * @brief //Given a temp and humidity, write this data to the CSS811 for better compensation
	 //This function expects the humidity and temp to come in as floats
  * @param  relativeHumidity HUMIDITY.
  * @param  temperature TEMPERATURE.
  * @retval None.
 */
void setEnvironmentalData(float relativeHumidity, float temperature)
{
		int rH = relativeHumidity * 1000; //42.348 becomes 42348
		int temp = temperature * 1000; //23.2 becomes 23200

		uint8_t envData[4];

		//Split value into 7-bit integer and 9-bit fractional
		envData[0] = ((rH % 1000) / 100) > 7 ? (rH / 1000 + 1) << 1 : (rH / 1000) << 1;
		envData[1] = 0; //CCS811 only supports increments of 0.5 so bits 7-0 will always be zero
		if (((rH % 1000) / 100) > 2 && (((rH % 1000) / 100) < 8))
		{
			envData[0] |= 1; //Set 9th bit of fractional to indicate 0.5%
		}

		temp += 25000; //Add the 25C offset
		//Split value into 7-bit integer and 9-bit fractional
		envData[2] = ((temp % 1000) / 100) > 7 ? (temp / 1000 + 1) << 1 : (temp / 1000) << 1;
		envData[3] = 0;
		if (((temp % 1000) / 100) > 2 && (((temp % 1000) / 100) < 8))
		{
			envData[2] |= 1;  //Set 9th bit of fractional to indicate 0.5C
		}

		uint8_t env[6];
		env[0]=CSS811_ENV_DATA;
		env[1]=envData[0];
		env[2]=envData[1];
		env[3]=envData[2];
		env[5]=envData[3];
		HAL_I2C_Mem_Write( &hi2c1, CCS811_ADDRD, ( uint8_t )CSS811_ENV_DATA, I2C_MEMADD_SIZE_8BIT, env, 4,
																100 );
//		 while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			} 
//			while (HAL_I2C_IsDeviceReady(&hi2c1, CCS811_ADDRD, 10, 300) == HAL_TIMEOUT);
//			while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			}
		 
	}


 /*
  * @brief  reading_Multiple_Register
  * @param  addr ADDRESS.
  * @param  val VALUE.
  * @param  size SIZE.
  * @retval None.
 */
void read_Mul_Register(uint8_t addr, uint8_t * val,uint8_t size)
	{
			HAL_I2C_Mem_Read( &hi2c1, CCS811_ADDRD, ( uint8_t )addr, I2C_MEMADD_SIZE_8BIT, val, size,100 );
		
	}

 /*
  * @brief  softRest
  * @param  NONE.
  * @retval None.
 */
void softRest() {
     
          uint8_t rstCMD[5] = {CSS811_SW_RESET, 0x11,0xE5,0x72,0x8A};
 
         	HAL_I2C_Mem_Write( &hi2c1, CCS811_ADDRD, CSS811_SW_RESET, I2C_MEMADD_SIZE_8BIT, rstCMD, 5,300);
	//	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
	      
}	


 /*
  * @brief  get_Sensor_Resistance
  * @param  NONE.
  * @retval RESISTANCE OF SENSOR.
 */
uint32_t get_Sensor_Resistance(){
	uint8_t sensor_Resistance_raw[2];

	uint32_t sensor_Resistance;
	
	HAL_I2C_Mem_Read( &hi2c1, CCS811_ADDRD, CSS811_RAW_DATA , I2C_MEMADD_SIZE_8BIT, sensor_Resistance_raw, 2,100 );
        current_value=sensor_Resistance_raw[0]>>2;
	sensor_Resistance_raw[0]=sensor_Resistance_raw[0]&0x03;
	
	adc_raw=(sensor_Resistance_raw[0]<<8)|sensor_Resistance_raw[1];
	
	sensor_Resistance=((165*adc_raw)*10000)/(current_value*1023);
 
	return sensor_Resistance;
}

 /*
  * @brief  sleep
  * @param  NONE.
  * @retval NONE.
 */
void sleep()
{
  // sets sensor to idle; measurements are disabled; lowest power mode
 writeRegister(CSS811_MEAS_MODE, 0);
 }



 /*
  * @brief  Reads from a give location from the CSS811
  * @param  addr  ADDRESS.
  * @retval VALUE AT THE ADDRESS.
 */	 
uint8_t readRegister(uint8_t addr)
{
			uint8_t dt;
					
			 HAL_I2C_Mem_Read( &hi2c1, CCS811_ADDRD, ( uint8_t )addr,1, &dt, 1,
																	300 );
			//	 while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
		 
		 
			return dt;
 }
		
		 
		 
uint32_t  restore_Baselines;
		 
/*
  * @brief  	 //restore the baseline value
	       //Used for telling sensor what 'clean' air is
	      //You must put the sensor in clean air and record this value
  * @param  NONE.
  * @retval NONE.
 */
void restore_Baseline()
  {   
       uint32_t  restore_Baseline;
		 restore_Baseline= * ((  uint32_t *)DATA_EEPROM_BASE);
		
		restore_Baselines=restore_Baseline ;
		uint8_t res_bs[2];
		res_bs[0]=restore_Baseline>>8;
		res_bs[1]=restore_Baseline&0x000000FF;
    	        HAL_I2C_Mem_Write( &hi2c1, CCS811_ADDRD, CSS811_BASELINE, I2C_MEMADD_SIZE_8BIT, res_bs,2,300);
//		 while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			} 
    }

 
/*
  * @brief  	//Returns the baseline value and saves into EEPROM
	       //Used for telling sensor what 'clean' air is
	      //You must put the sensor in clean air and record this value
  * @param  NONE.
  * @retval BASELINE VALUE.
 */
unsigned int getBaseline()
{
		 
  	uint8_t ada[2];
		HAL_StatusTypeDef status = HAL_OK;
	 
		  status = HAL_I2C_Mem_Read( &hi2c1, CCS811_ADDRD, ( uint8_t )CSS811_BASELINE,1,  ada, 2,100 );
//			 while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			} 
//			while (HAL_I2C_IsDeviceReady(&hi2c1, CCS811_ADDRD, 10, 300) == HAL_TIMEOUT);
//			while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			}

		uint8_t baselineMSB = ada[0];
		uint8_t baselineLSB = ada[1];

		unsigned int baseline = ((unsigned int)baselineMSB << 8) | baselineLSB;
		//HAL_FLASHEx_DATAEEPROM_Unlock();
	 // HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, DATA_EEPROM_BASE, baseline);
			
		return (baseline);
}



	//Write a value to a spot in the CCS811
/*
  * @brief  Write a value to a spot in the CCS811
  * @param  addr ADDRESS.
  * @param  val  VALUE.
  * @retval NONE.
 */
void writeRegister(uint8_t addr, uint8_t val)
{
			

			HAL_I2C_Mem_Write( &hi2c1, CCS811_ADDRD, ( uint8_t )addr, I2C_MEMADD_SIZE_8BIT, &val, 1,300);
//		 while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			} 
//			while (HAL_I2C_IsDeviceReady(&hi2c1, CCS811_ADDRD, 10, 300) == HAL_TIMEOUT);
//			while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//			{
//			}
}
/*-----------------------------------------------------------------------------*/

void hdc1080_init(Temp_Reso Temperature_Resolution_x_bit,int Humidity_Resolution_x_bit)
{
	/* Temperature and Humidity are acquired in sequence, Temperature first
	 * Default:   Temperature resolution = 14 bit,
	 *            Humidity resolution = 14 bit
	 */

	/* Set the acquisition mode to measure both temperature and humidity by setting Bit[12] to 1 */
	uint16_t config_reg_value=0x1000;
	uint8_t data_send[2];

	if(Temperature_Resolution_x_bit == Temperature_Resolution_11_bit)
	{
		config_reg_value |= (1<<10); //11 bit
	}

	switch(Humidity_Resolution_x_bit)
	{
	case Humidity_Resolution_11_bit:
		config_reg_value|= (1<<8);
		break;
	case Humidity_Resolution_8_bit:
		config_reg_value|= (1<<9);
		break;
	}

	data_send[0]= (config_reg_value>>8);
	data_send[1]= (config_reg_value&0x00ff);

	HAL_I2C_Mem_Write(&hi2c1,HDC_1080_ADD<<1,Configuration_register_add,I2C_MEMADD_SIZE_8BIT,data_send,2,1000);
}


uint8_t hdc1080_start_measurement(float* temperature, float* humidity)
{
	uint8_t receive_data[4];
	uint16_t temp_x,humi_x;
	uint8_t send_data = Temperature_register_add;

	HAL_I2C_Master_Transmit(&hi2c1,HDC_1080_ADD<<1,&send_data,1,1000);

	/* Delay here 15ms for conversion compelete.
	 * Note: datasheet say maximum is 7ms, but when delay=7ms, the read value is not correct
	 */
	HAL_Delay(15);

	/* Read temperature and humidity */
	HAL_I2C_Master_Receive(&hi2c1,HDC_1080_ADD<<1,receive_data,4,1000);

	temp_x =((receive_data[0]<<8)|receive_data[1]);
	humi_x =((receive_data[2]<<8)|receive_data[3]);

	*temperature=((temp_x/65536.0)*165.0)-41.0;
	*humidity=  ((humi_x/65536.0)*100.0);

	return 0;

}
