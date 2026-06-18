#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "nvm_high_api.h"

/**
 * 	enum for status returns
 */

typedef struct
{
	nvm_data_t data;
	uint8_t checksum;
} nvm_device_data_t;

typedef struct
{
	nvm_data_t data;
	bool is_valid;
} nvm_data_cache;

struct nvm_device_api_handle_s
{
	I2C_HandleTypeDef *hi2c;
	uint8_t device_address;
	uint8_t device_mem_page;
	uint16_t last_busy_struct_address;
	uint16_t device_mem_capacity;
	nvm_high_api_status_t initializing_status;
	nvm_data_cache data_cache;
};
