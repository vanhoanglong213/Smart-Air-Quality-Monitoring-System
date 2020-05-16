#include <stdio.h>
#include <stdint.h>

#define MQ131_DEFAULT_TEMPERATURE_CELSIUS           25                // Default temperature to correct environmental drift
#define MQ131_DEFAULT_HUMIDITY_PERCENT              65                // Default humidity to correct environmental drift
#define MQ131_DEFAULT_LO_CONCENTRATION_R0           110470.60         // Default R0 for low concentration MQ131
#define MQ131_DEFAULT_RL                            10000   



uint16_t read_adc_value(void);
uint16_t getO3(int unit);
