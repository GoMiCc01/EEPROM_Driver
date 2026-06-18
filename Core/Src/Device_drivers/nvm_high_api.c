#include "nvm_high_api.h"
#include "nvm_handle.h"
#include "nvm_low_api.h"

#define LAST_MEM_STRUCT_ADDRESS 0xFFFF
#define DEVICE_DATA_SIZE 2
#define MAX_DEVICE_COUNT 10
#define CRC8_POLY 0x07

static nvm_device_api_handle handle_pool[MAX_DEVICE_COUNT];
static uint8_t device_count = 0;
static uint8_t count_checksum(const nvm_data_t *const data);

nvm_device_api_handle *createEntity(I2C_HandleTypeDef *hi2c, uint8_t device_address)
{
	nvm_device_api_handle *handle = NULL;
	if (device_count < MAX_DEVICE_COUNT)
	{
		nvm_device_api_handle *handle = NULL;

		handle = &handle_pool[device_count];
		handle->hi2c = hi2c;
		handle->device_address = device_address;
		handle->initializing_status = NVM_API_STATUS_NOT_INITIALIZED;
		handle->device_mem_page = 0;
		handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		handle->device_mem_capacity = 0;
		handle->data_cache.is_valid = 0;
		device_count++;
	}
	return handle;
}

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle);

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

	if (NULL == wl_handle || NULL == data)
	{
		retcode = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (NVM_API_STATUS_OK == retcode && NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status)
	{
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
	if (NULL == wl_handle || NULL == user_data)
	{
		retcode = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (retcode == NVM_API_STATUS_OK && NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status)
	{
		retcode = NVM_API_STATUS_NOT_INITIALIZED;
	}

	uint16_t _MemAddress = 0;
	nvm_device_data_t data_device = {
		.data = *user_data,
		.checksum = count_checksum(user_data),
	};

	if ((retcode == NVM_API_STATUS_OK) && ((LAST_MEM_STRUCT_ADDRESS != wl_handle->last_busy_struct_address) && (wl_handle->last_busy_struct_address + DEVICE_DATA_SIZE >= wl_handle->device_mem_capacity)))
	{
		if (NVM_DEVICE_STATUS_OK != at24c256n_low_api.erase_all(wl_handle))
		{
			retcode = NVM_API_STATUS_WRITE_ERROR;
		}
		else
		{
			wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		};
	}

	if (retcode == NVM_API_STATUS_OK)
	{
		_MemAddress = (wl_handle->last_busy_struct_address == LAST_MEM_STRUCT_ADDRESS) ? 0x0000 : wl_handle->last_busy_struct_address + DEVICE_DATA_SIZE;
		if (NVM_DEVICE_STATUS_OK != at24c256n_low_api.write(wl_handle, _MemAddress, (uint8_t *)&data_device, DEVICE_DATA_SIZE))
		{
			retcode = NVM_API_STATUS_WRITE_ERROR;
		}
		else
		{
			wl_handle->data_cache.data = data_device.data;
			wl_handle->data_cache.is_valid = true;
			wl_handle->initializing_status = NVM_API_STATUS_OK;
			wl_handle->last_busy_struct_address = _MemAddress;
		}
	}
	return retcode;
}

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle)
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
			status = NVM_DEVICE_STATUS_READ_ERROR;
			break;
		}
		uint8_t is_empty = 1;
		uint8_t *buffer = (uint8_t *)&data;
		for (uint16_t j = 0; j < sizeof(nvm_device_data_t); j++)
		{
			if (buffer[j] != 0xFF)
			{
				is_empty = 0;
				break;
			}
		}
		if (is_empty)
		{
			break;
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
		wl_handle->data_cache.is_valid = 1;
		wl_handle->last_busy_struct_address = last_written;
	}
	else
	{
		wl_handle->data_cache.is_valid = 0;
		wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		wl_handle->initializing_status = NVM_API_STATUS_NO_DATA;
		status = NVM_API_STATUS_NO_DATA;
	}
	return status;
}

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

nvm_api_t api = {
	.init = init,
	.read = read,
	.write = write};
