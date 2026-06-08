
#include "at24c256n.h"

static nvm_device_status_t init_low  		(nvm_device_api_handle *const wl_handle);
static nvm_device_status_t read_low  		(nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size);
static nvm_device_status_t write_low		(nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size);
static nvm_device_status_t erase_all_low 	(nvm_device_api_handle *const wl_handle);
static nvm_device_status_t search_last_busy_page  	(nvm_device_api_handle *const wl_handle, const uint16_t size);
static HAL_StatusTypeDef at24c256n_wait_for_ready	(nvm_device_api_handle *const wl_handle);


static nvm_device_status_t init_low  		(nvm_device_api_handle *const wl_handle){
	wl_handle->device_mem_capacity = 32768;
	wl_handle->device_mem_page = 64;
	wl_handle->last_busy_struct_address = 0xFFFF;
	//search_last_busy_page(wl_handle, ); //TODO: Впевнитися, що тут буде заповенено поле wl_handle->last_busy_page
	HAL_StatusTypeDef status = at24c256n_wait_for_ready(wl_handle);
	//HAL_Delay(10);
	if (status != HAL_OK) {
				return NVM_DEVICE_STATUS_NOT_CONNECTED;
			}
	/*search_last_busy_page(wl_handle, 100);
	uint16_t t= wl_handle->last_busy_struct_address;*/

	//status = erase_all_low(wl_handle);
	HAL_Delay(10);
	uint8_t array[100] = {0};
	for(int i = 0; i < sizeof(array); i++){
		array[i] = i;
	}

	status = write_low(wl_handle, array, sizeof(array));
	if (status != HAL_OK) {
				return NVM_DEVICE_STATUS_NOT_CONNECTED;
			}
	HAL_Delay(10);
	uint8_t array1[100] = {0};
		for(int i = 0; i < sizeof(array1); i++){
			array1[i] = i+1;
		}
	status = write_low(wl_handle, array1, sizeof(array1));
		if (status != HAL_OK) {
					return NVM_DEVICE_STATUS_NOT_CONNECTED;
			}

	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t read_low  		(nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size){

	if (at24c256n_wait_for_ready(wl_handle) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

	if (HAL_I2C_Mem_Read(
		wl_handle->hi2c,
	    wl_handle->device_address,
		wl_handle->last_busy_struct_address,
	    I2C_MEMADD_SIZE_16BIT,
	    data,
	    size,
	    100) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_READ_ERROR;
	}

	return NVM_DEVICE_STATUS_OK;

}

static nvm_device_status_t write_low		(nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size) {
	uint16_t _Size = size;
	uint8_t *_pData = (uint8_t*)data;
	//uint16_t _MemAddress = wl_handle->last_busy_struct_address + size;
	uint16_t _MemAddress = (wl_handle->last_busy_struct_address == 0xFFFF) ? 0x0000 : wl_handle->last_busy_struct_address + size;
	const uint16_t new_memoryof_last_busy_page = _MemAddress;

	while (_Size > 0) {
		const uint8_t used_memory_for_writing = _MemAddress % 64;
		const uint8_t free_memory = 64 - used_memory_for_writing;
		uint16_t dataSize = (free_memory >= _Size) ? _Size : free_memory;

		HAL_StatusTypeDef status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
				_MemAddress, MEMADD_SIZE, _pData, dataSize, TIMEOUT);

		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_WRITE_ERROR;
		}

		status = at24c256n_wait_for_ready(wl_handle);
		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_NOT_CONNECTED;
		}

		_MemAddress += dataSize;
		_Size -= dataSize;
		_pData += dataSize;
	}

	wl_handle->last_busy_struct_address = new_memoryof_last_busy_page;
	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t erase_all_low		(nvm_device_api_handle *const wl_handle) {
	uint8_t page_buf[64];
	for (int i = 0; i < sizeof(page_buf); i++) {
		page_buf[i] = 0xFF;
	}
	uint16_t mem_address = 0x0000;
	uint16_t size = wl_handle->device_mem_capacity;
	while (size > 0) {
		HAL_StatusTypeDef status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
				mem_address, MEMADD_SIZE, page_buf, wl_handle->device_mem_page, TIMEOUT);

		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_WRITE_ERROR;
		}

		status = at24c256n_wait_for_ready(wl_handle);
		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_NOT_CONNECTED;
		}

		mem_address += wl_handle->device_mem_page;
		size -= wl_handle->device_mem_page;
	}
	wl_handle->last_busy_struct_address = 0xFFFF;
	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t search_last_busy_page  	(nvm_device_api_handle *const wl_handle, const uint16_t size){

	if (at24c256n_wait_for_ready(wl_handle) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

    uint8_t byte;
    uint32_t last_written = 0xFFFF;

    for (uint32_t addr = 0; addr < wl_handle->device_mem_capacity; addr++)
    {
        if (HAL_I2C_Mem_Read(
                wl_handle->hi2c,
                wl_handle->device_address,
                addr,
                I2C_MEMADD_SIZE_16BIT,
                &byte,
                1,
                100) != HAL_OK)
        {
            return NVM_DEVICE_STATUS_READ_ERROR;
        }

        if (byte != 0xFF)
        {
            last_written = addr;
        }
    }

    if (last_written == 0xFFFF)
    {
        wl_handle->last_busy_struct_address = 0;
        return NVM_DEVICE_STATUS_OK;
    }

    wl_handle->last_busy_struct_address = (last_written / size) * size;

    return NVM_DEVICE_STATUS_OK;
}

static HAL_StatusTypeDef at24c256n_wait_for_ready	(nvm_device_api_handle *const wl_handle) {
    return HAL_I2C_IsDeviceReady(wl_handle->hi2c, wl_handle->device_address, 50, TIMEOUT);
}

nvm_device_api_t api_low = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low,
		.search_last_busy_page = search_last_busy_page,
};
