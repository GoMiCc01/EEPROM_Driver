/**
 * @file nvm_high_api.c
 * @brief High-level API implementation for Non-Volatile Memory (NVM) wear-leveling, caching, and bad block management.
 */

#include "nvm_high_api.h"
#include "nvm_handle.h"
#include "nvm_low_api.h"

#define LAST_MEM_STRUCT_ADDRESS 0xFFFF
#define MAX_DEVICE_COUNT 10
#define CRC8_POLY 0x07

static nvm_device_api_handle handle_pool[MAX_DEVICE_COUNT];
static uint8_t device_count = 0;

static uint8_t count_checksum(const nvm_data_t *const data);
static nvm_high_api_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle);
static bool is_memory_equal(const uint8_t* arg1 , const uint8_t* arg2, uint16_t size);

static inline bool is_parameter_valid(nvm_device_api_handle *const wl_handle, nvm_data_t * const data) {
	bool retcode = true;
	if (NULL == wl_handle || NULL == data)
		{
			retcode = false;
		}
	return retcode;
}

static inline bool is_initialized(nvm_device_api_handle *const wl_handle) {
	bool retcode = true;
	if (NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status)
	{
		retcode = false;
	}
	return retcode;
}

nvm_device_api_handle *createEntity(I2C_HandleTypeDef *hi2c, uint8_t device_address)
{
	nvm_device_api_handle *handle = NULL;
	if (device_count < MAX_DEVICE_COUNT)
	{
		handle = &handle_pool[device_count];
		handle->hi2c = hi2c;
		handle->device_address = device_address;
		handle->initializing_status = NVM_API_STATUS_NOT_INITIALIZED;
		handle->device_mem_page = 0;
		handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		handle->device_mem_capacity = 0;
		handle->data_cache.is_valid = false;
		device_count++;
	}
	return handle;
}

static nvm_high_api_status_t init(nvm_device_api_handle *const wl_handle, const nvm_formatting_status_t format)
{
	wl_handle->initializing_status = NVM_API_STATUS_OK;
	nvm_high_api_status_t status = NVM_API_STATUS_OK;

	// 0) verify parameters
	if (NULL == wl_handle)
	{
		status = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	// 1) call init_low
	if (NVM_API_STATUS_OK == status)
	{
		if (at24c256n_low_api.init(wl_handle) != NVM_DEVICE_STATUS_OK)
		{
			wl_handle->initializing_status = NVM_API_STATUS_NOT_INITIALIZED;
			status = NVM_API_STATUS_NOT_INITIALIZED;
		}
	}

	// 2) format
	if ((NVM_API_STATUS_OK == status) && (NVM_API_STATUS_FORMAT == format))
	{
		if (at24c256n_low_api.erase_all(wl_handle) != NVM_DEVICE_STATUS_OK)
		{
			status = NVM_API_STATUS_WRITE_ERROR;
		}
		else
		{
			wl_handle->initializing_status = NVM_API_STATUS_NO_DATA;
			wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		}
	}
	else if (NVM_API_STATUS_OK == status)
	{
		status = last_busy_struct_address(wl_handle);
	}

	return status;
}

static nvm_high_api_status_t read(nvm_device_api_handle *const wl_handle, nvm_data_t *data)
{
	nvm_high_api_status_t retcode = NVM_API_STATUS_OK;

	if (!is_parameter_valid(wl_handle, (nvm_data_t * const)data)) {
		retcode = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (!is_initialized(wl_handle)) {
		retcode = NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NVM_API_STATUS_OK == retcode && NVM_API_STATUS_NO_DATA == wl_handle->initializing_status)
	{
		retcode = NVM_API_STATUS_NO_DATA;
	}

	if (NVM_API_STATUS_OK == retcode && wl_handle->data_cache.is_valid)
	{
		*data = wl_handle->data_cache.data;
	}
	else
	{
		retcode = NVM_API_STATUS_READ_ERROR;
	}

	return retcode;
}

static nvm_high_api_status_t write(nvm_device_api_handle *const wl_handle, const nvm_data_t *const user_data)
{
	nvm_high_api_status_t retcode = NVM_API_STATUS_OK;
	if (!is_parameter_valid(wl_handle, (nvm_data_t * const)user_data)) {
		retcode = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (!is_initialized(wl_handle)) {
		retcode = NVM_API_STATUS_NOT_INITIALIZED;
	}

	uint16_t _MemAddress = 0;
	nvm_device_data_t data_device = {
		.data = *user_data,
		.checksum = count_checksum(user_data),
		.memory_flag = NVM_RECORD_VALID
	};

	if (retcode == NVM_API_STATUS_OK)
	{
		_MemAddress = (wl_handle-> last_busy_struct_address == LAST_MEM_STRUCT_ADDRESS)?
					  0x0000 : wl_handle-> last_busy_struct_address+sizeof(nvm_device_data_t);
		bool write_ok = false;
		while(!write_ok && retcode == NVM_API_STATUS_OK)
		{
			if(_MemAddress + sizeof(nvm_device_data_t) >= wl_handle->device_mem_capacity)
			{
				if (NVM_DEVICE_STATUS_OK != at24c256n_low_api.erase_all(wl_handle))
				{
					retcode = NVM_API_STATUS_WRITE_ERROR;
					break;
				}
				_MemAddress = 0x0000;
				wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
			}
			if (NVM_DEVICE_STATUS_OK != at24c256n_low_api.write(wl_handle, _MemAddress, (uint8_t *)&data_device, sizeof(nvm_device_data_t)))
			{
				retcode = NVM_API_STATUS_WRITE_ERROR;
				break;
			}
			nvm_device_data_t read_back;
			if(NVM_DEVICE_STATUS_OK != at24c256n_low_api.read(wl_handle,_MemAddress, (uint8_t *)&read_back, sizeof(nvm_device_data_t)))
			{
				retcode = NVM_API_STATUS_READ_ERROR;
				break;
			}
			if(is_memory_equal((uint8_t*)&data_device, (uint8_t*)&read_back, sizeof(nvm_device_data_t)))
			{
				write_ok = true;
			}
			else
			{
				uint8_t bad_flag = NVM_RECORD_BAD;
				at24c256n_low_api.write(wl_handle, _MemAddress, &bad_flag , sizeof(bad_flag));
				_MemAddress += sizeof(nvm_device_data_t);
			}

		}
		if(write_ok == true)
		{
			wl_handle->data_cache.data = data_device.data;
			wl_handle->data_cache.is_valid = true;
			wl_handle->initializing_status = NVM_API_STATUS_OK;
			wl_handle->last_busy_struct_address = _MemAddress;
			retcode = NVM_API_STATUS_OK;
		}
	}

	return retcode;
}

/**
 * @brief Searches for the last valid record in memory.
 * Scans the EEPROM to find the latest written data and loads it into the cache.
 * @param wl_handle Pointer to the device handle.
 * @return Search status (OK, NO_DATA, or CORRUPTED_DATA).
 */
static nvm_high_api_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle)
{
	nvm_high_api_status_t status = NVM_API_STATUS_OK;
	nvm_device_data_t data;
	uint16_t struct_count = wl_handle->device_mem_capacity / sizeof(nvm_device_data_t);
	uint16_t last_written = LAST_MEM_STRUCT_ADDRESS;
	uint8_t found_valid_data = 0;
	nvm_device_data_t valid_data;

	for (uint16_t struct_num = 0; struct_num < struct_count; struct_num++)
	{
		uint16_t addr = struct_num * sizeof(nvm_device_data_t);

		if (NVM_DEVICE_STATUS_OK != at24c256n_low_api.read(wl_handle, addr, (uint8_t *)&data, sizeof(nvm_device_data_t)))
		{
			status = NVM_API_STATUS_READ_ERROR;
			break;
		}
		if(data.memory_flag == NVM_RECORD_EMPTY){
			break;
		}
		else if(data.memory_flag == NVM_RECORD_BAD){
			continue;
		}
		if (count_checksum(&data.data) == data.checksum)
		{
			last_written = addr;
			valid_data = data;
			found_valid_data = 1;
		}
		else
		{
			wl_handle->initializing_status = NVM_API_STATUS_CORRUPTED_DATA;
			status = NVM_API_STATUS_CORRUPTED_DATA;
			break;
		}
	}

	if (found_valid_data)
	{
		wl_handle->data_cache.data = valid_data.data;
		wl_handle->data_cache.is_valid = true;
		wl_handle->last_busy_struct_address = last_written;
	}
	else
	{
		wl_handle->data_cache.is_valid = false;
		wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;

		if(status == NVM_API_STATUS_OK)
		{
			wl_handle->initializing_status = NVM_API_STATUS_NO_DATA;
			status = NVM_API_STATUS_NO_DATA;
		}
	}
	return status;
}

/**
 * @brief Calculates the CRC8 checksum for the data structure.
 * @param data Pointer to the user data.
 * @return Calculated CRC value.
 */
static uint8_t count_checksum(const nvm_data_t *const data)
{
	const uint8_t *bytes = (const uint8_t *)&data->data;
	uint8_t crc = 0;

	for (uint16_t i = 0; i < sizeof(data->data); i++)
	{
		crc ^= bytes[i];
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (crc & 0x80)
			{
				crc = (crc << 1) ^ CRC8_POLY;
			}
			else
			{
				crc <<= 1;
			}
		}
	}
	return crc;
}

/**
 * @brief Compares two memory blocks byte by byte.
 * @param arg1 Pointer to the first memory block.
 * @param arg2 Pointer to the second memory block.
 * @param size Number of bytes to compare.
 * @return true if both memory blocks are identical, false if there are any differences.
 */
static bool is_memory_equal(const uint8_t* arg1 , const uint8_t* arg2, uint16_t size)
{
	for(uint16_t i = 0; i < size; i++)
	{
		if(arg1[i]!=arg2[i]){
			return false;
		}
	}
	return true;
}

/**
 * @brief Global instance of the NVM High API interface.
 * Contains function pointers to the high-level operations.
 */
nvm_api_t api = {
	.init = init,
	.read = read,
	.write = write};
