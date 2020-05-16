#include "SVM30.h"
#include "Led_indicator.h"
#include <string.h> 
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/* Extern variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
static const uint8_t SGP30_I2C_ADDRESS = 0x58;
static const uint8_t SHTC1_ADDRESS = 0x70;
static const uint16_t SHTC3_CMD_WAKEUP = (uint16_t)13591U;
static const uint16_t SHTC1_CMD_DURATION_USEC = (uint16_t)10U;
static uint16_t shtc1_cmd_measure = SHTC1_CMD_MEASURE_HPM;


const char *SGP_DRV_VERSION_STR;
/* Private variables ---------------------------------------------------------*/
//uint8_t _Receive_BUF[40];   			// buffers
//uint8_t _Send_BUF[10];      			// 2 command + max 6 data
//uint8_t _Receive_BUF_Length;
//uint8_t _I2C_address;       				// I2C address to use (SGP30 or SHTC1)
//uint8_t 	_Send_BUF_Length = 0;								
//uint8_t	_Receive_BUF_Length = 0;			
//int _SVM30_Debug = 0;										// program debug level
//int _started = 0;										// indicate the SGP30 measurement has started
//uint16_t _wait;        						// wait time after sending command
//struct svm_values _svm_value;
/////////////////////////////////////////////////////////////////////////
int8_t sensirion_i2c_write(uint8_t address, const uint8_t *data,
                           uint16_t count) {
    return (int8_t)HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(address << 1),
                                           (uint8_t *)data, count, 100);
}
int8_t sensirion_i2c_read(uint8_t address, uint8_t *data, uint16_t count) {
    return (int8_t)HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(address << 1),
                                          data, count, 100);
}

uint8_t sensirion_common_generate_crc(uint8_t *data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}
int8_t sensirion_common_check_crc(uint8_t *data, uint16_t count,
                                  uint8_t checksum) {
    if (sensirion_common_generate_crc(data, count) != checksum)
        return STATUS_FAIL;
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////
uint16_t sensirion_fill_cmd_send_buf(uint8_t *buf, uint16_t cmd,
                                     const uint16_t *args, uint8_t num_args) {
    uint8_t crc;
    uint8_t i;
    uint16_t idx = 0;

    buf[idx++] = (uint8_t)((cmd & 0xFF00) >> 8);
    buf[idx++] = (uint8_t)((cmd & 0x00FF) >> 0);

    for (i = 0; i < num_args; ++i) {
        buf[idx++] = (uint8_t)((args[i] & 0xFF00) >> 8);
        buf[idx++] = (uint8_t)((args[i] & 0x00FF) >> 0);

        crc = sensirion_common_generate_crc((uint8_t *)&buf[idx - 2],
                                            SENSIRION_WORD_SIZE);
        buf[idx++] = crc;
    }
    return idx;
}


int16_t sensirion_i2c_write_cmd_with_args(uint8_t address, uint16_t command,
                                          const uint16_t *data_words,
                                          uint16_t num_words) {
    uint8_t buf[SENSIRION_MAX_BUFFER_WORDS];
    uint16_t buf_size;

    buf_size = sensirion_fill_cmd_send_buf(buf, command, data_words, num_words);
    return sensirion_i2c_write(address, buf, buf_size);
}
																					


int16_t sgp30_set_absolute_humidity(uint32_t absolute_humidity) {
    int16_t ret;
    uint16_t ah_scaled;

    if (absolute_humidity > 256000)
        return STATUS_FAIL;

    /* ah_scaled = (absolute_humidity / 1000) * 256 */
    ah_scaled = (uint16_t)((absolute_humidity * 16777) >> 16);

    ret = sensirion_i2c_write_cmd_with_args(
        SGP30_I2C_ADDRESS, SGP30_CMD_SET_ABSOLUTE_HUMIDITY, &ah_scaled,
        SENSIRION_NUM_WORDS(ah_scaled));

    //sensirion_sleep_usec(SGP30_CMD_SET_ABSOLUTE_HUMIDITY_DURATION_US);

    return ret;
}



int16_t sensirion_i2c_read_bytes(uint8_t address, uint8_t *data,
                                 uint16_t num_words) {
    int16_t ret;
    uint16_t i, j;
    uint16_t size = num_words * (SENSIRION_WORD_SIZE + CRC8_LEN);
    uint16_t word_buf[SENSIRION_MAX_BUFFER_WORDS];
    uint8_t *const buf8 = (uint8_t *)word_buf;

    ret = sensirion_i2c_read(address, buf8, size);
    if (ret != STATUS_OK)
        return ret;

    /* check the CRC for each word */
    for (i = 0, j = 0; i < size; i += SENSIRION_WORD_SIZE + CRC8_LEN) {

        ret = sensirion_common_check_crc(&buf8[i], SENSIRION_WORD_SIZE,
                                         buf8[i + SENSIRION_WORD_SIZE]);
        if (ret != STATUS_OK)
            return ret;

        data[j++] = buf8[i];
        data[j++] = buf8[i + 1];
    }

    return STATUS_OK;
}


int16_t sensirion_i2c_read_words(uint8_t address, uint16_t *data_words,
                                 uint16_t num_words) {
    int16_t ret;
    uint8_t i;

    ret = sensirion_i2c_read_bytes(address, (uint8_t *)data_words, num_words);
    if (ret != STATUS_OK)
        return ret;

    for (i = 0; i < num_words; ++i)
        data_words[i] = be16_to_cpu(data_words[i]);

    return STATUS_OK;
}


int16_t sensirion_i2c_write_cmd(uint8_t address, uint16_t command) {
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_fill_cmd_send_buf(buf, command, NULL, 0);
    return sensirion_i2c_write(address, buf, SENSIRION_COMMAND_SIZE);
}



int16_t sgp30_measure_iaq() {
    return sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_IAQ_MEASURE);
}



int16_t sgp30_read_iaq(uint16_t *tvoc_ppb, uint16_t *co2_eq_ppm) {
    int16_t ret;
    uint16_t words[SGP30_CMD_IAQ_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGP30_I2C_ADDRESS, words,
                                   SGP30_CMD_IAQ_MEASURE_WORDS);

    *tvoc_ppb = words[1];
    *co2_eq_ppm = words[0];

    return ret;
}

int16_t sgp30_measure_iaq_blocking_read(uint16_t *tvoc_ppb,
                                        uint16_t *co2_eq_ppm) {
    int16_t ret;

    ret = sgp30_measure_iaq();
    if (ret != STATUS_OK)
        return ret;

			HAL_Delay(500);

    return sgp30_read_iaq(tvoc_ppb, co2_eq_ppm);
}

int16_t sgp30_measure_raw() {
    return sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_RAW_MEASURE);
}

int16_t sgp30_read_raw(uint16_t *ethanol_raw_signal, uint16_t *h2_raw_signal) {
    int16_t ret;
    uint16_t words[SGP30_CMD_RAW_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGP30_I2C_ADDRESS, words,
                                   SGP30_CMD_RAW_MEASURE_WORDS);

    *ethanol_raw_signal = words[1];
    *h2_raw_signal = words[0];

    return ret;
}


int16_t sgp30_measure_raw_blocking_read(uint16_t *ethanol_raw_signal,
                                        uint16_t *h2_raw_signal) {
    int16_t ret;

    ret = sgp30_measure_raw();
    if (ret != STATUS_OK)
        return ret;

    		HAL_Delay(300);

    return sgp30_read_raw(ethanol_raw_signal, h2_raw_signal);
}

int16_t sensirion_i2c_delayed_read_cmd(uint8_t address, uint16_t cmd,
                                       uint32_t delay_us, uint16_t *data_words,
                                       uint16_t num_words) {
    int16_t ret;
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_fill_cmd_send_buf(buf, cmd, NULL, 0);
    ret = sensirion_i2c_write(address, buf, SENSIRION_COMMAND_SIZE);
    if (ret != STATUS_OK)
        return ret;

   //if (delay_us)
       HAL_Delay(100);

    return sensirion_i2c_read_words(address, data_words, num_words);
}																			


int16_t sgp30_get_feature_set_version(uint16_t *feature_set_version,
                                      uint8_t *product_type) {
    int16_t ret;
    uint16_t words[SGP30_CMD_GET_FEATURESET_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(SGP30_I2C_ADDRESS,
                                         SGP30_CMD_GET_FEATURESET,
                                         SGP30_CMD_GET_FEATURESET_DURATION_US,
                                         words, SGP30_CMD_GET_FEATURESET_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *feature_set_version = words[0] & 0x00FF;
    *product_type = (uint8_t)((words[0] & 0xF000) >> 12);

    return STATUS_OK;
}

static int16_t sgp30_check_featureset(uint16_t needed_fs) {
    int16_t ret;
    uint16_t fs_version;
    uint8_t product_type;

    ret = sgp30_get_feature_set_version(&fs_version, &product_type);
    if (ret != STATUS_OK)
        return ret;

    if (product_type != SGP30_PRODUCT_TYPE)
        return SGP30_ERR_INVALID_PRODUCT_TYPE;

    if (fs_version < needed_fs)
        return SGP30_ERR_UNSUPPORTED_FEATURE_SET;

    return STATUS_OK;
}

int16_t sgp30_iaq_init(void) {
    int16_t ret =
        sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_IAQ_INIT);
    	HAL_Delay(20);
    return ret;
}


int16_t sgp30_probe() {
    int16_t ret = sgp30_check_featureset(0x20);

    if (ret != STATUS_OK)
        return ret;

    return sgp30_iaq_init();
}



/* ----------------------------------------------------------------------------------*/

int16_t shtc1_wake_up() {
    return sensirion_i2c_write_cmd(SHTC1_ADDRESS, SHTC3_CMD_WAKEUP);
}

int16_t shtc1_read_serial(uint32_t *serial) {
    int16_t ret;
    const uint16_t tx_words[] = {0x007B};
    uint16_t serial_words[SENSIRION_NUM_WORDS(*serial)];

    ret = sensirion_i2c_write_cmd_with_args(SHTC1_ADDRESS, 0xC595, tx_words,
                                            SENSIRION_NUM_WORDS(tx_words));
    if (ret)
        return ret;

    HAL_Delay(20);

    ret = sensirion_i2c_delayed_read_cmd(
        SHTC1_ADDRESS, 0xC7F7, SHTC1_CMD_DURATION_USEC, &serial_words[0], 1);
    if (ret)
        return ret;

    ret = sensirion_i2c_delayed_read_cmd(
        SHTC1_ADDRESS, 0xC7F7, SHTC1_CMD_DURATION_USEC, &serial_words[1], 1);
    if (ret)
        return ret;

    *serial = ((uint32_t)serial_words[0] << 16) | serial_words[1];
    return ret;
}



int16_t shtc1_measure(void) {
    return sensirion_i2c_write_cmd(SHTC1_ADDRESS, shtc1_cmd_measure);
}


int16_t shtc1_read(int32_t *temperature, int32_t *humidity) {
    uint16_t words[2];
    int16_t ret = sensirion_i2c_read_words(SHTC1_ADDRESS, words,
                                           SENSIRION_NUM_WORDS(words));
    /**
     * formulas for conversion of the sensor signals, optimized for fixed point
     * algebra:
     * Temperature = 175 * S_T / 2^16 - 45
     * Relative Humidity = 100 * S_RH / 2^16
     */
    *temperature = ((21875 * (int32_t)words[0]) >> 13) - 45000;
    *humidity = ((12500 * (int32_t)words[1]) >> 13);

    return ret;
}

int16_t shtc1_probe(void) {
    uint32_t serial;

    (void)shtc1_wake_up(); /* Try to wake up the sensor, ignore return value */
    return shtc1_read_serial(&serial);
}

int16_t shtc1_measure_blocking_read(int32_t *temperature, int32_t *humidity) {
    int16_t ret;

    ret = shtc1_measure();
    if (ret)
        return ret;
#if !defined(USE_SENSIRION_CLOCK_STRETCHING) || !USE_SENSIRION_CLOCK_STRETCHING
    HAL_Delay(100);

#endif /* USE_SENSIRION_CLOCK_STRETCHING */
    return shtc1_read(temperature, humidity);
}

/* ----------------------------------------------------------------------------------*/
#define T_LO (-20000)
#define T_HI 70000

static const uint32_t AH_LUT_100RH[] = {1078,  2364,  4849,  9383,   17243,
                                        30264, 50983, 82785, 130048, 198277};
static const uint32_t T_STEP = (T_HI - T_LO) / (ARRAY_SIZE(AH_LUT_100RH) - 1);

static void svm_compensate_rht(int32_t *temperature, int32_t *humidity) {
    *temperature = ((*temperature * 8225) >> 13) - 500;
    *humidity = (*humidity * 8397) >> 13;
}

/**
 * Convert relative humidity [%RH*1000] and temperature [mC] to
 * absolute humidity [mg/m^3]
 */
static uint32_t sensirion_calc_absolute_humidity(const int32_t *temperature,
                                                 const int32_t *humidity) {
    uint32_t t, i, rem, ret;

    if (*humidity <= 0)
        return 0;

    if (*temperature < T_LO)
        t = 0;
    else
        t = (uint32_t)(*temperature - T_LO);

    i = t / T_STEP;
    rem = t % T_STEP;

    if (i >= ARRAY_SIZE(AH_LUT_100RH) - 1) {
        ret = AH_LUT_100RH[ARRAY_SIZE(AH_LUT_100RH) - 1];

    } else if (rem == 0) {
        ret = AH_LUT_100RH[i];

    } else {
        ret = (AH_LUT_100RH[i] +
               ((AH_LUT_100RH[i + 1] - AH_LUT_100RH[i]) * rem / T_STEP));
    }

    // Code is mathematically (but not numerically) equivalent to
    //    return (ret * (*humidity)) / 100000;
    // Maximum ret = 198277 (Or last entry from AH_LUT_100RH)
    // Maximum *humidity = 119000 (theoretical maximum)
    // Multiplication might overflow with a maximum of 3 digits
    // Trick: ((ret >> 3) * (uint32_t)(*humidity)) does never overflow
    // Now we only need to divide by 12500, as the tripple righ shift
    // divides by 8

    return ((ret >> 3) * (uint32_t)(*humidity)) / 12500;
}

static int16_t svm_set_humidity(const int32_t *temperature,
                                const int32_t *humidity) {
    uint32_t absolute_humidity;

    absolute_humidity = sensirion_calc_absolute_humidity(temperature, humidity);

    if (absolute_humidity == 0)
        absolute_humidity = 1; /* avoid disabling humidity compensation */

    return sgp30_set_absolute_humidity(absolute_humidity);
}

const char *svm_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}


int16_t svm_measure_iaq_blocking_read(uint16_t *tvoc_ppb, uint16_t *co2_eq_ppm,
                                      int32_t *temperature, int32_t *humidity) {
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = svm_set_humidity(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    svm_compensate_rht(temperature, humidity);

    err = sgp30_measure_iaq_blocking_read(tvoc_ppb, co2_eq_ppm);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t svm_measure_raw_blocking_read(uint16_t *ethanol_raw_signal,
                                      uint16_t *h2_raw_signal,
                                      int32_t *temperature, int32_t *humidity) {
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = svm_set_humidity(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = sgp30_measure_raw_blocking_read(ethanol_raw_signal, h2_raw_signal);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t svm_probe() {
    int16_t err;

    err = shtc1_probe();
    if (err != STATUS_OK)
        return err;

    return sgp30_probe();
}
