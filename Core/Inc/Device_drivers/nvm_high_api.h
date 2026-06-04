#ifndef INC_DEVICE_DRIVERS_NVM_HIGH_API_H_
#define INC_DEVICE_DRIVERS_NVM_HIGH_API_H_

#include <stdint.h>
#include "nvm_api.h"

/**
 * 	stores pointers to low-level functions
 */
typedef struct
{
	nvm_api_status_t (*init)  (const nvm_device_api_handle *const wl_handle);
	nvm_api_status_t (*read)  (const nvm_device_api_handle *const wl_handle, uint8_t *data);
	nvm_api_status_t (*write) (const nvm_device_api_handle *const wl_handle, const uint8_t *const data);
} nvm_api_t;


#endif /* INC_DEVICE_DRIVERS_NVM_HIGH_API_H_ */
