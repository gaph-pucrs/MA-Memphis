/**
 * 
 * @file dmni.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Defines the DMNI functions for payload handling.
 */

#include "dmni.h"

void dmni_read(hal_word_t *payload_address, uint16_t payload_size)
{
	*HAL_DMNI_SIZE = payload_size;
	*HAL_DMNI_OP = HAL_DMNI_WRITE;
	*HAL_DMNI_ADDRESS = (hal_word_t)payload_address;
	*HAL_DMNI_START = 1;
	while(*HAL_DMNI_RECEIVE_ACTIVE);
}
