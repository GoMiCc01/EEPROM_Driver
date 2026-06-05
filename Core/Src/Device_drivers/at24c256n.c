
#include "at24c256n.h"

static nvm_api_status_t init_low  		(const nvm_device_api_handle *const wl_handle){
	return NVM_API_STATUS_OK;
}
static nvm_api_status_t read_low  		(const nvm_device_api_handle *const wl_handle, const uint16_t address, uint8_t *const data, const uint32_t){
	return NVM_API_STATUS_OK;
}
static nvm_api_status_t write_low 		(const nvm_device_api_handle *const wl_handle, const uint16_t address, const uint8_t *const data, const uint32_t){
	return NVM_API_STATUS_OK;
}
static nvm_api_status_t erase_all_low 	(const nvm_device_api_handle *const wl_handle){
	return NVM_API_STATUS_OK;
}

nvm_device_api_t api_low = {
		.init = init_low,
		.read = read_low,
		.write = write_low,
		.erase_all = erase_all_low
};
