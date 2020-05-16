#ifndef SVM30_H
#define SVM30_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


#define STATUS_OK 0
#define STATUS_FAIL (-1)

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF
#define CRC8_LEN 1

#define SENSIRION_COMMAND_SIZE 2
#define SENSIRION_WORD_SIZE 2
#define SENSIRION_NUM_WORDS(x) (sizeof(x) / SENSIRION_WORD_SIZE)
#define SENSIRION_MAX_BUFFER_WORDS 32


#define SGP30_CMD_RAW_MEASURE 0x2050
#define SGP30_CMD_SET_ABSOLUTE_HUMIDITY 0x2061
#define SGP30_CMD_IAQ_MEASURE 0x2008
#define SGP30_CMD_IAQ_MEASURE_WORDS 2

/* command and constants for raw measure */
#define SGP30_CMD_RAW_MEASURE 0x2050
#define SGP30_CMD_RAW_MEASURE_WORDS 2

/* command and constants for IAQ init */
#define SGP30_CMD_IAQ_INIT 0x2003

#define SGP30_ERR_UNSUPPORTED_FEATURE_SET (-10)
#define SGP30_ERR_INVALID_PRODUCT_TYPE (-12)
#define SGP30_PRODUCT_TYPE 0

/* command and constants for reading the featureset version */
#define SGP30_CMD_GET_FEATURESET 0x202f
#define SGP30_CMD_GET_FEATURESET_DURATION_US 10
#define SGP30_CMD_GET_FEATURESET_WORDS 1

#define be16_to_cpu(s) (((uint16_t)(s) << 8) | (0xff & ((uint16_t)(s)) >> 8))

#define SHTC1_MEASUREMENT_DURATION_USEC 14
#define SHTC1_CMD_MEASURE_HPM 0x7866

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif


/**
 * svm_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char *svm_get_driver_version(void);

/**
 * svm_probe() - check if an SVM30 module is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp30_iaq_init())
 *
 * Return:  STATUS_OK on success.
 */
int16_t svm_probe(void);

/**
 * svm_measure_iaq_blocking_read() - Measure IAQ concentrations tVOC, CO2-Eq.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 * @co2_eq_ppm: The CO2-Equivalent ppm value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
int16_t svm_measure_iaq_blocking_read(uint16_t *tvoc_ppb, uint16_t *co2_eq_ppm,
                                      int32_t *temperature, int32_t *humidity);

/**
 * svm_measure_raw_blocking_read() - Measure raw signals
 *
 * The output values are written to the memory locations passed as parameters:
 * @ethanol_raw_signal: The ethanol signal
 * @h2_raw_signal:      The h2 signal
 * @temperature:        Temperature in [degree Celsius] multiplied by 1000
 * @humidity:           Relative humidity in [%RH (0..100)] multiplied by 1000
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
int16_t svm_measure_raw_blocking_read(uint16_t *ethanol_raw_signal,
                                      uint16_t *h2_raw_signal,
                                      int32_t *temperature, int32_t *humidity);


int16_t sgp30_measure_iaq_blocking_read(uint16_t *tvoc_ppb,
                                        uint16_t *co2_eq_ppm) ;

int16_t sgp30_iaq_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SVM30_H */
