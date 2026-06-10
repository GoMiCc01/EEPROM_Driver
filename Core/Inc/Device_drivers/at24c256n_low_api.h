#pragma once

#include "nvm_handle.h"

/**
 * @brief Status codes returned by NVM device operations.
 */
typedef enum
{
    NVM_DEVICE_STATUS_OK = 0, /** Operation completed successfully. */
    NVM_DEVICE_STATUS_NOT_CONNECTED,
	// read_low
    NVM_DEVICE_STATUS_READ_ERROR,
	// write_low & erase_low
	NVM_DEVICE_STATUS_WRITE_ERROR,
} nvm_device_status_t;

typedef struct
{
	nvm_device_status_t (*init)  (nvm_device_api_handle *const wl_handle);
	nvm_device_status_t (*read)  (nvm_device_api_handle *const wl_handle, const uint16_t mem_address, uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*write) (nvm_device_api_handle *const wl_handle, const uint16_t mem_address, const uint8_t *const data, const uint16_t size);
	nvm_device_status_t (*erase_all) (nvm_device_api_handle *const wl_handle);
} nvm_device_api_t;
