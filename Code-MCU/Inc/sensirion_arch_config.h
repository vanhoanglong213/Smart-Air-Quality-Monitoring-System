#ifndef SENSIRION_ARCH_CONFIG_H
#define SENSIRION_ARCH_CONFIG_H

/**
 * If your platform does not provide the library stdlib.h you have to remove the
 * include and define NULL yourself (see below).
 */
#include <stdlib.h>

/**
 * #ifndef NULL
 * #define NULL ((void *)0)
 * #endif
 */

/**
 * If your platform does not provide the library stdint.h you have to
 * define the integral types yourself (see below).
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Typedef section for types commonly defined in <stdint.h>
 * If your system does not provide stdint headers, please define them
 * accordingly. Please make sure to define int64_t and uint64_t.
 */
/* typedef unsigned long long int uint64_t;
 * typedef long long int int64_t;
 * typedef long int32_t;
 * typedef unsigned long uint32_t;
 * typedef short int16_t;
 * typedef unsigned short uint16_t;
 * typedef char int8_t;
 * typedef unsigned char uint8_t; */

/* Types not typically provided by <stdint.h> */
typedef float float32_t;

/**
 * Define the endianness of your architecture:
 * 0: little endian, 1: big endian
 * Use the following code to determine if unsure:
 * ```c
 * #include <stdio.h>
 *
 * int is_big_endian(void) {
 *     union {
 *         unsigned int u;
 *         char c[sizeof(unsigned int)];
 *     } e = { 0 };
 *     e.c[0] = 1;
 *
 *     return (e.i != 1);
 * }
 *
 * int main(void) {
 *     printf("Use #define SENSIRION_BIG_ENDIAN %d\n", is_big_endian());
 *
 *     return 0;
 * }
 * ```
 */
#define SENSIRION_BIG_ENDIAN 0

/**
 * The clock period of the i2c bus in microseconds. Increase this, if your GPIO
 * ports cannot support a 200 kHz output rate. (2 * 1 / 10usec == 200Khz)
 *
 * This is only relevant for the sw-i2c HAL (bit-banging on GPIO pins). The
 * pulse length is half the clock period, the number should thus be even.
 */
#define SENSIRION_I2C_CLOCK_PERIOD_USEC 10

#ifdef __cplusplus
}
#endif

#endif /* SENSIRION_ARCH_CONFIG_H */
