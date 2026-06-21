#include "nvm_handle.h"
#include "nvm_low_api.h"
#include <stdbool.h>
#include <string.h>

#define MEMADD_SIZE 			I2C_MEMADD_SIZE_16BIT
#define WRITE_TIMEOUT 			10
#define READ_TIMEOUT 			100
#define MAX_ATTEMPTS_TRY 		50
#define AT24C256N_MEM_CAPACITY	32768
#define AT24C256N_PAGE_SIZE 	64

static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle);
static inline bool is_handle_valid(nvm_device_api_handle *const wl_handle){
	return (NULL != wl_handle) && (NULL != wl_handle->hi2c) && (0 != wl_handle->device_address);
}

static inline bool is_parameter_valid(nvm_device_api_handle *const wl_handle, const uint16_t mem_address, const uint8_t *const data, const uint16_t size){
	return is_handle_valid(wl_handle) && (size <= (wl_handle->device_mem_capacity - mem_address)) && (NULL != data) && (0 != size);
}
/**
 * @brief Initializes AT24C256N low-level driver.
 *
 * Configures device-specific parameters such as total memory size
 * and page size, then verifies device availability on the I2C bus.
 *
 * @param[in,out] wl_handle Pointer to device handle.
 *
 * @retval NVM_DEVICE_STATUS_OK Device initialized successfully.
 * @retval NVM_DEVICE_STATUS_INVALID_PARAMETERS Invalid handle or device configuration.
 * @retval NVM_DEVICE_STATUS_NOT_CONNECTED Device is not responding.
 */
static nvm_device_status_t init_low(nvm_device_api_handle *const wl_handle)
{
	nvm_device_status_t retcode = NVM_DEVICE_STATUS_OK;
	HAL_StatusTypeDef status;

	if (!is_handle_valid(wl_handle)) {
		retcode = NVM_DEVICE_STATUS_INVALID_PARAMETERS;
	}

	if (NVM_DEVICE_STATUS_OK == retcode)
	{
		wl_handle->device_mem_capacity = AT24C256N_MEM_CAPACITY;
		wl_handle->device_mem_page = AT24C256N_PAGE_SIZE;

		status = at24c256n_wait_for_ready(wl_handle);
		if (HAL_OK != status)
		{
			retcode = NVM_DEVICE_STATUS_NOT_CONNECTED;
		}
	}
	return retcode;
}

/**
 * @brief Reads data from EEPROM.
 *
 * Performs a random read operation starting from the specified
 * memory address.
 *
 * @param[in] wl_handle Pointer to device handle.
 * @param[in] mem_address Start memory address.
 * @param[out] data Destination buffer.
 * @param[in] size Number of bytes to read.
 *
 * @retval NVM_DEVICE_STATUS_OK Read completed successfully.
 * @retval NVM_DEVICE_STATUS_INVALID_PARAMETERS Invalid handle or device configuration or read parameters.
 * @retval NVM_DEVICE_STATUS_READ_ERROR Read transaction failed.
 */
static nvm_device_status_t read_low(nvm_device_api_handle *const wl_handle, const uint16_t mem_address, uint8_t *const data, const uint16_t size)
{
	nvm_device_status_t retcode = NVM_DEVICE_STATUS_OK;

	if (!is_parameter_valid(wl_handle, mem_address, data, size)) {
		retcode = NVM_DEVICE_STATUS_INVALID_PARAMETERS;
	}
	else
	{
        if (at24c256n_wait_for_ready(wl_handle) != HAL_OK)
        {
        	retcode = NVM_DEVICE_STATUS_NOT_CONNECTED;
        }
        else if (HAL_I2C_Mem_Read(
        		wl_handle->hi2c,
				wl_handle->device_address,
				mem_address,
				I2C_MEMADD_SIZE_16BIT,
				data,
				size,
				READ_TIMEOUT) != HAL_OK)
		{
        	retcode = NVM_DEVICE_STATUS_READ_ERROR;
		}
	}
	return retcode;
}

/**
 * @brief Writes data to EEPROM.
 *
 * Data is written using page-oriented transactions. If the requested
 * write operation crosses a page boundary, the data is automatically
 * split into multiple page writes according to the device page size.
 *
 * The function waits for completion of each internal EEPROM write cycle
 * before starting the next page transaction.
 *
 * Write algorithm:
 *
 * 1. Calculate current page offset.
 * 2. Determine available space in the current page.
 * 3. Limit transaction size to page boundary.
 * 4. Wait until EEPROM completes previous write cycle.
 * 5. Perform page write transaction.
 * 6. Advance memory address and data pointer.
 * 7. Repeat until all requested bytes are written.
 *
 * @param[in] wl_handle Pointer to device handle.
 * @param[in] mem_address Start memory address.
 * @param[in] data Source buffer.
 * @param[in] size Number of bytes to write.
 *
 * @retval NVM_DEVICE_STATUS_OK Write completed successfully.
 * @retval NVM_DEVICE_STATUS_INVALID_PARAMETERS Invalid handle or device configuration or write parameters.
 * @retval NVM_DEVICE_STATUS_NOT_CONNECTED Device is not responding.
 * @retval NVM_DEVICE_STATUS_WRITE_ERROR Write transaction failed.
 */

static nvm_device_status_t write_low(nvm_device_api_handle *const wl_handle, const uint16_t mem_address, const uint8_t *const data, const uint16_t size)
{
	nvm_device_status_t retcode = NVM_DEVICE_STATUS_OK;
	HAL_StatusTypeDef status;

	if (!is_parameter_valid(wl_handle, mem_address, data, size)) {
		retcode = NVM_DEVICE_STATUS_INVALID_PARAMETERS;
	}

	if (NVM_DEVICE_STATUS_OK == retcode)
	{
		uint16_t _MemAddress = mem_address;
		uint8_t *_pData = (uint8_t *)data;
		int32_t _Size = size;

		while (_Size > 0 && NVM_DEVICE_STATUS_OK == retcode)
		{
			const uint8_t used_memory_for_writing = _MemAddress % wl_handle->device_mem_page;
			const uint8_t free_memory_for_writing = wl_handle->device_mem_page - used_memory_for_writing;
			uint16_t dataSize = (free_memory_for_writing >= _Size) ? _Size : free_memory_for_writing;

			status = at24c256n_wait_for_ready(wl_handle);
			if (HAL_OK != status)
			{
				retcode = NVM_DEVICE_STATUS_NOT_CONNECTED;
			}

			if (NVM_DEVICE_STATUS_OK == retcode)
			{
				status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
										   _MemAddress, MEMADD_SIZE, _pData, dataSize, WRITE_TIMEOUT);

				if (HAL_OK != status)
				{
					retcode = NVM_DEVICE_STATUS_WRITE_ERROR;
				}
			}

			if (NVM_DEVICE_STATUS_OK == retcode)
			{
				_MemAddress += dataSize;
				_Size -= dataSize;
				_pData += dataSize;
			}
		}
	}

	return retcode;
}

/**
 * @brief Erases the entire EEPROM.
 *
 * The device memory is filled with 0xFF page by page.
 *
 * @param[in] wl_handle Pointer to device handle.
 *
 * @retval NVM_DEVICE_STATUS_OK Erase completed successfully.
 * @retval NVM_DEVICE_STATUS_INVALID_PARAMETERS Invalid handle or device configuration.
 * @retval NVM_DEVICE_STATUS_NOT_CONNECTED Device is not responding.
 * @retval NVM_DEVICE_STATUS_WRITE_ERROR Erase transaction failed.
 */
static nvm_device_status_t erase_all_low(nvm_device_api_handle *const wl_handle)
{
	nvm_device_status_t retcode = NVM_DEVICE_STATUS_OK;
	HAL_StatusTypeDef status;

	if (!is_handle_valid(wl_handle)) {
		retcode = NVM_DEVICE_STATUS_INVALID_PARAMETERS;
	}

	if (NVM_DEVICE_STATUS_OK == retcode)
	{
		uint16_t mem_address = 0x0000;
		int32_t size = wl_handle->device_mem_capacity;
		uint8_t page_buf[wl_handle->device_mem_page];
		memset(page_buf, 0xFF, sizeof(page_buf));

		while (size > 0 && NVM_DEVICE_STATUS_OK == retcode)
		{
			status = at24c256n_wait_for_ready(wl_handle);
			if (HAL_OK != status)
			{
				retcode = NVM_DEVICE_STATUS_NOT_CONNECTED;
			}

			if (NVM_DEVICE_STATUS_OK == retcode)
			{
				status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
										   mem_address, MEMADD_SIZE, page_buf, wl_handle->device_mem_page, WRITE_TIMEOUT);

				if (HAL_OK != status)
				{
					retcode = NVM_DEVICE_STATUS_WRITE_ERROR;
				}
			}

			if (NVM_DEVICE_STATUS_OK == retcode)
			{
				mem_address += wl_handle->device_mem_page;
				size -= wl_handle->device_mem_page;
			}
		}
	}
	return retcode;
}

/**
 * @brief Waits until EEPROM becomes ready for the next operation.
 *
 * Polls the device using I2C acknowledge until the internal
 * write cycle is completed or timeout occurs.
 *
 * @param[in] wl_handle Pointer to device handle.
 *
 * @return HAL status code.
 * @retval HAL_OK Device is ready.
 * @retval HAL_ERROR Device is not responding.
 * @retval HAL_BUSY I2C bus is busy.
 * @retval HAL_TIMEOUT Timeout expired.
 */
static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle)
{
	return HAL_I2C_IsDeviceReady(wl_handle->hi2c, wl_handle->device_address, MAX_ATTEMPTS_TRY, WRITE_TIMEOUT);
}


nvm_device_api_t at24c256n_low_api = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low
};
