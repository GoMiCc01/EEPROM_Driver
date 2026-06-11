
#include "nvm_high_api.h"

#define LAST_MEM_STRUCT_ADDRESS 0xFFFF
#define DEVICE_DATA_SIZE 2

extern nvm_device_api_t api_low;

static nvm_data_cache data_cache;

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle, const uint16_t size);

static uint8_t count_checksum(const nvm_device_data_t *const data);

static void transform_to_read(uint8_t *data, nvm_device_data_t *data_device);

static void transform_to_write(nvm_device_data_t *data_device, uint8_t *data);

static nvm_high_api_status_t find_valid_data(nvm_device_api_handle *const wl_handle);

static nvm_high_api_status_t init(nvm_device_api_handle *const wl_handle, const nvm_formatting_status_t format)
{
	nvm_high_api_status_t status = NVM_API_STATUS_OK;

	// 0) verify parameters
	if (wl_handle == NULL || wl_handle->hi2c == NULL || wl_handle->device_address == 0)
	{
		status = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	// 1) call init_low
	if (NVM_API_STATUS_OK == status)
	{
		if (api_low.init(wl_handle) != NVM_DEVICE_STATUS_OK)
		{
			wl_handle->initializing_status = NVM_API_STATUS_NOT_INITIALIZED;
			status = NVM_API_STATUS_NOT_INITIALIZED;
		}
	}

	// 2) format
	if ((NVM_API_STATUS_OK == status) && (format == NVM_API_STATUS_FORMAT))
	{
		if (api_low.erase_all(wl_handle) != NVM_DEVICE_STATUS_OK)
		{
			status = NVM_API_STATUS_WRITE_ERROR;
		}
		else
		{
			wl_handle->initializing_status = NVM_API_STATUS_NO_DATA;
			wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		}
	}
	else if ((NVM_API_STATUS_OK == status) && last_busy_struct_address(wl_handle, sizeof(nvm_device_data_t)) != NVM_DEVICE_STATUS_OK)
	{
		wl_handle->initializing_status = NVM_API_STATUS_NO_DATA;
		status = NVM_API_STATUS_READ_ERROR;
	}

	if (NVM_API_STATUS_OK == status)
	{
		data_cache.is_valid = 0;

		if (wl_handle->last_busy_struct_address != LAST_MEM_STRUCT_ADDRESS)
		{
			status = find_valid_data(wl_handle);
		}
	}

	return status;
}

static nvm_high_api_status_t read(nvm_device_api_handle *const wl_handle, nvm_data_t *data)
{
	if (NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status)
	{
		return NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NULL == wl_handle || NULL == wl_handle->hi2c || 0 == wl_handle->device_address || NULL == data)
	{
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (1 == data_cache.is_valid)
	{
		*data = data_cache.data;
	}
	else
	{
		if (api_low.read(wl_handle, wl_handle->last_busy_struct_address, &(data->data), sizeof(data->data)) != NVM_DEVICE_STATUS_OK)
		{
			return NVM_API_STATUS_READ_ERROR;
		}
	}

	return NVM_API_STATUS_OK;
}

static nvm_high_api_status_t write(nvm_device_api_handle *const wl_handle, const nvm_data_t *const data)
{

	if (NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status)
	{
		return NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NULL == wl_handle || 0 == wl_handle->device_address || NULL == wl_handle->hi2c || NULL == data)
	{
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}
	if (LAST_MEM_STRUCT_ADDRESS != wl_handle->last_busy_struct_address && (wl_handle->last_busy_struct_address + sizeof(data->data) >= wl_handle->device_mem_capacity))
	{
		if (NVM_DEVICE_STATUS_OK != api_low.erase_all(wl_handle))
			return NVM_API_STATUS_WRITE_ERROR;
	}

	if (NVM_DEVICE_STATUS_OK != api_low.write(wl_handle, wl_handle->last_busy_struct_address, &(data->data), sizeof(data->data)))
	{
		data_cache.is_valid = 0;
		return NVM_API_STATUS_WRITE_ERROR;
	}
	else
	{
		data_cache.data = *data;
		data_cache.is_valid = 1;
		return NVM_API_STATUS_OK;
	}
}

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle, const uint16_t size)
{

	uint8_t buffer[wl_handle->device_mem_page];
	uint16_t pages = wl_handle->device_mem_capacity / wl_handle->device_mem_page;

	uint16_t last_written = LAST_MEM_STRUCT_ADDRESS;

	for (uint16_t page = 0; page < pages; page++)
	{
		uint16_t addr = page * wl_handle->device_mem_page;

		if (NVM_DEVICE_STATUS_OK != api_low.read(wl_handle, addr, buffer, wl_handle->device_mem_page))
		{
			return NVM_DEVICE_STATUS_READ_ERROR;
		}

		for (uint16_t i = 0; i < wl_handle->device_mem_page; i++)
		{
			if (buffer[i] != 0xFF)
			{
				last_written = addr + i;
			}
		}
	}

	if (last_written == LAST_MEM_STRUCT_ADDRESS)
	{
		wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
		return NVM_DEVICE_STATUS_OK;
	}

	wl_handle->last_busy_struct_address = (last_written / size) * size;

	return NVM_DEVICE_STATUS_OK;
}

static nvm_high_api_status_t find_valid_data(nvm_device_api_handle *const wl_handle)
{
	nvm_high_api_status_t status = NVM_API_STATUS_OK;
	uint16_t current_busy_address = wl_handle->last_busy_struct_address;
	uint8_t found_valid_data = 0;
	uint8_t raw_buffer[sizeof(nvm_device_data_t)];
	nvm_device_data_t buffer;
	while (!found_valid_data)
	{
		if (NVM_DEVICE_STATUS_OK == api_low.read(wl_handle, current_busy_address, raw_buffer, sizeof(raw_buffer)))
		{
			transform_to_read(raw_buffer, &buffer);
			if (count_checksum(&buffer) == buffer.checksum)
			{
				data_cache.data = buffer.data;
				data_cache.is_valid = 1;
				wl_handle->last_busy_struct_address = current_busy_address;
				found_valid_data = 1;
			}
			else
			{
				wl_handle->initializing_status = NVM_API_STATUS_CORRUPTED_DATA;
				if (current_busy_address >= sizeof(nvm_device_data_t))
				{
					current_busy_address -= sizeof(nvm_device_data_t);
				}
				else
				{
					wl_handle->last_busy_struct_address = LAST_MEM_STRUCT_ADDRESS;
					data_cache.is_valid = 0;
					break;
				}
			}
		}
		else
		{
			status = NVM_API_STATUS_READ_ERROR;
			data_cache.is_valid = 0;
			break;
		}
	}
	return status;
}

nvm_api_t api = {
	.init = init,
	.read = read,
	.write = write};
