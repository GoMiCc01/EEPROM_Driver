#pragma once

#include <stdint.h>
#include "at24c256n_low_api.h"
#include "nvm_handle.h"

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

/**
 * 	stores pointers to low-level functions
 */

typedef struct {
	uint8_t data;
} nvm_data_t;

typedef struct {
	nvm_data_t data;
	uint8_t checksum;
} nvm_device_data_t;

typedef struct {
	nvm_data_t data;
	uint8_t is_valid;
} nvm_data_cache;

typedef enum
{
	NVM_API_STATUS_DO_NOT_FORMAT = 0,
	NVM_API_STATUS_FORMAT,
} nvm_formatting_status_t;

typedef struct
{
	nvm_high_api_status_t (*init)  (nvm_device_api_handle *const wl_handle,const nvm_formatting_status_t format);
	nvm_high_api_status_t (*read)  (nvm_device_api_handle *const wl_handle, nvm_data_t *data);
	nvm_high_api_status_t (*write) (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data);
} nvm_api_t;


extern nvm_api_t api;
