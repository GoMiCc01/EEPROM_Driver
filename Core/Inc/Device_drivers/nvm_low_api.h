#pragma once
#include "nvm_handle.h"

/**
 * @brief Status codes returned by NVM device operations.
 */
typedef enum
{
    NVM_DEVICE_STATUS_OK = 0, 				/** Operation completed successfully. */
	NVM_DEVICE_STATUS_NOT_INITIALIZED,  	/** Device is not initialized. */
	NVM_DEVICE_STATUS_INVALID_PARAMETERS, 	/** Invalid function parameters. */
    NVM_DEVICE_STATUS_NOT_CONNECTED,  		/** Device is not accessible on the I2C bus. */
    NVM_DEVICE_STATUS_READ_ERROR, 			/** Read operation failed. */
	NVM_DEVICE_STATUS_WRITE_ERROR,			/** Write operation failed. */
} nvm_device_status_t;

/**
 * @brief Generic non-volatile memory device interface.
 *
 * Contains function pointers to low-level driver
 * implementations.
 */
typedef struct
{
	/**
	 * @brief Initializes the NVM device.
	 *
	 * @param[in,out] wl_handle Pointer to device handle.
	 */
	nvm_device_status_t (*init)  (nvm_device_api_handle *const wl_handle);

	/**
	 * @brief Reads data from the NVM device.
	 *
	 * @param[in] wl_handle Pointer to device handle.
	 * @param[in] mem_address Start memory address.
	 * @param[out] data Destination buffer.
	 * @param[in] size Number of bytes to read.
	 */
	nvm_device_status_t (*read)  (nvm_device_api_handle *const wl_handle, const uint16_t mem_address, uint8_t *const data, const uint16_t size);

	/**
	 * @brief Writes data to the NVM device.
	 *
	 * @param[in] wl_handle Pointer to device handle.
	 * @param[in] mem_address Start memory address.
	 * @param[in] data Source buffer.
	 * @param[in] size Number of bytes to write.
	 */
	nvm_device_status_t (*write) (nvm_device_api_handle *const wl_handle, const uint16_t mem_address, const uint8_t *const data, const uint16_t size);

	/**
	 * @brief Erases all data from the NVM device.
	 *
	 * @param[in] wl_handle Pointer to device handle.
	 */
	nvm_device_status_t (*erase_all) (nvm_device_api_handle *const wl_handle);
} nvm_device_api_t;

extern nvm_device_api_t at24c256n_low_api;
