/**
 * MA-Memphis
 * @file oda.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Standard function for OD(A) tasks
 */

#pragma once

#include <stdbool.h>

#define ODA_USER	0x01
#define ODA_ACT		0x02
#define ODA_DECIDE	0x04
#define ODA_OBSERVE 0x08

#define O_QOS		0x0100

#define D_QOS		0x010000

#define A_MIGRATION	0x01000000

typedef struct _oda {
	int id;
	int tag;
} oda_t;

/**
 * @brief Initializes a ODA
 * 
 * @param oda Pointer to the ODA
 */
void oda_init(oda_t *oda);

/**
 * @brief Requests an ODA service
 * 
 * @param oda Pointer to the ODA
 * @param type_tag Flags of the desired ODA capabilities
 */
void oda_request_service(oda_t *oda, int type_tag);

/**
 * @brief Verifies if the ODA is enabled
 * 
 * @details A ODA is enabled if it has a valid ID
 * 
 * @param oda Pointer to the ODA
 * 
 * @return True if enabled
 */
bool oda_is_enabled(oda_t *oda);

/**
 * @brief Handles a SERVICE_PROVIDER message
 * 
 * @param oda Pointer to the ODA
 * @param type_tag Task type tag of the service
 * @param id ID of the task that provides the service
 * 
 * @return True if the tag is correct and the actuator ID is set
 */
bool oda_service_provider(oda_t *oda, int type_tag, int id);
