
#include "nvm_high_api.h"

extern nvm_device_api_t api_low;

nvm_api_status_t init  (const nvm_device_api_handle *const wl_handle){
	return api_low.init(wl_handle);
}
nvm_api_status_t read  (const nvm_device_api_handle *const wl_handle, uint8_t *data){
	return api_low.read(wl_handle, wl_handle->device_address, data, sizeof(data));
}
nvm_api_status_t write (const nvm_device_api_handle *const wl_handle, const uint8_t *const data){
	return NVM_API_STATUS_OK;
}

nvm_api_t api ={
		init, read, write
};
