#include "Ozone.h"
#include <stddef.h>
#include <string.h>
#include <stm32f1xx_hal.h>
#include <math.h>
 

extern ADC_HandleTypeDef hadc1;
extern IWDG_HandleTypeDef hiwdg;

int temperatureCelsuis = MQ131_DEFAULT_TEMPERATURE_CELSIUS;
int humidityPercent = MQ131_DEFAULT_HUMIDITY_PERCENT;



/*

22:36:02.117 -> Calibration done!
22:36:02.117 -> R0 = 125500.00 Ohms
22:36:02.117 -> Time to heat = 693 s
22:36:02.117 -> Sampling...
22:47:35.960 -> Concentration O3 : 0.01 ppm
22:47:36.007 -> Concentration O3 : 11.54 ppb
22:47:36.007 -> Concentration O3 : 0.02 mg/m3
22:47:36.007 -> Concentration O3 : 24.40 ug/m3
22:48:36.075 -> Sampling..
*/

uint16_t read_adc_value(void){
	HAL_ADC_Start(&hadc1);
	HAL_Delay(200);
	uint16_t value_read=0;
	uint16_t	temp_total = 0, temp_read = 0;
	for(int i = 0; i < 10; i ++)
	{
		temp_read= HAL_ADC_GetValue(&hadc1);
		temp_total += temp_read;
		HAL_Delay(10);
	}
	value_read = temp_total/10;
	temp_total = 0;
	HAL_IWDG_Refresh(&hiwdg);
	HAL_ADC_Stop(&hadc1);
	return value_read;
}
enum MQ131Unit {PPM, PPB, MG_M3, UG_M3};

 float convert(float input, int unitIn, int unitOut) {
  if(unitIn == unitOut) {
    return input;
  }

  float concentration = 0;

  switch(unitOut) {
    case PPM :
      // We assume that the unit IN is PPB as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPB to PPM
      return input / 1000.0;
    case PPB :
      // We assume that the unit IN is PPM as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPM to PPB
      return input * 1000.0;
    case MG_M3 :
      if(unitIn == PPM) {
        concentration = input;
      } else {
        concentration = input / 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    case UG_M3 :
      if(unitIn == PPB) {
        concentration = input;
      } else {
        concentration = input * 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    default :
      return input;
  }
}

 
 
float readRs() {
 	// Read the value
 	uint16_t valueSensor = read_adc_value()-550;
	//if(valueSensor < 0)
	//		valueSensor = 50;
 	// Compute the voltage on load resistance (for 5V Arduino)
	//3.3 / 4096 = 0.8mV / 1 don vi
 	float vRL = ((float)valueSensor)/ 4095 * 5;
 	// Compute the resistance of the sensor (for 5V Arduino)
 	float rS =( (5.0 / vRL )- 1.0) * 2500;//dien tro tren board, chinh la 2k5
 	return rS;
 }
 

 float getEnvCorrectRatio() {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio
 	if(humidityPercent == 60 && temperatureCelsuis == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
 		// R^2 = 0.9986
 		return -0.0141 * temperatureCelsuis + 1.5623;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
 	// R^2 = 0.996
 	return -0.0103 * temperatureCelsuis + 1.1507;
 }

 /**
 * Get gas concentration for O3 in ppm
 */
 
 uint16_t getO3(int unit) {
 	// If no value Rs read, return 0.0
	 float  lastValueRs = readRs();
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

  float ratio = 0.0;

 			// Use the equation to compute the O3 concentration in ppm
 			// R^2 = 0.9987
      // Compute the ratio Rs/R0 and apply the environmental correction
  ratio = lastValueRs / 125500 * 0.9986;// getEnvCorrectRatio();
      //return convert(9.4783 * pow(ratio, 2.3348), 1, unit);
	float kq = 9.4783 * pow(ratio, 2.3348);
	uint16_t kq_int = (uint16_t)kq;
	return kq_int;
	
  }

 
