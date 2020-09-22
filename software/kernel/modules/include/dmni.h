/**
 * 
 * @file dmni.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Declares the DMNI functions for payload handling.
 */

#pragma once

/**
 * @brief Abstracts the DMNI programming for read data from NoC and copy to memory.
 * 
 * @param payload_address	Address where the payload will be saved
 * @param payload_size		Number of bytes to copy
 */
void dmni_read(unsigned int *payload_address, unsigned short payload_size);
