
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

	uint16_t addr = wl_handle->last_busy_page * wl_handle->device_mem_page;

	if (HAL_I2C_Mem_Read(
		wl_handle->hi2c,
	    wl_handle->device_address,
	    addr,
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
static nvm_device_status_t search_last_busy_page  	(nvm_device_api_handle *const wl_handle){

	if (at24c256n_wait_for_ready(wl_handle) != HAL_OK)
	{
		return NVM_DEVICE_STATUS_NOT_CONNECTED;
	}

    uint8_t buffer[wl_handle->device_mem_page];
    uint16_t pages = wl_handle->device_mem_capacity / wl_handle->device_mem_page;

    for (uint16_t page = 0; page < pages; page++)
    {
        uint16_t addr = page * wl_handle->device_mem_page;

        if (HAL_I2C_Mem_Read(
                wl_handle->hi2c,
                wl_handle->device_address,
                addr,
                I2C_MEMADD_SIZE_16BIT,
                buffer,
                wl_handle->device_mem_page,
                100) != HAL_OK)
        {
            return NVM_DEVICE_STATUS_READ_ERROR;
        }

        uint8_t empty = 1;

        for (uint16_t i = 0; i < wl_handle->device_mem_page; i++)
        {
        	if (buffer[i] != 0xFF)
        	{
        		empty = 0;
        		break;
        	}
        }

        if (empty)
        {
        	wl_handle->last_busy_page = (page == 0) ? 0 : (page - 1);
        	return NVM_DEVICE_STATUS_OK;
        }
    }
    wl_handle->last_busy_page = pages - 1;

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
