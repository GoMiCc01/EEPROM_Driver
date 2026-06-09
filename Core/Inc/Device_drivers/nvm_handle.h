#pragma once

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef struct
{
	I2C_HandleTypeDef *hi2c;
	uint8_t device_address;
	uint8_t device_mem_page;
	uint16_t last_busy_struct_address;
	uint16_t device_mem_capacity;
	uint8_t is_device_initialized;
} nvm_device_api_handle;
