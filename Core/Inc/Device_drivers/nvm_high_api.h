/**
 * @file nvm_high_api.h
 * @brief High-level API for Non-Volatile Memory (NVM) management.
 * Provides structures and function prototypes for reading, writing,
 * and wear-leveling data in EEPROM/Flash.
 */

#pragma once

#include <stdint.h>
#include "main.h"

/**
 * @brief Execution statuses for NVM High API operations.
 */
typedef enum
{
	// init
	NVM_API_STATUS_NOT_INITIALIZED = 99,/**< NVM_API_STATUS_NOT_INITIALIZED */
	NVM_API_STATUS_OK = 0,              /**< NVM_API_STATUS_OK */
	// write
	NVM_API_STATUS_WRITE_ERROR,         /**< NVM_API_STATUS_WRITE_ERROR */
	// read
	NVM_API_STATUS_READ_ERROR,          /**< NVM_API_STATUS_READ_ERROR */
	NVM_API_STATUS_NO_DATA,             /**< NVM_API_STATUS_NO_DATA */
	NVM_API_STATUS_CORRUPTED_DATA,      /**< NVM_API_STATUS_CORRUPTED_DATA */
	// general
	NVM_API_STATUS_INVALID_PARAMETERS,  /**< NVM_API_STATUS_INVALID_PARAMETERS */
} nvm_high_api_status_t;

/**
 * @brief Opaque type for the NVM device handle.
 */
typedef struct nvm_device_api_handle_s nvm_device_api_handle;

/**
 * @brief Creates or allocates a new NVM device handle.
 * @param hi2c Pointer to the I2C HAL handle.
 * @param device_address I2C address of the memory device.
 * @return Pointer to the created handle, or NULL if the device limit is reached.
 */
nvm_device_api_handle *createEntity(I2C_HandleTypeDef *hi2c, uint8_t device_address);

/**
 * @brief User data structure for saving.
 */
typedef struct
{
	uint8_t data;
} nvm_data_t;

/**
 * @brief Formatting modes during initialization.
 */
typedef enum
{
	NVM_API_STATUS_DO_NOT_FORMAT = 0,/**< Do not format memory on startup */
	NVM_API_STATUS_FORMAT,           /**< Clear memory on startup */
} nvm_formatting_status_t;

/**
 * @brief High API interface for NVM operations.
 */
typedef struct
{
	/**
	* @brief Initializes the device and restores its state.
	* @param wl_handle Pointer to the device handle.
	* @param format Memory formatting flag.
	* @return Initialization status.
	*/
	nvm_high_api_status_t (*init)(nvm_device_api_handle *const wl_handle, const nvm_formatting_status_t format);

	/**
	* @brief Reads the latest valid data (either from cache or memory).
	* @param wl_handle Pointer to the device handle.
	* @param data Pointer to the structure where data will be stored.
	* @return Read operation status.
	*/
	nvm_high_api_status_t (*read)(nvm_device_api_handle *const wl_handle, nvm_data_t *data);

	/**
	* @brief Writes new data to memory (incorporating wear-leveling).
	* @param wl_handle Pointer to the device handle.
	* @param data Pointer to the data to be written.
	* @return Write operation status.
	*/
	nvm_high_api_status_t (*write)(nvm_device_api_handle *const wl_handle, const nvm_data_t *const data);
} nvm_api_t;

extern nvm_api_t api;
