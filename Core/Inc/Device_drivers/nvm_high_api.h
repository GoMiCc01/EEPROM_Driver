#pragma once

#include <stdint.h>
#include <nvm_low_api.h>
#include "nvm_handle.h"


typedef struct
{
	nvm_data_t data;
	uint8_t checksum;
} nvm_device_data_t;


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
