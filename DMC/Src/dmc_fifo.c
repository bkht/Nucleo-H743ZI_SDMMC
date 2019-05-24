/*
 * Copyright 2012 Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include "dmc_fifo.h"

struct fifo rf_tx_fifo;
struct fifo rf_rx_fifo;

uint8_t *fifo_get_head(struct fifo *fifo)
{
	if (fifo->count < FIFO_SIZE)
		return fifo->entry[fifo->head].data;
	else
		return NULL;
}

void fifo_push(struct fifo *fifo)
{
	fifo->head = (fifo->head + 1) % FIFO_SIZE;
	fifo->count++;
}

uint8_t *fifo_get_tail(struct fifo *fifo)
{
	if (fifo->count)
		return fifo->entry[fifo->tail].data;
	else
		return NULL;
}

void fifo_pop(struct fifo *fifo)
{
	fifo->tail = (fifo->tail + 1) % FIFO_SIZE;
	fifo->count--;
}

uint8_t fifo_full(struct fifo *fifo)
{
	return (fifo->count >= FIFO_SIZE);
}

uint8_t fifo_count(struct fifo *fifo)
{
	return fifo->count;
}

void fifo_clear(struct fifo *fifo)
{
	fifo->head = 0;
	fifo->tail = 0;
	fifo->count = 0;
}
