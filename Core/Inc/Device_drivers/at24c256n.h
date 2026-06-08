#pragma once

#include "nvm_api.h"

typedef struct
{
	nvm_device_status_t (*init)  (nvm_device_api_handle *const wl_handle);
	nvm_device_status_t (*read)  (nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*search_last_busy_page) (nvm_device_api_handle *const wl_handle, const uint16_t size);
	nvm_device_status_t (*write) (nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*erase_all) (nvm_device_api_handle *const wl_handle);
} nvm_device_api_t;




