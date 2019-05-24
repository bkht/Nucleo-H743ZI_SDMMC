#ifndef __DMC_FIFO_H
#define __DMC_FIFO_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"

#define PAYLOAD_SIZE	16
#define FIFO_SIZE	4

struct entry {
	uint8_t data[PAYLOAD_SIZE];
};

struct fifo {
	uint8_t head;
	uint8_t tail;
	uint8_t count;
	struct entry entry[FIFO_SIZE];
};

extern struct fifo rf_tx_fifo;
extern struct fifo rf_rx_fifo;

uint8_t *fifo_get_head(struct fifo *fifo);
void fifo_push(struct fifo *fifo);
uint8_t *fifo_get_tail(struct fifo *fifo);
void fifo_pop(struct fifo *fifo);
uint8_t fifo_full(struct fifo *fifo);
uint8_t fifo_count(struct fifo *fifo);
void fifo_clear(struct fifo *fifo);


#ifdef __cplusplus
}
#endif
#endif /* __DMC_FIFO_H */
