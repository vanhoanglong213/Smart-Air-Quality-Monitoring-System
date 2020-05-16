#include "Check_on_Startup.h"
#include "SVM30.h"
#include "HONEYWEL_DUS_MEAS.h"
#include "ESP8266.h"
#include "Ozone.h"
#include "CCS811-HDC1080.h"


extern SPI_HandleTypeDef hspi2; //esp
//extern struct svm_values _svm_value;
uint8_t flag_task_five_min = 0;

int  USE_MODUL_CCS811 = 1;
int USE_MODUL_SVM  =  	0;

extern uint8_t trang_thai;
extern IWDG_HandleTypeDef hiwdg;



void check_on_startup(void)
{

//1 - check esp	
	LED_turn_ON_RGB(GREEN);
	HAL_Delay(50);
	int kq = ESP8266_Check_connection();
	HAL_Delay(200);
	if(kq == 1)
			LED_the_result_ok();
	else
			LED_the_result_no_good();
	LED_turn_OFF_RGB(GREEN);
	HAL_Delay(1000);
	HAL_IWDG_Refresh(&hiwdg);
	
	
//2 - check pm25 and pm10
	LED_turn_ON_RGB(BLUE);
	HAL_Delay(50);
	int result_hpm =  hpmStopAutoSend();
	hpmStopParticleMeasurement();
	HAL_Delay(200);
	if(result_hpm == 0)
			LED_the_result_ok();
	else
			LED_the_result_no_good();
	LED_turn_OFF_RGB(BLUE);
	HAL_Delay(1000);
	HAL_IWDG_Refresh(&hiwdg);
	
//3 - CHECK ozone3	
		LED_turn_ON_RGB(PURPLE);
		HAL_Delay(50);

		LED_the_result_ok();
		LED_turn_OFF_RGB(PURPLE);
		HAL_Delay(1000);
		HAL_IWDG_Refresh(&hiwdg);
	
//4 - CHECK VOC	
		LED_turn_ON_RGB(YELLOW);
		HAL_Delay(50);
		uint8_t kq_tvoc;
	  if(USE_MODUL_CCS811 == 1)
		{
			hdc1080_init(0,0);
			kq_tvoc = configureCCS811();
		}
		if(USE_MODUL_SVM == 1)
		{
			kq_tvoc = svm_probe();
			HAL_Delay(500);
		}
		if(kq_tvoc) 
			LED_the_result_ok();
		else
			LED_the_result_no_good();
		LED_turn_OFF_RGB(YELLOW);
		HAL_Delay(1000);
		HAL_IWDG_Refresh(&hiwdg);
}	


union uint8_to_uint16
{
	uint8_t uint8_data[2];
	uint16_t uint16_data;
}one_to_for_data;

uint16_t save_all_data[7][12];
extern uint16_t tVOC, CO2;
int col = 0, row = 0, flag_reset_svm = 0; //row < 8; col <=12

/*
  id_device: String,
    co2: String,
    o3: String,
    tvoc: String,
    pm10: String,
    pm25: String,
    temp: String,
    humd: String
    time: String
*/


void task_every_5_min(void){
	uint8_t data_dust[4];
	uint8_t data_tvoc[8];
	uint8_t data_ozone[2];
//	 struct svm_values v;
	//uint8_t data_ozone[2];
	// mang bang 0
	for(int i = 0; i < 8; i ++){
		 if(i <2)
				{
					 data_dust[i] = 0;
					 data_tvoc[i] = 0;
					data_ozone[i] = 0;
				}
			else if((i >= 2)&& (i < 4)){
						data_dust[i] = 0;
						data_tvoc[i] = 0;
				}
			else
					data_tvoc[i] = 0;
	}
/*-------------------------------------------------------------------------------------*/	
			//do bui				
		HPM_measure(data_dust);
		
		one_to_for_data.uint8_data[0] =data_dust[0] ;
		one_to_for_data.uint8_data[1] =data_dust[1] ;
		save_all_data[0][col] = one_to_for_data.uint16_data ;
	
		one_to_for_data.uint8_data[0] =data_dust[2] ;
		one_to_for_data.uint8_data[1] =data_dust[3] ;
		save_all_data[1][col] = one_to_for_data.uint16_data ;
	//	one_to_for_data.uint8_data[0] = 0 ;
			HAL_IWDG_Refresh(&hiwdg);
/*-------------------------------------------------------------------------------------*/			
			
			//do tvoc
			//HAL_Delay(100);
			HAL_IWDG_Refresh(&hiwdg);
			uint16_t tvoc1=0, co21=0;			
			int32_t temp =0, humi  = 0;
			float tem_hd = 0;
			float	hum_hd= 0;
			if(USE_MODUL_CCS811 == 1)
			{
			//	if( dataAvailable() == 1)		
					//	configureCCS811();
					readAlgorithmResults();
				
						tvoc1 = tVOC;
						co21 = CO2;
						hdc1080_start_measurement(&tem_hd,&hum_hd);
						temp = (int32_t)tem_hd*1000;
						humi = (int32_t)hum_hd*1000;
						HAL_Delay(1000);
			}
			else if(USE_MODUL_SVM == 1)
			{
					svm_measure_iaq_blocking_read(&tvoc1,&co21, &temp, &humi);
					flag_reset_svm++;
					if(flag_reset_svm % 300 == 299)
					{
							svm_probe();
							HAL_Delay(100);
							flag_reset_svm = 0;
					}
			}
	// co2
		one_to_for_data.uint16_data = co21;
		save_all_data[2][col] = one_to_for_data.uint16_data  ;
	data_tvoc[0] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[1] =one_to_for_data.uint8_data[1]; 
	//tvoc
		one_to_for_data.uint16_data =  tvoc1; 
		save_all_data[3][col] = one_to_for_data.uint16_data ;
	data_tvoc[2] =   one_to_for_data.uint8_data[0];
	data_tvoc[3] =  one_to_for_data.uint8_data[1]; 
	//temp
		one_to_for_data.uint16_data = (uint16_t) temp/1000; 
		save_all_data[4][col] = one_to_for_data.uint16_data ;
	data_tvoc[4] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[5] = one_to_for_data.uint8_data[1]; 
	//humi
		one_to_for_data.uint16_data = (uint16_t) humi/1000;
	save_all_data[5][col] = one_to_for_data.uint16_data ;
	data_tvoc[6] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[7] = one_to_for_data.uint8_data[1]; 
/*-------------------------------------------------------------------------------------*/		
			//do ozone
			
			save_all_data[6][col] = getO3(1);
			//save_all_data[6][col] =  read_adc_value();	
			one_to_for_data.uint16_data = save_all_data[6][col];				
			data_ozone[0] = one_to_for_data.uint8_data[0];
			data_ozone[1] = one_to_for_data.uint8_data[1];
			HAL_Delay(200);
			HAL_IWDG_Refresh(&hiwdg);
			
/*-------------------------------------------------------------------------------------*/			

		col++;
		if(col > 11)
			col = 0;
			
			//gui den esp
			ESP8266_SEND_DATA(data_dust, data_tvoc, data_ozone);
		//kt xem truyen dc hay khong, neu khong gui lai	
		
			HAL_IWDG_Refresh(&hiwdg);
			
//xong task chuyen sang trang thai idle					
}

void send_data_to_esp(void){
	
	HAL_Delay(100);
	
	uint8_t data_dust[4];
	uint8_t data_tvoc[8];
	uint8_t data_ozone[2];
	
	uint16_t total_temp = 0, total_each[7];
 for(int j = 0; j < 7; j ++)
	{
		for(int i = 0; i < 12; i++){
				 total_temp += save_all_data[j][i];			 
		}
	  total_each[j] = total_temp/12;
		total_temp = 0;
	}
 
	//dust 10
	one_to_for_data.uint16_data =  total_each[0] ;
	data_dust[0] = one_to_for_data.uint8_data[0]  ;
	data_dust[1] = one_to_for_data.uint8_data[1]  ;
	//dust 25
		one_to_for_data.uint16_data =  total_each[1] ;
	data_dust[2] = one_to_for_data.uint8_data[0]  ;
	data_dust[3] = one_to_for_data.uint8_data[1]  ;
	// co2
		one_to_for_data.uint16_data =  total_each[2] ;
	data_tvoc[0] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[1] = one_to_for_data.uint8_data[1]; 
	//tvoc
		one_to_for_data.uint16_data =  total_each[3] ;
	data_tvoc[2] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[3] = one_to_for_data.uint8_data[1]; 
	//temp
		one_to_for_data.uint16_data =  total_each[4] ;
	data_tvoc[4] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[5] = one_to_for_data.uint8_data[1]; 
	//humi
		one_to_for_data.uint16_data =  total_each[5] ;
	data_tvoc[6] = one_to_for_data.uint8_data[0]  ;
	data_tvoc[7] = one_to_for_data.uint8_data[1]; 
	//ozone
		one_to_for_data.uint16_data =  total_each[6] ;
	data_ozone[0] = one_to_for_data.uint8_data[0]  ;
	data_ozone[1] = one_to_for_data.uint8_data[1]; 
	
	ESP8266_SEND_DATA(data_dust, data_tvoc, data_ozone);
	HAL_IWDG_Refresh(&hiwdg);
}

