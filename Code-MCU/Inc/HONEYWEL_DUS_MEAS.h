#include <stdint.h>


#define HPM_ERR_OK 0
#define HPM_ERR_GENERAL -1
#define HPM_ERR_BAD_ARG -2
#define HPM_ERR_NEG_ACK -3
#define HPM_ERR_NO_RESPONSE -4
#define HPM_ERR_BAD_RESPONSE -5
#define HPM_ERR_BAD_CHECKSUM -6



int hpmStopAutoSend(void);
int hpmStartParticleMeasurement(void);
int hpmStopParticleMeasurement(void);
int hpmReadResults(uint16_t *pm25concentration, uint16_t *pm10concentration);
int hpmStartParticleMeasurement(void);
int hpmStopParticleMeasurement(void);
void HPM_measure(uint8_t *dataout);


