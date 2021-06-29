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

#include <oda.h>

void rt_check(oda_t *decider, int id, int waiting_msg, int slack_time, int remaining_time);
