
#include "nvm_high_api.h"

#define LAST_MEM_STRUCT_ADDRESS 0xFFFF
#define DEVICE_DATA_SIZE 2

extern nvm_device_api_t api_low;

static nvm_data_cache data_cache;

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle, const uint16_t size);

static uint8_t count_checksum(const nvm_data_t *const data);

static void transform_to_read(uint8_t* data, nvm_device_data_t* data_device);

static void transform_to_write(nvm_device_data_t* data_device, uint8_t *data);

static nvm_high_api_status_t init(nvm_device_api_handle *const wl_handle, const nvm_formatting_status_t format)
{
	// 0) Verify parameters
	if (wl_handle->hi2c == NULL || wl_handle->device_address == 0)
	{
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}
	// 1) Verify format
	if (format == NVM_API_STATUS_FORMAT)
	{
		// 1) erase -> busy_page = 0xffff
		nvm_device_status_t status = api_low.erase_all(wl_handle);
		if (status != NVM_DEVICE_STATUS_OK)
		{
			return NVM_API_STATUS_WRITE_ERROR;
		}
	}
	// 2) call init_low
	nvm_device_status_t status = api_low.init(wl_handle);
	if (status != NVM_DEVICE_STATUS_OK)
	{
		return NVM_API_STATUS_NOT_INITIALIZED;
	}
	// 3) fill the field is_device_initialized
	//wl_handle->is_device_initialized = 1;
	// 4) call search last busy page
	//status = api_low.last_busy_struct_address(wl_handle, sizeof(nvm_data_t)); //TODO: зробити цю функцію
	// 5) it looks like nothing to fill
	// 6) return status
	return NVM_API_STATUS_OK;
}

static nvm_high_api_status_t read  (nvm_device_api_handle *const wl_handle, nvm_data_t *data){
	nvm_high_api_status_t retcode = NVM_API_STATUS_OK;

	if (NULL == wl_handle || NULL == wl_handle->hi2c || 0 == wl_handle->device_address || NULL == data) {
		retcode = NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (NVM_API_STATUS_OK == retcode && NVM_API_STATUS_NOT_INITIALIZED == wl_handle->initializing_status) {
		retcode = NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NVM_API_STATUS_NO_DATA == wl_handle->initializing_status) {
		retcode = NVM_API_STATUS_NO_DATA;
	}

	if (NVM_API_STATUS_OK == retcode && 1 == data_cache.is_valid && wl_handle->initializing_status != NVM_API_STATUS_CORRUPTED_DATA) {
		*data = data_cache.data;
	} else if (NVM_API_STATUS_OK == retcode && 1 == data_cache.is_valid && NVM_API_STATUS_CORRUPTED_DATA == wl_handle->initializing_status) {
		*data = data_cache.data;
		retcode = NVM_API_STATUS_CORRUPTED_DATA;
	} else {
		if (NVM_API_STATUS_OK == retcode) {
			uint8_t read_data_arr[DEVICE_DATA_SIZE] = {0};
			if (api_low.read(wl_handle, wl_handle->last_busy_struct_address, read_data_arr, sizeof(read_data_arr)) == NVM_DEVICE_STATUS_OK) {
				nvm_device_data_t read_data;
				transform_to_read(read_data_arr, &read_data);
				if(read_data.checksum != count_checksum(&(read_data.data))) {
					retcode = NVM_API_STATUS_CORRUPTED_DATA;
				}
			} else {
				retcode = NVM_API_STATUS_READ_ERROR;
			}
		}
	}

	return retcode;
}

static nvm_high_api_status_t write (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data){

//	if (1 != wl_handle->is_device_initialized) {
//		return NVM_API_STATUS_NOT_INITIALIZED;
//	}

	if (NULL == wl_handle || 0 == wl_handle->device_address || NULL == wl_handle->hi2c || NULL == data) {
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}
	if (LAST_MEM_STRUCT_ADDRESS != wl_handle->last_busy_struct_address && (wl_handle->last_busy_struct_address + sizeof(data->data) >= wl_handle->device_mem_capacity)) {
		if (NVM_DEVICE_STATUS_OK != api_low.erase_all(wl_handle)) return NVM_API_STATUS_WRITE_ERROR;
	}

	if (NVM_DEVICE_STATUS_OK != api_low.write(wl_handle, wl_handle->last_busy_struct_address, &(data->data), sizeof(data->data))) {
		data_cache.is_valid = 0;
		return NVM_API_STATUS_WRITE_ERROR;
	} else {
		data_cache.data = *data;
		data_cache.is_valid = 1;
		return NVM_API_STATUS_OK;
	}

}

static nvm_device_status_t last_busy_struct_address(nvm_device_api_handle *const wl_handle, const uint16_t size){

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

static uint8_t count_checksum(const nvm_data_t *const data) {
	uint8_t bits = sizeof(data->data) * 8;
	uint8_t mask = 1;
	uint8_t checksum = 0;

	for (int i = 0; i < bits; ++i) {
		if (((mask << i) & data->data) != 0) {
			checksum++;
		}
	}

	return checksum;
}

static void transform_to_read(uint8_t* data, nvm_device_data_t* data_device) {

}

static void transform_to_write(nvm_device_data_t* data_device, uint8_t *data) {

}

nvm_api_t api = {
	.init = init,
	.read = read,
	.write = write};
