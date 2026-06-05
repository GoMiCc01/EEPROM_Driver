#pragma once

#include <stdint.h>
#include "stm32f4xx_hal.h"

/**
 * 	enum for status returns
 */
typedef enum
{
    NVM_API_STATUS_OK = 0,
    NVM_API_STATUS_NOT_INITIALIZED,
    NVM_API_STATUS_WRITE_ERROR,
    NVM_API_STATUS_READ_ERROR,
	NVM_API_STATUS_GET_DATA_ERROR,
	NVM_API_STATUS_NO_DATA,
	NVM_API_STATUS_CORRUPTED_MEMORY,
	NVM_API_STATUS_INVALID_PARAMETERS
} nvm_api_status_t;

typedef struct
{
	I2C_HandleTypeDef *hi2c;
	uint8_t device_address;
	uint8_t device_page_size;
	uint16_t device_mem_capacity;
} nvm_device_api_handle;


