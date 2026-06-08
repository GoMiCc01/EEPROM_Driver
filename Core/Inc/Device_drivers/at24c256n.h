#pragma once

#include "nvm_api.h"

#define MEMADD_SIZE I2C_MEMADD_SIZE_16BIT
#define TIMEOUT 10

typedef struct
{
	nvm_device_status_t (*init)  (nvm_device_api_handle *const wl_handle);
	nvm_device_status_t (*read)  (nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*last_busy_struct_address) (nvm_device_api_handle *const wl_handle, const uint16_t size);
	nvm_device_status_t (*write) (nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*erase_all) (nvm_device_api_handle *const wl_handle);
} nvm_device_api_t;




