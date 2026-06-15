#pragma once

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef enum
{
	// init
	NVM_API_STATUS_NOT_INITIALIZED = 99,
	NVM_API_STATUS_OK = 0,
	// write
	NVM_API_STATUS_WRITE_ERROR,
	// read
	NVM_API_STATUS_READ_ERROR,
	NVM_API_STATUS_NO_DATA,
	NVM_API_STATUS_CORRUPTED_DATA,
	// general
	NVM_API_STATUS_INVALID_PARAMETERS,
} nvm_high_api_status_t;

// Opaque pointer
typedef struct nvm_device_api_handle_s nvm_device_api_handle;
nvm_device_api_handle *createEntity(I2C_HandleTypeDef *hi2c, uint8_t device_address);

typedef struct
{
	uint8_t data;
} nvm_data_t;

typedef enum
{
	NVM_API_STATUS_DO_NOT_FORMAT = 0,
	NVM_API_STATUS_FORMAT,
} nvm_formatting_status_t;

typedef struct
{
	nvm_high_api_status_t (*init)(nvm_device_api_handle *const wl_handle, const nvm_formatting_status_t format);
	nvm_high_api_status_t (*read)(nvm_device_api_handle *const wl_handle, nvm_data_t *data);
	nvm_high_api_status_t (*write)(nvm_device_api_handle *const wl_handle, const nvm_data_t *const data);
} nvm_api_t;

extern nvm_api_t api;
