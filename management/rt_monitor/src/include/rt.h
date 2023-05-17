/**
 * MA-Memphis
 * @file rt.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Real-time observer functions
 */

#pragma once

#include <memphis/oda.h>

/**
 * @brief Verifies for a missed deadline in the observed task
 * 
 * @details
 * Forwards to the decider if deadline is missed
 * 
 * @param decider Pointer to the decider structure
 * @param id ID of the observed task
 * @param rt_diff Difference between slack time and remaining execution time
 */
void rt_check(oda_t *decider, int id, int rt_diff);
