/**
 * @file nvm_handle.h
 * @brief Internal structures and state markers for the NVM High API.
 */

#pragma once


#include <stdint.h>
#include <stdbool.h>
#include "nvm_high_api.h"

/**
 * @name NVM Record Status Flags
 * @brief Markers used to identify the physical and logical state of a memory cell in EEPROM.
 * @{
 */
#define NVM_RECORD_EMPTY   0xFF	/**< Erased (free space) */
#define NVM_RECORD_VALID   0xAA	/**< Actual data written */
#define NVM_RECORD_BAD     0xBB /**< Physically bad cell */
/** @} */

/**
 * @brief Data structure stored directly in memory.
 * Contains user data and a checksum for validation.
 */
typedef struct
{
    uint8_t memory_flag; /**< Indicates memory cell state */
	uint8_t checksum; /**< CRC8 checksum */
    nvm_data_t data;  /**< Main payload data */
} nvm_device_data_t;

/**
 * @brief Structure for caching the latest read/written data in RAM.
 */
typedef struct
{
    nvm_data_t data; /**< Cached data */
    bool is_valid;   /**< Cache validity flag */
} nvm_data_cache;


/**
 * @brief Internal structure of the NVM device handle.
 */
struct nvm_device_api_handle_s
{
    I2C_HandleTypeDef *hi2c;             /**< Pointer to the I2C bus handle */
    uint8_t device_address;              /**< I2C address of the device */
    uint8_t device_mem_page;             /**< Memory page size */
    uint16_t last_busy_struct_address;   /**< Address of the last valid record */
    uint16_t device_mem_capacity;        /**< Total memory capacity of the device */
    nvm_high_api_status_t initializing_status; /**< Current initialization status */
    nvm_data_cache data_cache;           /**< Local data cache */
};
