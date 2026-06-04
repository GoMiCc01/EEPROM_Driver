#ifndef INC_DEVICE_DRIVERS_AT24C256N_H_
#define INC_DEVICE_DRIVERS_AT24C256N_H_

#include "nvm_api.h"

typedef struct
{
	nvm_api_status_t (*init)  (const nvm_device_api_handle *const wl_handle);
	nvm_api_status_t (*read)  (const nvm_device_api_handle *const wl_handle, const uint16_t address, uint8_t *const data, const uint32_t);
	nvm_api_status_t (*write) (const nvm_device_api_handle *const wl_handle, const uint16_t address, const uint8_t *const data, const uint32_t);
	nvm_api_status_t (*erase_all) (const nvm_device_api_handle *const wl_handle);
} nvm_device_api_t;

nvm_api_status_t init_low  (const nvm_device_api_handle *const wl_handle);
nvm_api_status_t read_low  (const nvm_device_api_handle *const wl_handle, const uint16_t address, uint8_t *const data, const uint32_t);
nvm_api_status_t write_low (const nvm_device_api_handle *const wl_handle, const uint16_t address, const uint8_t *const data, const uint32_t);
nvm_api_status_t erase_all_low (const nvm_device_api_handle *const wl_handle);

#endif /* INC_DEVICE_DRIVERS_AT24C256N_H_ */
