
#include "nvm_high_api.h"

extern nvm_device_api_t api_low;

static nvm_high_api_status_t init  (nvm_device_api_handle *const wl_handle, nvm_formatting_status_t format){
	 return api_low.init(wl_handle);
}
static nvm_high_api_status_t read  (nvm_device_api_handle *const wl_handle, nvm_data_t *data){
	return api_low.read(wl_handle, &(data->data), sizeof(data->data));
}
static nvm_high_api_status_t write (nvm_device_api_handle *const wl_handle, const nvm_data_t *const data){
	return NVM_API_STATUS_OK;
}

nvm_api_t api ={
		.init = init,
		.read = read,
		.write = write
};
