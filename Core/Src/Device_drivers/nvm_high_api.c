
#include "nvm_high_api.h"

extern nvm_device_api_t api_low;

nvm_data_cache data_cache;

static nvm_high_api_status_t init  (nvm_device_api_handle *const wl_handle, nvm_formatting_status_t format){
	 return api_low.init(wl_handle);
}


static nvm_high_api_status_t read  (nvm_device_api_handle *const wl_handle, nvm_data_t *data){
	if (0 == wl_handle->is_device_initialized) {
		return NVM_API_STATUS_NOT_INITIALIZED;
	}

	if (NULL == wl_handle || NULL == wl_handle->hi2c || 0 == wl_handle->device_address || NULL == data) {
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}

	if (1 == data_cache.is_valid) {
		*data = data_cache.data;
	} else {
		if (api_low.read(wl_handle, &(data->data), sizeof(data->data)) != NVM_DEVICE_STATUS_OK ) {
			return NVM_API_STATUS_READ_ERROR;
		}
	}

	return NVM_API_STATUS_OK;
}
static nvm_high_api_status_t write (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data){
	return NVM_API_STATUS_OK;
}

nvm_api_t api ={
		.init = init,
		.read = read,
		.write = write
};
