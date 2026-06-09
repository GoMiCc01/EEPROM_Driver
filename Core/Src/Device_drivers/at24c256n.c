
#include <at24c256n_low_api.h>

#define MEMADD_SIZE 		I2C_MEMADD_SIZE_16BIT
#define WRITE_TIMEOUT 		10
#define READ_TIMEOUT 		100
#define MAX_ATTEMPTS_TRY 	50

static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle);

static nvm_device_status_t init_low(nvm_device_api_handle *const wl_handle){
	wl_handle->device_mem_capacity = 32768;
	wl_handle->device_mem_page = 64;

	HAL_StatusTypeDef status = at24c256n_wait_for_ready(wl_handle);
	if (status != HAL_OK) {
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t read_low(nvm_device_api_handle *const wl_handle, const uint16_t mem_address, uint8_t *const data, const uint16_t size){
	if (at24c256n_wait_for_ready(wl_handle) != HAL_OK) {
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

	if (HAL_I2C_Mem_Read(
		wl_handle->hi2c,
	    wl_handle->device_address,
		mem_address,
	    I2C_MEMADD_SIZE_16BIT,
	    data,
	    size,
		READ_TIMEOUT) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_READ_ERROR;
	}

	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t write_low(nvm_device_api_handle *const wl_handle, const uint16_t mem_address, const uint8_t *const data, const uint16_t size) {
	uint16_t _MemAddress = mem_address;
	uint8_t *_pData = (uint8_t*)data;
	int32_t _Size = size;

	while (_Size > 0) {
		const uint8_t used_memory_for_writing = _MemAddress % wl_handle->device_mem_page;
		const uint8_t free_memory_for_writing = wl_handle->device_mem_page - used_memory_for_writing;
		uint16_t dataSize = (free_memory_for_writing >= _Size) ? _Size : free_memory_for_writing;

		HAL_StatusTypeDef status = at24c256n_wait_for_ready(wl_handle);
		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_NOT_CONNECTED;
		}

		status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
				_MemAddress, MEMADD_SIZE, _pData, dataSize, WRITE_TIMEOUT);

		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_WRITE_ERROR;
		}

		_MemAddress += dataSize;
		_Size -= dataSize;
		_pData += dataSize;
	}

	return NVM_DEVICE_STATUS_OK;
}

static nvm_device_status_t erase_all_low(nvm_device_api_handle *const wl_handle) {
	uint16_t mem_address = 0x0000;
	int32_t size = wl_handle->device_mem_capacity;
	uint8_t page_buf[wl_handle->device_mem_page];
	for (int i = 0; i < sizeof(page_buf); i++) {
		page_buf[i] = 0xFF;
	}

	while (size > 0) {
		HAL_StatusTypeDef status = at24c256n_wait_for_ready(wl_handle);
		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_NOT_CONNECTED;
		}

		status = HAL_I2C_Mem_Write(wl_handle->hi2c, wl_handle->device_address,
				mem_address, MEMADD_SIZE, page_buf, wl_handle->device_mem_page, WRITE_TIMEOUT);

		if (status != HAL_OK) {
			return NVM_DEVICE_STATUS_WRITE_ERROR;
		}

		mem_address += wl_handle->device_mem_page;
		size -= wl_handle->device_mem_page;
	}
	return NVM_DEVICE_STATUS_OK;
}

static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle) {
    return HAL_I2C_IsDeviceReady(wl_handle->hi2c, wl_handle->device_address, MAX_ATTEMPTS_TRY, WRITE_TIMEOUT);
}

nvm_device_api_t api_low = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low
};
