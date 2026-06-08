
#include "at24c256n.h"

static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle);

static nvm_device_status_t init_low  		(nvm_device_api_handle *const wl_handle){
	at24c256n_wait_for_ready(wl_handle);
	return NVM_DEVICE_STATUS_OK; //!
}
static nvm_device_status_t read_low  		(nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size){

	if (at24c256n_wait_for_ready(wl_handle) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

	if (HAL_I2C_Mem_Read(
		wl_handle->hi2c,
	    wl_handle->device_address,
		wl_handle->last_busy_page,
	    I2C_MEMADD_SIZE_16BIT,
	    data,
	    size,
	    100) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_READ_ERROR;
	}

	return NVM_DEVICE_STATUS_OK;

}
static nvm_device_status_t write_low 		(nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size){
	return NVM_DEVICE_STATUS_OK; //!
}
static nvm_device_status_t erase_all_low 	(nvm_device_api_handle *const wl_handle){
	return NVM_DEVICE_STATUS_OK; //!
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
        wl_handle->last_busy_page = 0;
        return NVM_DEVICE_STATUS_OK;
    }

    wl_handle->last_busy_page = (last_written / size) * size;

    return NVM_DEVICE_STATUS_OK;
}

static HAL_StatusTypeDef at24c256n_wait_for_ready(nvm_device_api_handle *const wl_handle) {
    return HAL_I2C_IsDeviceReady(wl_handle->hi2c, wl_handle->device_address, 100, 100);
}

nvm_device_api_t api_low = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low,
		.search_last_busy_page = search_last_busy_page,
};
