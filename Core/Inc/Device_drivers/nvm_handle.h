#pragma once

#include <stdint.h>
#include "stm32f4xx_hal.h"

/**
 * 	enum for status returns
 */
typedef enum
{
	// init
	NVM_API_STATUS_OK = 0,
	NVM_API_STATUS_NOT_INITIALIZED,
	// write
	NVM_API_STATUS_WRITE_ERROR,
	// read
	NVM_API_STATUS_READ_ERROR,
	NVM_API_STATUS_NO_DATA,
	NVM_API_STATUS_CORRUPTED_DATA,
	// general
	NVM_API_STATUS_INVALID_PARAMETERS,
} nvm_high_api_status_t;

typedef struct
{
	I2C_HandleTypeDef *hi2c;
	uint8_t device_address;
	uint8_t device_mem_page;
	uint16_t last_busy_struct_address;
	uint16_t device_mem_capacity;
	nvm_high_api_status_t initializing_status;
} nvm_device_api_handle;
