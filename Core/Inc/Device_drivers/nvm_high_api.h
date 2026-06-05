#pragma once

#include <stdint.h>
#include "at24c256n.h"
#include "nvm_api.h"

/**
 * 	stores pointers to low-level functions
 */

typedef struct {
	uint8_t data;
} nvm_data_t;

typedef enum
{
	NVM_API_STATUS_DO_NOT_FORMAT = 0,
	NVM_API_STATUS_FORMAT,
} nvm_formatting_status_t;

typedef struct
{
	nvm_high_api_status_t (*init)  (nvm_device_api_handle *const wl_handle, nvm_formatting_status_t format);
	nvm_high_api_status_t (*read)  (nvm_device_api_handle *const wl_handle, nvm_data_t *data);
	nvm_high_api_status_t (*write) (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data);
} nvm_api_t;


extern nvm_api_t api;

/*
 init:
 1) Verify format
    format:
    1) erase: busy_page = 0x0000
 2) call init_low
 3) call search last busy page
 4) filling structure necessary fields */
