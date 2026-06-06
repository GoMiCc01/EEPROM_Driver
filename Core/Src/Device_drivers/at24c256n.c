
#include "at24c256n.h"

static nvm_device_status_t init_low  		(nvm_device_api_handle *const wl_handle){
	return NVM_DEVICE_STATUS_OK;
}
static nvm_device_status_t read_low  		(nvm_device_api_handle *const wl_handle, uint8_t *const data, const uint16_t size){
	return NVM_DEVICE_STATUS_OK;
}
static nvm_device_status_t write_low 		(nvm_device_api_handle *const wl_handle, const uint8_t *const data, const uint16_t size){
	return NVM_DEVICE_STATUS_OK;
}
static nvm_device_status_t erase_all_low 	(nvm_device_api_handle *const wl_handle){
	return NVM_DEVICE_STATUS_OK;
}
static nvm_device_status_t search_last_busy_page  	(nvm_device_api_handle *const wl_handle, const uint16_t size){
	return NVM_DEVICE_STATUS_OK;
}

nvm_device_api_t api_low = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low,
		.search_last_busy_page = search_last_busy_page,
};
