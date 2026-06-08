
#include "nvm_high_api.h"

extern nvm_device_api_t api_low;

static nvm_high_api_status_t init(nvm_device_api_handle *const wl_handle, nvm_formatting_status_t format)
{
	// 0) Verify parameters
	if (wl_handle->hi2c == NULL || wl_handle->device_address == NULL)
	{
		return NVM_API_STATUS_INVALID_PARAMETERS;
	}
	// 1) Verify format
	if (format == NVM_API_STATUS_FORMAT)
	{
		// 1) erase -> busy_page = 0x0000
		nvm_device_status_t status = api_low.erase_all(wl_handle);
		if (status != NVM_DEVICE_STATUS_OK)
		{
			return NVM_API_STATUS_WRITE_ERROR;
		}
	}
	// 2) call init_low
	nvm_device_status_t status = api_low.init(wl_handle);
	if (status != NVM_DEVICE_STATUS_OK)
	{
		return NVM_API_STATUS_NOT_INITIALIZED;
	}
	// 3) fill the field is_device_initialized
	wl_handle->is_device_initialized = 1;
	// 4) call search last busy page
	status = api_low.search_last_busy_page(wl_handle, sizeof(nvm_data_t));
	// 5) it looks like nothing to fill
	// 6) return status
	return NVM_API_STATUS_OK;
}
static nvm_high_api_status_t read(nvm_device_api_handle *const wl_handle, nvm_data_t *data)
{
	return api_low.read(wl_handle, &(data->data), sizeof(data->data));
}
static nvm_high_api_status_t write(nvm_device_api_handle *const wl_handle, const nvm_data_t *const data)
{
	return NVM_API_STATUS_OK;
}

nvm_api_t api = {
	.init = init,
	.read = read,
	.write = write};
