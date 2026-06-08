
#include "nvm_high_api.h"
#define LAST_BUSY_PAGE 0x7fff

extern nvm_device_api_t api_low;

nvm_data_cache data_cache;

static nvm_high_api_status_t init  (nvm_device_api_handle *const wl_handle, nvm_formatting_status_t format){
	 return api_low.init(wl_handle);
}
static nvm_high_api_status_t read  (nvm_device_api_handle *const wl_handle, nvm_data_t *data){
	return api_low.read(wl_handle, &(data->data), sizeof(data->data));
}
static nvm_high_api_status_t write (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data){

	if (1 != wl_handle->is_device_initialized) {
		return NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NULL == wl_handle->device_address || NULL == wl_handle->hi2c || NULL == data->data) {
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}
	if (LAST_BUSY_PAGE == wl_handle->last_busy_page) {
		if (NVM_DEVICE_STATUS_OK != api_low.erase_all(wl_handle)) return NVM_API_STATUS_WRITE_ERROR;
	}

	if (NVM_DEVICE_STATUS_OK != api_low.write(wl_handle, data->data, sizeof(data->data))) {
		return NVM_API_STATUS_WRITE_ERROR;
	} else {
		data_cache.data = data->data;
		data_cache.is_valid = 1;
		return NVM_API_STATUS_OK;
	}

}

nvm_api_t api ={
		.init = init,
		.read = read,
		.write = write
};
